/*
Check potential issue with preprocessor directive parsing
*/

#ifinternal && defined(stuff)

#if internal && defined(stuff)
