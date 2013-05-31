// Copyright 2012 Florian Petran
#ifndef PARAMS_H_
#define PARAMS_H_
#include<string>
#include<utility>
#include<iostream>
#include<boost/program_options.hpp> // NOLINT[build/include_order]
#include"align_config.h"

namespace Align {

/// Hold parameters for alignment.
/*! A singleton that encapsulates all parameters.
 *
 *  We're using a lazy Meyers singleton, since apparently
 *  those are threadsafe in C++11.
 *
 *  It also takes care of the parsing of options from the
 *  command line, and maybe eventually from a config file.
 **/
class Params {
    public:
        Params(const Params& p) = delete;
        const Params& operator=(const Params& p) = delete;

        static Params& get();

        int max_skip();
        int closeness();
        bool monotony();
        const std::string& dict_base();

        void set_max_skip(int value);
        void set_closeness(int value);
        void set_monotony(bool disabled);
        void set_dict_base(const std::string& dirname);

        /// Parse command line for parameters. Set Params members as
        /// needed, and return a pair of file names (e_name, f_name).
        /// If --help is specified, or if one or both source and target
        /// files are missing, runtime_error is thrown, which should
        /// be caught by the client.
        /// The options descriptions is shown on stdout either way.
        std::pair<std::string, std::string> parse(int argc, char* argv[]);

    private:
        Params() = default;

        int  _closeness         = ALIGN_DEFAULT_CLOSENESS;
        bool _monotony          = ALIGN_DEFAULT_MONOTONY;
        int _max_skip           = ALIGN_DEFAULT_MAX_SKIP;
        std::string _dict_base  = ALIGN_DEFAULT_DICT_BASE;
};
}  // namespace Align

#endif  // PARAMS_H_

