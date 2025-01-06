/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.10.2019
 *
 * Parser that extracts an API from C++ source code.
 *
 */

// TOP

#include "4coder_base_types.h"
#include "4coder_token.h"
#include "generated/lexer_cpp.h"
#include "4ed_api_definition.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"
#include "4coder_token.cpp"
#include "generated/lexer_cpp.cpp"
#include "4coder_file.cpp"
#include "4ed_api_definition.cpp"
#include "4ed_api_parser.cpp"

#include <stdio.h>

////////////////////////////////

int
main(int argc, char **argv){
    Arena arena = make_arena_malloc();
    
    if (argc < 2){
        printf("usage: <script> <source> {<source>}\n"
               " source : file to load and parse into the output list\n");
        exit(1);
    }
    
    String_Const_u8 exe = SCu8("4ed_api_parser_main.exe");
    u32 command_line_length = exe.size + 1;
    String_Const_u8 command_line = { 0 };
    
    for (i32 i = 1; i < argc; i+=1){
        command_line_length += cstring_length(argv[i]) + 1;
    }
    
    command_line = push_data(&arena, command_line_length);
    u8* command_line_current = command_line.str;
    u8* command_line_end = command_line.str + command_line.size;
    
    {
        char* c = ( char* ) exe.str;
        
        while ( *c != 0 && command_line_current < command_line_end ) {
            *command_line_current = *c;
            command_line_current++;
            c++;
        }
    }
    
    if ( command_line_current < command_line_end ) {
        *command_line_current = ' ';
        command_line_current++;
    }
    
    API_Definition_List list = {};
    for (i32 i = 1; i < argc; i += 1){
        char *file_name = argv[i];
        
        char* c = file_name;
        while ( *c != 0 && command_line_current < command_line_end ) {
            *command_line_current = *c;
            command_line_current++;
            c++;
        }
        
        if ( command_line_current < command_line_end ) {
            *command_line_current = ' ';
            command_line_current++;
        }
        
        FILE *file = fopen(file_name, "rb");
        if (file == 0){
            printf("error: could not open input file: '%s'\n", argv[i]);
            continue;
        }
        
        String_Const_u8 text = data_from_file(&arena, file);
        fclose(file);
        
        if (text.size > 0){
            api_parse_source_add_to_list(&arena, SCu8(file_name), text, &list);
        }
    }
    
    for (API_Definition *node = list.first;
         node != 0;
         node = node->next){
        api_definition_generate_api_includes(&arena, node, GeneratedGroup_Custom, APIGeneration_NoAPINameOnCallables, command_line);
    }
}

// BOTTOM
