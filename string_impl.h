// Copyright 2012 Florian Petran
#ifndef ALIGN_STRING_IMPL_H
#define ALIGN_STRING_IMPL_H
#include<cstdlib>
#include<string>
#include<ostream>

// XXX too lazy to do it properly
#define ALIGN_HAS_UCI_STRING

// all string implementation stuff goes here
// that goes for the strings that are actually in the file
// and the functions associated with them, NOT strings from the
// options, or filenames. Those are always std strings.
//
// the reason we're using butt ugly C style functions here
// instead of nice, clean C++ objects, is
// so that it will work well with UnicodeString, which has a
// disgraceful interface.


#ifdef ALIGN_HAS_UCI_STRING
#include<unicode/unistr.h>
#include<unicode/uchar.h>

typedef UnicodeString string_impl;
typedef UChar32 char_impl;
typedef int string_size;

inline void lower_case(string_impl* str)
    { str->toLower(); }
static const string_size string_npos = -1;
inline void extract(const string_impl& str, int from, int to, string_impl* out)
    { str.extractBetween(from, to, *out); }
inline string_size string_find(const string_impl& me, const char* you)
    { return me.indexOf(you); }

inline bool check_if_alpha(char_impl c) {
    return u_isalpha(c);
}

// printString for UCI needs a local char_ptr, so it's not inlined
void printString(const string_impl&);

const char* to_cstr(const string_impl&);

std::ostream& operator<<(std::ostream& strm, const string_impl& ustr);

#else

#include<algorithm>

typedef std::string string_impl;
typedef char char_impl;
typedef size_t string_size;

inline void lower_case(string_impl* str)
    { std::transform(str->begin(), str->end(), str->begin(), ::tolower); }

static const size_t string_npos = std::string::npos;

inline void extract(const string_impl& str, int from, int to, string_impl* out)
    { *out = str.substr(from, to); }

inline string_size string_find(const string_impl& me, const char* you)
    { return me.find(you); }

inline void printString(const string_impl& str)
    { printf("%s\n", str.c_str()); }

inline bool check_if_alpha(char_impl c) {
    return isalpha(c);
}

inline const char* to_cstr(const string_impl& str) {
    return str.c_str();
}

#endif // ALIGN_HAS_UCI_STRING

bool has_alpha(const string_impl& str);

#endif // ALIGN_STRING_IMPL_H
