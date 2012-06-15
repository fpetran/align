#ifndef BI_SIM_H
#define BI_SIM_H
#include<vector>
#include<unicode/unistr.h>

namespace bi_sim {
    typedef double num_ty;
    typedef icu::UnicodeString String;

    num_ty bi_sim( const String&, const String& );
}


#endif // BI_SIM_H
