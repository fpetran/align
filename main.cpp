// Copyright 2012 Florian Petran
#include<string>
#include<iostream>

#include<boost/program_options.hpp>

#include"align.h"
#include"align_config.h"

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    po::options_description desc
        (static_cast<std::string>("pAlign v")
          + ALIGN_VERSION
          + "\nAllowed options");

    std::string e_name, f_name, dict_base;
    int closeness, skip;

    desc.add_options()
        ("help,h",
          "display this helpful message")
        ("source,e",
          po::value<std::string>(&e_name),
          "source text to align")
        ("target,f",
          po::value<std::string>(&f_name),
          "target text to align to")
        ("closeness,c",
          po::value<int>(&closeness)->default_value(ALIGN_DEFAULT_CLOSENESS),
          "closeness criteria")
        ("skip,k",
          po::value<int>(&skip)->default_value(ALIGN_DEFAULT_MAX_SKIP),
          "max skip value")
        ("dictionary,D",
          po::value<std::string>(&dict_base),
          "base directory for dictionaries")
        ;
    // call a function
    // ( "closeness,c",
    //   po::value<int>()
    //   ->notifier( &(Params::get()->set_closeness) ),
    //   "closeness criteria" )

    po::variables_map m;
    po::store(po::parse_command_line(argc, argv, desc), m);
    // TODO(fpetran) make config file name variable
    //po::store(po::parse_config_file("align.cfg", desc), m);
    po::notify(m);
    Params* par = Params::get();

    if (m.count("skip"))
        par->set_max_skip(skip);
    if (m.count("closeness"))
        par->set_closeness(closeness);
    if (m.count("dictionary"))
        par->set_dict_base(dict_base);

    if (m.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (!m.count("source") || !m.count("target")) {
        std::cout << "Not enough arguments!" << std::endl
            << desc << std::endl;
        return 1;
    }

    try {
        std::string e_name = m["source"].as<std::string>(),
                    f_name = m["target"].as<std::string>();

        DictionaryFactory* df = DictionaryFactory::get_instance();
        const Dictionary* dict = df->get_dictionary(e_name, f_name);
        Candidates c(*dict);
        c.collect();

        SequenceContainer sc(&c);
        sc.initial_sequences()
          .expand_sequences();

        const Dictionary* rdict = df->get_dictionary(f_name, e_name);
        Candidates rc(*rdict);
        SequenceContainer rsc(&rc);
        rsc.initial_sequences()
           .expand_sequences();
        Hypothesis  *rresult = rsc.get_result(),
                    *result = sc.get_result();
        rresult->reverse();
        result->munch(rresult);

        sc.merge_sequences()
          .collect_scores();
        sc.get_topranking();

        for (Sequence* seq : *result) {
            std::cout << "{ ";
            for (const Pair& pair : *seq)
                std::cout << pair;
            std::cout << " }"
                << " Length: " << seq->length()
                << std::endl;
        }
    }
    catch(std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

