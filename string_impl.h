// Copyright 2012 Florian Petran
//
// All string implementation stuff goes here. Provides
// wrapper for all operations and queries related to strings
// so that the actual code has the option of using std::string
// as well as UCI strings.
#ifndef STRING_IMPL_H_
#define STRING_IMPL_H_

#ifdef USE_ICU_STRING
#include<ostream>
#include<unicode/unistr.h> // NOLINT[build/include_order]
#include<unicode/uchar.h>  // NOLINT[build/include_order]

typedef UnicodeString string_impl;
typedef UChar32 char_impl;
typedef int string_size;

inline void lower_case(string_impl* str)
    { str->toLower(); }
inline void upper_case(string_impl* str)
    { str->toUpper(); }
static const string_size string_npos = -1;
inline void extract(const string_impl& str, int from, int to, string_impl* out)
    { str.extractBetween(from, to, *out); }
inline string_size string_find(const string_impl& me, const char* you)
    { return me.indexOf(you); }

inline bool check_if_alpha(char_impl c) {
    return u_isalpha(c);
}

const char* to_cstr(const string_impl&);

std::ostream& operator<<(std::ostream& strm, const string_impl& ustr);

#else  // USE_ICU_STRING
#include<algorithm>
#include<string>

typedef std::string string_impl;
typedef char char_impl;
typedef size_t string_size;

inline void lower_case(string_impl* str)
    { std::transform(str->begin(), str->end(), str->begin(), ::tolower); }

inline void upper_case(string_impl* str)
    { std::transform(str->begin(), str->end(), str->begin(), ::toupper); }

static const size_t string_npos = std::string::npos;

inline void extract(const string_impl& str, int from, int to, string_impl* out)
    { *out = str.substr(from, to); }

inline string_size string_find(const string_impl& me, const char* you)
    { return me.find(you); }

inline bool check_if_alpha(char_impl c) {
    return isalpha(c);
}

inline const char* to_cstr(const string_impl& str) {
    return str.c_str();
}

#endif  // USE_ICU_STRING

bool has_alpha(const string_impl& str);

inline void upper_case(char_impl* c) {
    string_impl s = "";
    s += *c;
    upper_case(&s);
    *c = s[0];
}

#endif  // STRING_IMPL_H_

