// Copyright 2012 Florian Petran
#include"string_impl.h"

#ifdef USE_ICU_STRING
#include<string>
#include<ostream>

const char* to_cstr(const string_impl& str) {
    // one problem with this function:
    // it returns a char ptr, so if i call it
    // twice, both char ptr will have the value
    // of the second call. might be fixable by
    // making the ptr non static, but idk what
    // that will do to performance.
    static char out[256];
    out[str.extract(0, 99, out)] = 0;
    return out;
}

std::ostream& operator<<(std::ostream& strm, const string_impl& ustr) {
    std::string str = to_cstr(ustr);
    strm << str;
    return strm;
}

#endif  // USE_ICU_STRING

bool has_alpha(const string_impl& str) {
    for (string_size i = 0; i < str.length(); ++i)
        if (check_if_alpha(str[i]))
            return true;
    return false;
}

