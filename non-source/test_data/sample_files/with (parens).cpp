/*

This is a file meant to be built in order to create 
a tricky test for the jump to parser, by placing
parens inside the file name.  Therefore it must
trigger a number of compilation errors.

It should be accompanied with the approprioate build.bat

*/

struct Important_Thing{
    char *ptr;
};

Important_Thing
foo(int x){
    Important_Thing thing;
    thing.ptr = x;
    thing.bar = foo;
    return(
}

int
main(int argc, char **argv){
    Important_Thing a = foo(3) = foo(4);
    return(0);
}


