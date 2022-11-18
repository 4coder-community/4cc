/*

In order to avoid run-time font interpretation (expensive)
and potential copy right infringement (actually expensive)
it is best for us bake fonts into prepacked bitmaps.

I will output a font as a specialized format that includes
the font as a bitmap format texture atlas and the metadata
for rendering the font.

-Allen
15.07.2016

*/


// TOP

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define FSTRING_INLINE static
#define FSTRING_IMPLEMENTATION
//#include "4tech_string.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct Get_Files_Result{
    FILE *src;
    FILE *dst;
    int32_t successful_open;
};

static struct Get_Files_Result
get_files(char *source_file_name){
    struct Get_Files_Result result = {0};
    
    char *file_name_base = source_file_name;
    String file_name = make_string_slowly(file_name_base);
    
    String file_name_ext = file_extension(file_name);
    
    if (match_sc(file_name_ext, "ttf")){
        FILE *source_file = fopen(file_name_base, "rb");
        
        if (source_file){
            int32_t dest_file_name_size = file_name.size*2 + 1;
            char *dest_file_name_base = (char*)malloc(dest_file_name_size);
            String dest_file_name = make_string_max(dest_file_name_base, 0, dest_file_name_size);
            
            copy_ss(&dest_file_name, file_name);
            remove_extension(&dest_file_name);
            append_sc(&dest_file_name, ".bpft");
            
            if (terminate_with_null(&dest_file_name)){
                FILE *dest_file = fopen(dest_file_name_base, "wb");
                
                if (dest_file){
                    result.src = source_file;
                    result.dst = dest_file;
                    result.successful_open = 1;
                }
            }
            
            free(dest_file_name_base);
            
            if (!result.successful_open){
                fclose(source_file);
            }
        }
    }
    
    return(result);
}

static String
dump_file(FILE *file){
    String result = {0};
    
    if (file){
        int32_t pos = ftell(file);
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        
        result.memory_size = result.size + 1;
        result.str = (char*)malloc(result.memory_size);
        
        if (result.str){
            fseek(file, 0, SEEK_SET);
            fread(result.str, 1, result.size, file);
            result.str[result.size] = 0;
        }
        else{
            result.size = 0;
            result.memory_size = 0;
        }
        
        fseek(file, pos, SEEK_SET);
    }
    
    return(result);
}

struct Glyph_Meta_Data{
    uint16_t x0;
    uint16_t y0;
    uint16_t x1;
    uint16_t y1;
    float xoff;
    float yoff;
    float advance;
};

struct Font_Meta_Data{
    struct Glyph_Meta_Data glyphdata[128];
    float pull_up;
    int32_t texture_w;
    int32_t texture_h;
};

int main(int argc, char **argv){
    if (argc != 2){
        printf("usage:\n\t%s <filename>\n", argv[0]);
        exit(1);
    }
    
    struct Get_Files_Result files = get_files(argv[1]);
    
    if (files.successful_open){
        String src_data = dump_file(files.src);
        
        if (src_data.str){
            stbtt_fontinfo fontinfo = {0};
            int32_t ascent = 0;
            float scale = 0;
            float pixel_height = 16.f;
            
            stbtt_InitFont(&fontinfo, src_data.str, 0);
            scale = stbtt_ScaleForPixelHeight(&fontinfo, pixel_height);
            stbtt_GetFontVMetrics(&fontinfo, &ascent, 0, 0);
            
            
        }
        
        fclose(files.dst);
        fclose(files.src);
    }
    else{
        printf("error: opening files for the operation\n");
        exit(1);
    }
    
    return(0);
}


// BOTTOM

