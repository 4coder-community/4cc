/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

struct meta_struct
{
    char *Name;
    meta_struct *Next;
};
static meta_struct *FirstMetaStruct;

static char *
ReadEntireFileIntoMemoryAndNullTerminate(char *FileName)
{
    char *Result = 0;
    
    FILE *File = fopen(FileName, "r");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        size_t FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);

        Result = (char *)malloc(FileSize + 1);
        fread(Result, FileSize, 1, File);
        Result[FileSize] = 0;
        
        fclose(File);
    }

    return(Result);
}

enum token_type
{
    Token_Unknown,
    
    Token_OpenParen,    
    Token_CloseParen,    
    Token_Colon,    
    Token_Semicolon,    
    Token_Asterisk,    
    Token_OpenBracket,    
    Token_CloseBracket,    
    Token_OpenBrace,    
    Token_CloseBrace,

    Token_String,    
    Token_Identifier,    

    Token_EndOfStream,
};
struct token
{
    token_type Type;
    
    size_t TextLength;
    char *Text;
};

struct tokenizer
{
    char *At;
};

inline bool
IsEndOfLine(char C)
{
    bool Result = ((C == '\n') ||
                   (C == '\r'));

    return(Result);
}

inline bool
IsWhitespace(char C)
{
    bool Result = ((C == ' ') ||
                   (C == '\t') ||
                   (C == '\v') ||
                   (C == '\f') ||
                   IsEndOfLine(C));

    return(Result);
}

inline bool
IsAlpha(char C)
{
    bool Result = (((C >= 'a') && (C <= 'z')) ||
                   ((C >= 'A') && (C <= 'Z')));

    return(Result);
}

inline bool
IsNumber(char C)
{
    bool Result = ((C >= '0') && (C <= '9'));
    
    return(Result);
}

inline bool
TokenEquals(token Token, char *Match)
{
    char *At = Match;
    for(int Index = 0;
        Index < Token.TextLength;
        ++Index, ++At)
    {
        if((*At == 0) ||
           (Token.Text[Index] != *At))
        {
            return(false);
        }
        
    }

    bool Result = (*At == 0);
    return(Result);
}

static void
EatAllWhitespace(tokenizer *Tokenizer)
{
    for(;;)
    {
        if(IsWhitespace(Tokenizer->At[0]))
        {
            ++Tokenizer->At;
        }
        else if((Tokenizer->At[0] == '/') &&
                (Tokenizer->At[1] == '/'))
        {
            Tokenizer->At += 2;
            while(Tokenizer->At[0] && !IsEndOfLine(Tokenizer->At[0]))
            {
                ++Tokenizer->At;
            }
        }
        else if((Tokenizer->At[0] == '/') &&
                (Tokenizer->At[1] == '*'))
        {
            Tokenizer->At += 2;
            while(Tokenizer->At[0] &&
                  !((Tokenizer->At[0] == '*') &&
                    (Tokenizer->At[1] == '/')))
            {
                ++Tokenizer->At;
            }
            
            if(Tokenizer->At[0] == '*')
            {
                Tokenizer->At += 2;
            }
        }
        else
        {
            break;
        }
    }
}

static token
GetToken(tokenizer *Tokenizer)
{
    EatAllWhitespace(Tokenizer);

    token Token = {};
    Token.TextLength = 1;
    Token.Text = Tokenizer->At;
    char C = Tokenizer->At[0];
    ++Tokenizer->At;
    switch(C)
    {
        case '\0': {Token.Type = Token_EndOfStream;} break;
           
        case '(': {Token.Type = Token_OpenParen;} break;
        case ')': {Token.Type = Token_CloseParen;} break;
        case ':': {Token.Type = Token_Colon;} break;
        case ';': {Token.Type = Token_Semicolon;} break;
        case '*': {Token.Type = Token_Asterisk;} break;
        case '[': {Token.Type = Token_OpenBracket;} break;
        case ']': {Token.Type = Token_CloseBracket;} break;
        case '{': {Token.Type = Token_OpenBrace;} break;
        case '}': {Token.Type = Token_CloseBrace;} break;
            
        case '"':
        {
            Token.Type = Token_String;

            Token.Text = Tokenizer->At;
            
            while(Tokenizer->At[0] &&
                  Tokenizer->At[0] != '"')
            {
                if((Tokenizer->At[0] == '\\') &&
                   Tokenizer->At[1])
                {
                    ++Tokenizer->At;
                }                
                ++Tokenizer->At;
            }

            Token.TextLength = Tokenizer->At - Token.Text;
            if(Tokenizer->At[0] == '"')
            {
                ++Tokenizer->At;
            }
        } break;

        default:
        {
            if(IsAlpha(C))
            {
                Token.Type = Token_Identifier;
                
                while(IsAlpha(Tokenizer->At[0]) ||
                      IsNumber(Tokenizer->At[0]) ||
                      (Tokenizer->At[0] == '_'))
                {
                    ++Tokenizer->At;
                }
                
                Token.TextLength = Tokenizer->At - Token.Text;                
            }
#if 0
            else if(IsNumeric(C))
            {
                ParseNumber();
            }
#endif
            else
            {
                Token.Type = Token_Unknown;
            }
        } break;        
    }

    return(Token);
}

static bool
RequireToken(tokenizer *Tokenizer, token_type DesiredType)
{
    token Token = GetToken(Tokenizer);
    bool Result = (Token.Type == DesiredType);
    return(Result);
}

static void
ParseIntrospectionParams(tokenizer *Tokenizer)
{
    for(;;)
    {
        token Token = GetToken(Tokenizer);
        if((Token.Type == Token_CloseParen) ||
           (Token.Type == Token_EndOfStream))
        {
            break;
        }
    }
}

static void
ParseMember(tokenizer *Tokenizer, token StructTypeToken, token MemberTypeToken)
{
#if 1
    bool Parsing = true;
    bool IsPointer = false;
    while(Parsing)
    {
        token Token = GetToken(Tokenizer);
        switch(Token.Type)
        {
            case Token_Asterisk:
            {
                IsPointer = true;
            } break;

            case Token_Identifier:
            {
                printf("   {%s, MetaType_%.*s, \"%.*s\", (u32)&((%.*s *)0)->%.*s},\n",
                       IsPointer ? "MetaMemberFlag_IsPointer" : "0",
                       MemberTypeToken.TextLength, MemberTypeToken.Text,
                       Token.TextLength, Token.Text,
                       StructTypeToken.TextLength, StructTypeToken.Text,
                       Token.TextLength, Token.Text);                
            } break;

            case Token_Semicolon:
            case Token_EndOfStream:
            {

                Parsing = false;
            } break;
        }
    }
#else
    token Token = GetToken(Tokenizer);
    switch(Token.Type)
    {
        case Token_Asterisk:
        {
            ParseMember(Tokenizer, Token);
        } break;

        case Token_Identifier:
        {
            printf("DEBUG_VALUE(%.*s);\n", Token.TextLength, Token.Text);                
        } break;
    }
#endif
}

static void
ParseStruct(tokenizer *Tokenizer)
{
    token NameToken = GetToken(Tokenizer);
    if(RequireToken(Tokenizer, Token_OpenBrace))
    {
        printf("member_definition MembersOf_%.*s[] = \n", NameToken.TextLength, NameToken.Text);
        printf("{\n");
        for(;;)
        {
            token MemberToken = GetToken(Tokenizer);
            if(MemberToken.Type == Token_CloseBrace)
            {
                break;
            }
            else
            {
                ParseMember(Tokenizer, NameToken, MemberToken);
            }
        }
        printf("};\n");

        meta_struct *Meta = (meta_struct *)malloc(sizeof(meta_struct));
        Meta->Name = (char *)malloc(NameToken.TextLength + 1);
        memcpy(Meta->Name, NameToken.Text, NameToken.TextLength);
        Meta->Name[NameToken.TextLength] = 0;
        Meta->Next = FirstMetaStruct;
        FirstMetaStruct = Meta;
    }
}

static void
ParseIntrospectable(tokenizer *Tokenizer)
{
    if(RequireToken(Tokenizer, Token_OpenParen))
    {
        ParseIntrospectionParams(Tokenizer);

        token TypeToken = GetToken(Tokenizer);
        if(TokenEquals(TypeToken, "struct"))
        {
            ParseStruct(Tokenizer);
        }
        else
        {
            fprintf(stderr, "ERROR: Introspection is only supported for structs right now :(\n");            
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Missing parentheses.\n");
    }
}

int
main(int ArgCount, char **Args)
{
    char *FileNames[] =
    {
        "handmade_sim_region.h",
        "handmade_platform.h",
        "handmade_math.h",
        "handmade_world.h",
    };
    for(int FileIndex = 0;
        FileIndex < (sizeof(FileNames)/sizeof(FileNames[0]));
        ++FileIndex)
    {
        char *FileContents = ReadEntireFileIntoMemoryAndNullTerminate(FileNames[FileIndex]);

        tokenizer Tokenizer = {};
        Tokenizer.At = FileContents;

        bool Parsing = true;
        while(Parsing)
        {
            token Token = GetToken(&Tokenizer);
            switch(Token.Type)
            {
                case Token_EndOfStream:
                {
                    Parsing = false;
                } break;

                case Token_Unknown:
                {
                } break;

                case Token_Identifier:
                {                
                    if(TokenEquals(Token, "introspect"))
                    {
                        ParseIntrospectable(&Tokenizer);
                    }
                } break;
                        
                default:
                {
//                printf("%d: %.*s\n", Token.Type, Token.TextLength, Token.Text);
                } break;
            }
        }
    }

    printf("#define META_HANDLE_TYPE_DUMP(MemberPtr, NextIndentLevel) \\\n");    
    for(meta_struct *Meta = FirstMetaStruct;
        Meta;
        Meta = Meta->Next)
    {
        printf("    case MetaType_%s: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_%s), MembersOf_%s, MemberPtr, (NextIndentLevel));} break; %s\n",
               Meta->Name, Meta->Name, Meta->Name,
               Meta->Next ? "\\" : "");
    }
}
