#include"params.h"
#include<string>

Params* Params::_instance = new Params;

Params::Params() {
    // default params
    _closeness  = 2;
    _max_skip   = 1;
    _dict_base  = "";
    ;
}

Params* Params::get() {
    return _instance;
}

unsigned int Params::closeness() { return _closeness; }
void Params::set_closeness( unsigned int what )
    { _closeness = what; }
void Params::set_max_skip( unsigned int what )
    { _max_skip = what; }
void Params::set_dict_base( const std::string& what )
    { _dict_base = what; }
unsigned int Params::max_skip() { return _max_skip; }
const std::string& Params::dict_base() { return _dict_base; }
