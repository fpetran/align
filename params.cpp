// Copyright 2012 Florian Petran
#include"params.h"

#include<iostream>
#include<string>
#include<utility>
#include<stdexcept>
#include<boost/program_options.hpp> // NOLINT[build/include_order]
#include"align_config.h"

using std::string;
using std::pair;

namespace Align {

Params* Params::_instance = new Params;

Params::Params()
    : _closeness(ALIGN_DEFAULT_CLOSENESS),
      _monotony(ALIGN_DEFAULT_MONOTONY),
      _max_skip(ALIGN_DEFAULT_MAX_SKIP),
      _dict_base(ALIGN_DEFAULT_DICT_BASE)
    {}

Params* Params::get() {
    return _instance;
}

int Params::closeness() {
    return _closeness;
}

bool Params::monotony() {
    return _monotony;
}

void Params::set_closeness(int what) {
    _closeness = what;
}

void Params::set_monotony(bool disabled) {
    _monotony = !disabled;
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

pair<string, string> Params::parse(int argc, char* argv[]) {
    namespace cfg = boost::program_options;
    cfg::options_description desc
        (static_cast<std::string>("pAlign v")
         + ALIGN_VERSION
         + "\nAllowed Options");
    bool disable_monotony;
    desc.add_options()
        ("help,h", "display this helpful message")
        ("source,e", cfg::value<std::string>(), "source text to align")
        ("target,f", cfg::value<std::string>(), "target text to align to")
        ("closeness,c",
          cfg::value<int>(&_closeness)->default_value(ALIGN_DEFAULT_CLOSENESS),
          "closeness criteria")
        ("skip,k",
          cfg::value<int>(&_max_skip)->default_value(ALIGN_DEFAULT_MAX_SKIP),
          "max skip value")
        ("dictionary,D", cfg::value<std::string>(&_dict_base)
                              ->default_value(ALIGN_DEFAULT_DICT_BASE),
          "base directory for dictionaries")
        ("no-monotony,M", cfg::bool_switch(&disable_monotony)
                              ->default_value(!ALIGN_DEFAULT_MONOTONY),
          "disable monotony constraint")
        ; // NOLINT[whitespace/semicolon]
    // needs to be read separately, since the switch has opposite
    // meanings for true and false.
    _monotony = !disable_monotony;

    cfg::variables_map m;
    cfg::store(cfg::parse_command_line(argc, argv, desc), m);
    cfg::notify(m);

    if (m.count("help")) {
        std::cout << desc << std::endl;
        throw std::runtime_error("");
    }

    if (!m.count("source") || !m.count("target")) {
        std::cout << desc << std::endl;
        throw std::runtime_error("Not enough arguments!");
    }

    return make_pair(m["source"].as<string>(),
                     m["target"].as<string>());
}
}

