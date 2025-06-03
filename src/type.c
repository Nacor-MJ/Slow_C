#include "../include/slow_c.h"

#ifndef TYPE_C
#define TYPE_C

#define td_none {0}
Type *ty_none = &(Type){TY_NONE, 0, 0, td_none};

Type *ty_void = &(Type){TY_VOID, 1, 1, td_none};
Type *ty_bool = &(Type){TY_BOOL, 1, 1, td_none};

Type *ty_char = &(Type){TY_CHAR, 1, 1, td_none};
Type *ty_int = &(Type){TY_INT, 4, 4, td_none};

Type *ty_float = &(Type){TY_FLOAT, 4, 4, td_none};

#endif
