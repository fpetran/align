// Copyright 2012 Florian Petran
#include<iostream>
#include<string>

#ifndef ALIGN_USE_BUILTIN_PARAM
#include<boost/program_options.hpp>
namespace po = boost::program_options;
#endif

#include"align.h"
#include"align_config.h"
#include"params.h"


int main(int argc, char* argv[]) {

#ifndef ALIGN_USE_BUILTIN_PARAM
    po::options_description desc
        ( static_cast<std::string>("pAlign v")
          + ALIGN_VERSION
          + "\nAllowed options" );

    std::string e_name, f_name, dict_base;
    unsigned int closeness, skip;

    desc.add_options()
        ( "help,h",
          "display this helpful message" )
        ( "source,e",
          po::value<std::string>(&e_name),
          "source text to align" )
        ( "target,f",
          po::value<std::string>(&f_name),
          "target text to align to" )
        ( "closeness,c",
          po::value<unsigned int>(&closeness)->default_value(ALIGN_DEFAULT_CLOSENESS),
          "closeness criteria" )
        ( "skip,k",
          po::value<unsigned int>(&skip)->default_value(ALIGN_DEFAULT_MAX_SKIP),
          "max skip value" )
        ( "dictionary,D",
          po::value<std::string>(&dict_base),
          "base directory for dictionaries" )
        ;
    // 
    // call a function 
    // ( "closeness,c",
    //   po::value<unsigned int>()
    //   ->notifier( &(Params::get()->set_closeness) ),
    //   "closeness criteria" )

    po::variables_map m;
    po::store( po::parse_command_line( argc, argv, desc ), m );
    // TODO make config file name variable
    //po::store( po::parse_config_file( "align.cfg", desc ), m );
    po::notify(m);

    if (m.count("skip"))
        Params::get()->set_max_skip(skip);
    if (m.count("closeness"))
        Params::get()->set_closeness(closeness);
    if (m.count("dictionary"))
        Params::get()->set_dict_base(dict_base);

    if (m.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (!m.count("source") || !m.count("target")) {
        std::cout << "Not enough arguments!" << std::endl
            << desc << std::endl;
        return 1;
    }
#else
    Params::get()->set_dict_base( ALIGN_DICT_BASE );
#endif // ALIGN_USE_BUILTIN_PARAM


    try {
        Dictionary dict(
#ifndef ALIGN_USE_BUILTIN_PARAM
                m["source"].as<std::string>().c_str(),
                m["target"].as<std::string>().c_str()
#else
                ALIGN_TEST_E, ALIGN_TEST_F
#endif // ALIGN_USE_BUILTIN_PARAM
                );

        Candidates c(dict);
        c.collect();
        SequenceContainer s(c);
        s.make();
    }
    catch(std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
    //Dictionary dict( e_name, f_name );
}
