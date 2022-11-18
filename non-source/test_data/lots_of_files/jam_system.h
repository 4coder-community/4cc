/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Platform Services
 */

// TODO(allen):
// - The file loading scheme requires that the platform layer
//     make an extra allocation and then is later used to free
//     that allocation.  It would be better for file loading to
//     be broken up into two steps: (1) get size (2) load into
//     buffer so the application can provide any buffer it wants.
//

struct File{
	void *data;
	u64 size;
};

#define SIG_SYS_LOAD_FILE(name) File name(char *filename)
#define SIG_SYS_SAVE_FILE(name) void name(File file, char *filename)
#define SIG_SYS_FREE_FILE(name) void name(File file)

internal
SIG_SYS_LOAD_FILE(system_load_file);

internal
SIG_SYS_SAVE_FILE(system_save_file);

internal
SIG_SYS_FREE_FILE(system_free_file);



#define SIS_SYS_SET_KEYBIND(name) void name(i32 btn_id, char key)

internal
SIS_SYS_SET_KEYBIND(system_set_keybind);

