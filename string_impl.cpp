// Copyright 2012 Florian Petran
#include"string_impl.h"

#ifdef ALIGN_HAS_UCI_STRING

char* to_cstr(const string_impl& str) {
    static char out[256];
    out[str.extract(0,99, out)] = 0;
    return out;
}

void printString(const string_impl& str) {
    char* out = to_cstr(str);
    printf("%s\n", out);
}

#endif

bool has_alpha(const string_impl& str) {
    for (int i = 0; i < str.length(); ++i)
        if (check_if_alpha(str[i]))
            return true;
    return false;
}
