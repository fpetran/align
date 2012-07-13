// Copyright 2012 Florian Petran
#include"params.h"
#include<string>

using std::string;

Params* Params::_instance = new Params;

Params::Params() :
    _closeness(2), _max_skip(1), _dict_base(".") {}

Params* Params::get() {
    return _instance;
}

int Params::closeness() {
    return _closeness;
}
void Params::set_closeness(int what) {
    _closeness = what;
}
void Params::set_max_skip(int what) {
    _max_skip = what;
}
void Params::set_dict_base(const string& what) {
    _dict_base = what;
}
int Params::max_skip() {
    return _max_skip;
}
const string& Params::dict_base() {
    return _dict_base;
}

