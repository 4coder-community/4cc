///////////////////////////////////////////////////////////////////
//*-------------------------------------------------------------*//
//| Part of the Game Jolt API C++ Library (http://gamejolt.com) |//
//*-------------------------------------------------------------*//
//| Released under the zlib License                             |//
//| More information available in the readme file               |//
//*-------------------------------------------------------------*//
///////////////////////////////////////////////////////////////////
#pragma once
#ifndef _GJ_GUARD_LOOKUP_H_
#define _GJ_GUARD_LOOKUP_H_

typedef unsigned int gjUint;
#ifndef ASSERT
    #define ASSERT(x) 
#endif


// ****************************************************************
// lookup container class
template <typename T> class gjLookup final
{
public:
    //! internal types
    typedef std::pair<std::string, T>       gjEntry;
    typedef std::vector<gjEntry>            gjList;
    typedef typename gjList::iterator       gjIterator;
    typedef typename gjList::const_iterator gjConstIterator;


private:
    gjList m_aList;   //!< vector-list with pair-values


public:
    gjLookup()noexcept;
    gjLookup(const gjLookup<T>& c)noexcept;
    gjLookup(gjLookup<T>&& m)noexcept;
    ~gjLookup();

    //! assignment operator
    //! @{
    gjLookup<T>& operator = (gjLookup<T> o)noexcept;
    template <typename S> friend void swap(gjLookup<S>& a, gjLookup<S>& b)noexcept;
    //! @}

    //! access specific entry
    //! @{
    const T& at(const char* pcKey)const noexcept;
    T& operator [] (const char* pcKey)noexcept;
    inline T& operator [] (const gjUint& iIndex)noexcept {return m_aList[iIndex].second;}
    //! @}

    //! check number of existing entries
    //! @{
    inline gjUint count(const char* pcKey)const noexcept {return this->__check(this->__retrieve(pcKey)) ? 1 : 0;}
    inline gjUint size()const noexcept                   {return m_aList.size();}
    inline bool empty()const noexcept                    {return m_aList.empty();}
    //! @}

    //! control memory consumption
    //! @{
    inline void reserve(const gjUint& iReserve)noexcept {m_aList.reserve(iReserve);}
    inline gjUint capacity()const noexcept              {return m_aList.capacity();}
    //! @}

    //! remove existing entries
    //! @{
    void erase(const T& Entry)noexcept;
    void erase(const char* pcKey)noexcept;
    inline void erase(const gjUint& iIndex)noexcept            {ASSERT(iIndex < m_aList.size()) m_aList.erase(m_aList.begin()+iIndex);}
    inline void erase(const gjConstIterator& Iterator)noexcept {m_aList.erase(Iterator);}
    inline void clear()noexcept                                {m_aList.clear();}
    //! @}

    //! retrieve internal iterator
    //! @{
    inline gjIterator begin()noexcept            {return m_aList.begin();}
    inline gjConstIterator begin()const noexcept {return m_aList.begin();}
    inline gjIterator end()noexcept              {return m_aList.end();}
    inline gjConstIterator end()const noexcept   {return m_aList.end();}
    //! @}


private:
    //! retrieve iterator
    //! @{
    gjIterator __retrieve(const T& Entry)noexcept;
    gjConstIterator __retrieve(const T& Entry)const noexcept;
    gjIterator __retrieve(const char* pcKey)noexcept;
    gjConstIterator __retrieve(const char* pcKey)const noexcept;
    //! @}

    //! check for valid iterator
    //! @{
    inline bool __check(const gjIterator& it)const noexcept      {return (it != m_aList.end()) ? true : false;}
    inline bool __check(const gjConstIterator& it)const noexcept {return (it != m_aList.end()) ? true : false;}
    //! @}
};


// ****************************************************************
// constructor
template <typename T> gjLookup<T>::gjLookup()noexcept
{
    // reserve variable sized memory
    constexpr_var gjUint iSize = 1 + 64/sizeof(T);
    m_aList.reserve(iSize);
}

template <typename T> gjLookup<T>::gjLookup(const gjLookup<T>& c)noexcept
: m_aList (c.m_aList)
{
}

template <typename T> gjLookup<T>::gjLookup(gjLookup<T>&& m)noexcept
: m_aList (std::move(m.m_aList))
{
}


// ****************************************************************
// destructor
template <typename T> gjLookup<T>::~gjLookup()
{
    m_aList.clear();
}


// ****************************************************************
// assignment operator
template <typename T> gjLookup<T>& gjLookup<T>::operator = (gjLookup<T> o)noexcept
{
    swap(*this, o);
    return *this;
}

template <typename S> void swap(gjLookup<S>& a, gjLookup<S>& b)noexcept
{
    using std::swap;
    swap(a.m_aList, b.m_aList);
}


// ****************************************************************
// access specific entry
template <typename T> const T& gjLookup<T>::at(const char* pcKey)const noexcept
{
    // retrieve and check iterator by specific key
    auto it = this->__retrieve(pcKey);
    ASSERT(this->__check(it))

    return it->second;
}


// ****************************************************************
// access specific entry and create it if necessary
template <typename T> T& gjLookup<T>::operator [] (const char* pcKey)noexcept
{
    // retrieve and check iterator by specific key
    auto it = this->__retrieve(pcKey);
    if(!this->__check(it))
    {
        // create new entry
        m_aList.push_back(gjEntry(pcKey, T()));
        it = m_aList.end()-1;
    }

    return it->second;
}


// ****************************************************************
// remove existing entry
template <typename T> void gjLookup<T>::erase(const T& Entry)noexcept
{
    // retrieve and check iterator by specific value
    auto it = this->__retrieve(Entry);
    if(this->__check(it))
    {
        // remove existing entry
        m_aList.erase(it);
    }
}


template <typename T> void gjLookup<T>::erase(const char* pcKey)noexcept
{
    // retrieve and check iterator by specific key
    auto it = this->__retrieve(pcKey);
    if(this->__check(it))
    {
        // remove existing entry
        m_aList.erase(it);
    }
}


// ****************************************************************
// retrieve iterator by specific value
template <typename T> typename gjLookup<T>::gjIterator gjLookup<T>::__retrieve(const T& Entry)noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare values
        if(it->second == Entry)
            return it;
    }
    return m_aList.end();
}

template <typename T> typename gjLookup<T>::gjConstIterator gjLookup<T>::__retrieve(const T& Entry)const noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare values
        if(it->second == Entry)
            return it;
    }
    return m_aList.end();
}


// ****************************************************************
// retrieve iterator by specific key
template <typename T> typename gjLookup<T>::gjIterator gjLookup<T>::__retrieve(const char* pcKey)noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare string-keys
        if(!std::strcmp(it->first.c_str(), pcKey))
            return it;
    }
    return m_aList.end();
}

template <typename T> typename gjLookup<T>::gjConstIterator gjLookup<T>::__retrieve(const char* pcKey)const noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare string-keys
        if(!std::strcmp(it->first.c_str(), pcKey))
            return it;
    }
    return m_aList.end();
}


#endif /* _GJ_GUARD_LOOKUP_H_ */