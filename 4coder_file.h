/*
 4coder_file.h - File enumeration and reading procedures for each platform.
 
TYPE: 'utility'
*/

// TOP

#if !defined(FCODER_FILE_ENUMERATOR_CPP)
#define FCODER_FILE_ENUMERATOR_CPP

#include "4coder_lib/4coder_mem.h"
#include "4coder_os_comp_cracking.h"
#define FSTRING_IMPLEMENTATION
#include "4coder_lib/4coder_string.h"

#include <stdio.h>

typedef int32_t bool32;

#if defined(IS_WINDOWS)

//// WINDOWS BEGIN ////
#define UNICODE
#include <Windows.h>
typedef TCHAR Filename_Character;
#define SLASH '\\'
//// WINDOWS END ////

#elif defined(IS_LINUX) || defined(IS_MAC)

//// UNIX BEGIN ////
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
typedef char Filename_Character;
#define SLASH '/'
//// UNIX END ////

#else
# error metdata generator not supported on this platform
#endif

struct Cross_Platform_File_Info{
    Filename_Character *name;
    int32_t len;
    bool32 is_folder;
};

struct Cross_Platform_File_List{
    Cross_Platform_File_Info *info;
    int32_t count;
    int32_t final_length;
    Filename_Character final_name[4096];
};

typedef bool32 File_Filter(Filename_Character *name, int32_t len);

static Cross_Platform_File_List
get_file_list(Partition *part, Filename_Character *dir, File_Filter *filter);

static Filename_Character*
encode(Partition *part, char *str){
    int32_t size = 0;
    for (;str[size]!=0;++size);
    
    Filename_Character *out = push_array(part, Filename_Character, size + 1);
    push_align(part, 8);
    
    if (out == 0){
        fprintf(stdout, "fatal error: ran out of memory encoding string to filename\n");
        exit(1);
    }
    
    int32_t j = 0;
    for (int32_t i = 0; i <= size; ++i){
        if (str[i] != '"'){
            out[j++] = str[i];
        }
    }
    
    return(out);
}

static char*
unencode(Partition *part, Filename_Character *str, int32_t len){
    Temp_Memory temp = begin_temp_memory(part);
    char *out = push_array(part, char, len + 1);
    push_align(part, 8);
    
    if (out == 0){
        fprintf(stdout, "fatal error: ran out of memory unencoding string to filename\n");
        exit(1);
    }
    
    for (int32_t i = 0; i <= len; ++i){
        if (str[i] <= 127){
            out[i] = (char)str[i];
        }
        else{
            out = 0;
            end_temp_memory(temp);
            break;
        }
    }
    
    return(out);
}

static bool32
filter_all(Filename_Character *name, int32_t len){
    return(true);
}

static bool32
filter_is_code_file(Filename_Character *name, int32_t len){
    bool32 is_code = false;
    if (len >= 5){
        Filename_Character *ext = &name[len - 4];
        if (ext[0] == '.' && ext[1] == 'c' && ext[2] == 'p' && ext[3] == 'p'){
            is_code = true;
        }
        else if (ext[0] == '.' && ext[1] == 'h' && ext[2] == 'p' && ext[3] == 'p'){
            is_code = true;
        }
    }
    if (len >= 4){
        Filename_Character *ext = &name[len - 3];
        if (ext[0] == '.' && ext[1] == 'c' && ext[2] == 'c'){
            is_code = true;
        }
    }
    if (len >= 3){
        Filename_Character *ext = &name[len - 2];
        if (ext[0] == '.' && ext[1] == 'h'){
            is_code = true;
        }
        else if (ext[0] == '.' && ext[1] == 'c'){
            is_code = true;
        }
    }
    return(is_code);
}

#if defined(IS_WINDOWS)

//// WINDOWS BEGIN ////
static Cross_Platform_File_List
get_file_list(Partition *part, Filename_Character *dir, File_Filter *filter){
    if (part == 0){
        fprintf(stdout, "fatal error: NULL part passed to %s\n", __FUNCTION__);
        exit(1);
    }
    if (dir == 0){
        fprintf(stdout, "fatal error: NULL dir passed to %s\n", __FUNCTION__);
        exit(1);
    }
    
    HANDLE dir_handle =
        CreateFile(dir,
                   FILE_LIST_DIRECTORY,
                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                   0,
                   OPEN_EXISTING,
                   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                   0);
    
    if (dir_handle == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not open directory handle\n");
        exit(1);
    }
    
    Filename_Character final_name[4096];
    DWORD final_length = GetFinalPathNameByHandle(dir_handle, final_name, sizeof(final_name), 0);
    if (final_length > sizeof(final_name)){
        fprintf(stdout, "fatal error: path name too long for local buffer\n");
        exit(1);
    }
    CloseHandle(dir_handle);
    
    final_length -= 4;
    memmove(final_name, final_name + 4, final_length*sizeof(*final_name));
    final_name[final_length] = '\\';
    final_name[final_length + 1] = '*';
    final_name[final_length + 2] = 0;
    
    WIN32_FIND_DATA find_data = {0};
    HANDLE search = FindFirstFile(final_name, &find_data);
    if (search == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not begin a file search\n");
        exit(1);
    }
    
    int32_t character_count = 0;
    int32_t file_count = 0;
    BOOL more_files = true;
    do{
        Filename_Character *name = &find_data.cFileName[0];
        
        int32_t size = 0;
        for(;name[size];++size);
        
        uint32_t attribs = find_data.dwFileAttributes;
        bool32 is_folder = ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
        bool32 is_hidden = ((attribs & FILE_ATTRIBUTE_HIDDEN) != 0);
        
        if (!is_hidden){
            if (name[0] != '.' && (is_folder || filter(name, size))){
                ++file_count;
                character_count += size + 1;
            }
        }
        
        more_files = FindNextFile(search, &find_data);
    }while(more_files);
    FindClose(search);
    
    Cross_Platform_File_List list = {0};
    Temp_Memory part_reset = begin_temp_memory(part);
    
    int32_t rounded_char_size = (character_count*sizeof(Filename_Character) + 7)&(~7);
    int32_t memsize = rounded_char_size + file_count*sizeof(Cross_Platform_File_Info);
    void *mem = push_array(part, uint8_t, memsize);
    if (mem == 0){
        fprintf(stdout, "fatal error: not enough memory on the partition for a file list.\n");
        exit(1);
    }
    
    Filename_Character *char_ptr = (Filename_Character*)mem;
    Cross_Platform_File_Info *info_ptr = (Cross_Platform_File_Info*)((uint8_t*)mem + rounded_char_size);
    
    Filename_Character *char_ptr_end = (Filename_Character*)info_ptr;
    Cross_Platform_File_Info *info_ptr_end = info_ptr + file_count;
    
    Cross_Platform_File_Info *info_ptr_base = info_ptr;
    
    search = FindFirstFile(final_name, &find_data);
    if (search == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not restart a file search\n");
        exit(1);
    }
    
    int32_t adjusted_file_count = 0;
    more_files = true;
    do{
        Filename_Character *name = &find_data.cFileName[0];
        
        int32_t size = 0;
        for(;name[size]!=0;++size);
        
        uint32_t attribs = find_data.dwFileAttributes;
        bool32 is_folder = ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
        bool32 is_hidden = ((attribs & FILE_ATTRIBUTE_HIDDEN) != 0);
        
        if (!is_hidden){
            if (name[0] != '.' && (is_folder || filter(name, size))){
                if (info_ptr + 1 > info_ptr_end || char_ptr + size + 1 > char_ptr_end){
                    memset(&list, 0, sizeof(list));
                    end_temp_memory(part_reset);
                    FindClose(search);
                    return(list);
                }
                
                info_ptr->name = char_ptr;
                info_ptr->len = size;
                info_ptr->is_folder = is_folder;
                
                memmove(char_ptr, name, size*sizeof(*name));
                char_ptr[size] = 0;
                
                char_ptr += size + 1;
                ++info_ptr;
                ++adjusted_file_count;
            }
        }
        
        more_files = FindNextFile(search, &find_data);
    }while(more_files);
    FindClose(search);
    
    list.info = info_ptr_base;
    list.count = adjusted_file_count;
    list.final_length = final_length;
    memcpy(list.final_name, final_name, list.final_length*sizeof(*final_name));
    list.final_name[list.final_length] = 0;
    
    return(list);
}
//// WINDOWS END ////

#elif defined(IS_LINUX) || defined(IS_MAC)

//// UNIX BEGIN ////
static Cross_Platform_File_List
get_file_list(Partition *part, Filename_Character *directory, File_Filter *filter){
    if (part == 0){
        fprintf(stdout, "fatal error: NULL part passed to %s\n", __FUNCTION__);
        exit(1);
    }
    if (directory == 0){
        fprintf(stdout, "fatal error: NULL dir passed to %s\n", __FUNCTION__);
        exit(1);
    }
    
    DIR *dir_handle = opendir(directory);
    if (dir_handle == 0){
        fprintf(stdout, "fatal error: could not open directory handle\n");
        if (sizeof(*directory) == 2){
            fprintf(stdout, "%ls\n", (wchar_t*)directory);
        }
        else{
            fprintf(stdout, "%s\n", (char*)directory);
        }
        exit(1);
    }
    
    Filename_Character final_name[4096];
    int32_t final_length = str_size(directory);
    if (final_length + 1 > sizeof(final_name)){
        fprintf(stdout, "fatal error: path name too long for local buffer\n");
        exit(1);
    }
    memcpy(final_name, directory, final_length + 1);
    
    int32_t character_count = 0;
    int32_t file_count = 0;
    for (struct dirent *entry = readdir(dir_handle);
         entry != 0;
         entry = readdir(dir_handle)){
        Filename_Character *name = entry->d_name;
        
        int32_t size = 0;
        for(;name[size];++size);
        
        bool32 is_folder = false;
        if (entry->d_type == DT_LNK){
            struct stat st;
            if (stat(entry->d_name, &st) != -1){
                is_folder = S_ISDIR(st.st_mode);
            }
        }
        else{
            is_folder = (entry->d_type == DT_DIR);
        }
        
        if (name[0] != '.' && (is_folder || filter(name, size))){
            ++file_count;
            character_count += size + 1;
        }
    }
    
    Cross_Platform_File_List list = {0};
    Temp_Memory part_reset = begin_temp_memory(part);
    
    int32_t rounded_char_size = (character_count*sizeof(Filename_Character) + 7)&(~7);
    int32_t memsize = rounded_char_size + file_count*sizeof(Cross_Platform_File_Info);
    void *mem = push_array(part, uint8_t, memsize);
    if (mem == 0){
        fprintf(stdout, "fatal error: not enough memory on the partition for a file list.\n");
        exit(1);
    }
    
    Filename_Character *char_ptr = (Filename_Character*)mem;
    Cross_Platform_File_Info *info_ptr = (Cross_Platform_File_Info*)((uint8_t*)mem + rounded_char_size);
    
    Filename_Character *char_ptr_end = (Filename_Character*)info_ptr;
    Cross_Platform_File_Info *info_ptr_end = info_ptr + file_count;
    
    Cross_Platform_File_Info *info_ptr_base = info_ptr;
    
    rewinddir(dir_handle);
    
    int32_t adjusted_file_count = 0;
    for (struct dirent *entry = readdir(dir_handle);
         entry != 0;
         entry = readdir(dir_handle)){
        Filename_Character *name = entry->d_name;
        
        int32_t size = 0;
        for(;name[size];++size);
        
        bool32 is_folder = false;
        if (entry->d_type == DT_LNK){
            struct stat st;
            if (stat(entry->d_name, &st) != -1){
                is_folder = S_ISDIR(st.st_mode);
            }
        }
        else{
            is_folder = (entry->d_type == DT_DIR);
        }
        
        if (name[0] != '.' && (is_folder || filter(name, size))){
            if (info_ptr + 1 > info_ptr_end || char_ptr + size + 1 > char_ptr_end){
                memset(&list, 0, sizeof(list));
                end_temp_memory(part_reset);
                closedir(dir_handle);
                return(list);
            }
            
            info_ptr->name = char_ptr;
            info_ptr->len = size;
            info_ptr->is_folder = is_folder;
            
            memmove(char_ptr, name, size*sizeof(*name));
            char_ptr[size] = 0;
            
            char_ptr += size + 1;
            ++info_ptr;
            ++adjusted_file_count;
        }
    }
    closedir(dir_handle);
    
    list.info = info_ptr_base;
    list.count = adjusted_file_count;
    list.final_length = final_length;
    memcpy(list.final_name, final_name, list.final_length*sizeof(*final_name));
    list.final_name[list.final_length] = 0;
    
    return(list);
}
//// UNIX END ////

#else
# error metdata generator not supported on this platform
#endif

static String
file_dump(Partition *part, char *name){
    String text = {0};
    
    FILE *file = fopen(name, "rb");
    if (file != 0){
        fseek(file, 0, SEEK_END);
        text.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        text.memory_size = text.size + 1;
        text.str = push_array(part, char, text.memory_size);
        push_align(part, 8);
        if (text.str == 0){
            fprintf(stdout, "fatal error: not enough memory in partition for file dumping");
            exit(1);
        }
        fread(text.str, 1, text.size, file);
        terminate_with_null(&text);
        fclose(file);
    }
    
    return(text);
}

#endif

// BOTTOM
