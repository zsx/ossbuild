FreeType - made 1 change to include/config/ftoption.h (line 229): made it: 
#define  FT_EXPORT(x)       __declspec( dllexport ) x
That's so it will generate exportable funcs for use in a dll.