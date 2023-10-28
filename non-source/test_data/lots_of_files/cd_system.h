/*

Platform layer services to other modules

*/

// TOP

#ifndef CD_SYSTEM_H
#define CD_SYSTEM_H

struct Platform_Handle{
    u8 opaque_memory[128];
};

#ifdef DEVELOPER

struct File_Dump{
    Platform_Handle handle;
    i32 size;
};

#define DBG_Dump_Begin(n) File_Dump n(char *file_name)
typedef DBG_Dump_Begin(DBG_Dump_Begin_Function);

#define DBG_Dump_End(n) b32 n(File_Dump dump, void *buffer)
typedef DBG_Dump_End(DBG_Dump_End_Function);

#define DBG_Dump_Out(n) b32 n(char *file_name, void *buffer, i32 size);
typedef DBG_Dump_Out(DBG_Dump_Out_Function);

#define DBG_Copy(n) b32 n(char *source, char *name)
typedef DBG_Copy(DBG_Copy_Function);

#define DBG_Call_Script(n) b32 n(char *script)
typedef DBG_Call_Script(DBG_Call_Script_Function);

#define DBG_Module_Path(n) i32 n(char *out, i32 capacity)
typedef DBG_Module_Path(DBG_Module_Path_Function);

#define DBG_Working_Path(n) i32 n(char *out, i32 capacity)
typedef DBG_Working_Path(DBG_Working_Path_Function);

#define DBG_Memory_Allocate(n) void *n(i32 size)
typedef DBG_Memory_Allocate(DBG_Memory_Allocate_Function);

#define DBG_Memory_Free(n) void n(void *ptr)
typedef DBG_Memory_Free(DBG_Memory_Free_Function);

#endif

struct System_API{
#ifdef DEVELOPER
    DBG_Dump_Begin_Function *DBG_dump_begin;
    DBG_Dump_End_Function *DBG_dump_end;
    DBG_Dump_Out_Function *DBG_dump_out;
    DBG_Copy_Function *DBG_copy;
    DBG_Call_Script_Function *DBG_call_script;
    DBG_Module_Path_Function *DBG_module_path;
    DBG_Working_Path_Function *DBG_working_path;
    DBG_Memory_Allocate_Function *DBG_memory_allocate;
    DBG_Memory_Free_Function *DBG_memory_free;
#endif
};

#endif

// BOTTOM