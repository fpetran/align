// Copyright 2012 Florian Petran
#include"string_impl.h"

#ifdef ALIGN_HAS_UCI_STRING

void printString(const string_impl& str) {
    static char out[256];
    out[str.extract(0, 99, out)] = 0;

    printf("%s\n", out);
}
#endif

bool has_alpha(const string_impl& str) {
    for (int i = 0; i < str.length(); ++i)
        if (check_if_alpha(str[i]))
            return true;
    return false;
}
