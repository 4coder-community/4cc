/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* pplconcrt.h
*
* Parallel Patterns Library - PPL ConcRT helpers
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#ifndef _PPLCONCRT_H
#define _PPLCONCRT_H

#include <pplinterface.h>
#include <concrt.h>

#define _noexcept

namespace Concurrency
{
// The extensibility namespace contains the type definitions that are used by ppltasks implementation
namespace extensibility
{
    typedef ::Concurrency::event event_t;

    typedef ::Concurrency::critical_section critical_section_t;
    typedef critical_section_t::scoped_lock scoped_critical_section_t;

    typedef ::Concurrency::reader_writer_lock reader_writer_lock_t;
    typedef ::Concurrency::reader_writer_lock::scoped_lock scoped_rw_lock_t;
    typedef ::Concurrency::reader_writer_lock::scoped_lock_read scoped_read_lock_t;

    typedef ::Concurrency::details::_ReentrantBlockingLock recursive_lock_t;
    typedef recursive_lock_t::_Scoped_lock scoped_recursive_lock_t;
}

_CRTIMP2 bool __cdecl is_current_task_group_canceling();

namespace details
{
    namespace platform 
    {
        _CRTIMP2 unsigned int __cdecl GetNextAsyncId();
        _CRTIMP size_t __cdecl CaptureCallstack(void **stackData, size_t skipFrames, size_t captureFrames);
        _CRTIMP long __cdecl GetCurrentThreadId();
    }
}

} // Concurrency

#include <pplcancellation_token.h>
#include <ppl.h>

namespace Concurrency
{

inline std::shared_ptr< ::Concurrency::scheduler_interface> get_ambient_scheduler()
{
    return nullptr;
}

inline void set_ambient_scheduler(std::shared_ptr< ::Concurrency::scheduler_interface> _Scheduler)
{
    throw invalid_operation("Scheduler is already initialized");
}

namespace details
{

// It has to be a macro because the debugger needs __debugbreak
// breaks on the frame with exception pointer.
// It can be only used within _ExceptionHolder
#ifndef _REPORT_PPLTASK_UNOBSERVED_EXCEPTION
#define _REPORT_PPLTASK_UNOBSERVED_EXCEPTION() do { \
        ReportUnhandledError(); \
        __debugbreak(); \
        Concurrency::details::_ReportUnobservedException(); \
} while(false)

#endif

    template<typename _T>
    struct _AutoDeleter
    {
        _AutoDeleter(_T *_PPtr) : _Ptr(_PPtr) {}
        ~_AutoDeleter () { delete _Ptr; } 
        _T *_Ptr;
    };

    struct _TaskProcHandle : Concurrency::details::_UnrealizedChore
    {
        _TaskProcHandle()
        {
            this->m_pFunction = &Concurrency::details::_UnrealizedChore::_InvokeBridge<_TaskProcHandle>;
            this->_SetRuntimeOwnsLifetime(true);
        }

        virtual ~_TaskProcHandle() {}
        virtual void invoke() const = 0;

        void operator()() const
        {
            this->invoke();
        }

        static void __cdecl _RunChoreBridge(void * _Parameter)
        {
            auto _PTaskHandle = static_cast<_TaskProcHandle *>(_Parameter);
            _AutoDeleter<_TaskProcHandle> _AutoDeleter(_PTaskHandle);
            _PTaskHandle->invoke();
        }
    };

    // This is an abstraction that is built on top of the scheduler to provide these additional functionalities
    // - Ability to wait on a work item
    // - Ability to cancel a work item
    // - Ability to inline work on invocation of RunAndWait
    // The concrt specific implementation provided the following additional features
    // - Interoperate with concrt task groups and ppl parallel_for algorithms for cancellation
    // - Stack guard
    // - Determine if the current task is cancelled
    class _TaskCollectionImpl 
    {
    public:

        typedef Concurrency::details::_TaskProcHandle _TaskProcHandle_t;

        _TaskCollectionImpl(::Concurrency::scheduler_ptr _PScheduler) 
            : _M_pTaskCollection(nullptr), _M_pScheduler(_PScheduler)
        {
        }

        ~_TaskCollectionImpl()
        {
            if (_M_pTaskCollection != nullptr)
            {
                _M_pTaskCollection->_Release();
                _M_pTaskCollection = nullptr;
            }
        }

        void _ScheduleTask(_TaskProcHandle_t* _Parameter, _TaskInliningMode _InliningMode)
        {
            if (!_M_pScheduler)
            {
                // Construct the task collection; We use none token to provent it becoming interruption point.
                _M_pTaskCollection = _AsyncTaskCollection::_NewCollection(::Concurrency::details::_CancellationTokenState::_None());
            }

            try 
            {
                if (_M_pTaskCollection != nullptr)
                {
                    // Do not need to check its returning state, more details please refer to _Wait method.
                    auto _PChore = static_cast< ::Concurrency::details::_UnrealizedChore*>(_Parameter);
                    _M_pTaskCollection->_ScheduleWithAutoInline(_PChore, _InliningMode);
                }
                else
                {
                    // Schedule the work on the user provided scheduler
                    if (_InliningMode == _ForceInline)
                    {
                        _TaskProcHandle_t::_RunChoreBridge(_Parameter);
                    }
                    else
                    {
                        _M_pScheduler->schedule(_TaskProcHandle_t::_RunChoreBridge, _Parameter);
                    }
                }
            }
            catch(...)
            {
                _SetScheduled();
                throw;
            }

            // Set the event in case anyone is waiting to notify that this task has been scheduled. In the case where we
            // execute the chore inline, the event should be set after the chore has executed, to prevent a different thread 
            // performing a wait on the task from waiting on the task collection before the chore is actually added to it, 
            // and thereby returning from the wait() before the chore has executed.
            _SetScheduled();
        }

        void _Cancel()
        {
            // Ensure that RunAndWait makes progress.
            _SetScheduled();

            if (_M_pTaskCollection != nullptr)
            {
                _M_pTaskCollection->_Cancel();
            }
        }

        void _RunAndWait()
        {
            _M_Scheduled.wait();

            if (_M_pTaskCollection != nullptr)
            {
                // When it returns cancelled, either work chore or the cancel thread should already have set task's state
                // properly -- cancelled state or completed state (because there was no interruption point). 
                // For tasks with unwrapped tasks, we should not change the state of current task, since the unwrapped task are still running.
                _M_pTaskCollection->_RunAndWait();
            }
            else
            {
                _M_Completed.wait();
            }
        }

        void _Wait()
        {
            _M_Completed.wait();
        }

        void _Complete()
        {
            // Ensure that RunAndWait makes progress.
            _SetScheduled();
            _M_Completed.set();
        }

        ::Concurrency::scheduler_ptr _GetScheduler() const
        {
            return _M_pScheduler;
        }

        // Fire and forget
        static void _RunTask(TaskProc _Proc, void * _Parameter, _TaskInliningMode _InliningMode)
        {
            Concurrency::details::_StackGuard _Guard;

            if (_Guard._ShouldInline(_InliningMode))
            {
                _Proc(_Parameter);
            }
            else
            {
                // Schedule the work on the current scheduler
                _CurrentScheduler::_ScheduleTask(_Proc, _Parameter);
            }
        }

        static bool __cdecl _Is_cancellation_requested()
        {
            // ConcRT scheduler under the hood is using TaskCollection, which is same as task_group
            return ::Concurrency::is_current_task_group_canceling();
        }

    private:
        void _SetScheduled()
        {
            _M_Scheduled.set();
        }

        ::Concurrency::extensibility::event_t _M_Scheduled;
        ::Concurrency::extensibility::event_t _M_Completed;
        _AsyncTaskCollection* _M_pTaskCollection;
        ::Concurrency::scheduler_ptr _M_pScheduler;
    };

    // For create_async lambdas that return a (non-task) result, we oversubscriber the current task for the duration of the
    // lambda.
    struct _Task_generator_oversubscriber
    {
        _Task_generator_oversubscriber()
        {
            _Context::_Oversubscribe(true);
        }

        ~_Task_generator_oversubscriber()
        {
            _Context::_Oversubscribe(false);
        }
    };

    typedef ::Concurrency::details::_TaskCollectionImpl _TaskCollection_t;
    typedef ::Concurrency::details::_TaskInliningMode _TaskInliningMode_t;
    typedef ::Concurrency::details::_Task_generator_oversubscriber _Task_generator_oversubscriber_t;
} // details

} // Concurrency

namespace concurrency = Concurrency;

#endif // _PPLCONCRT_H
