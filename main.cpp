// Copyright 2012 Florian Petran
#include<string>
#include<utility>
#include<stdexcept>
#include<iostream>

#include"align.h"

int main(int argc, char* argv[]) {
    Align::Params* par = Align::Params::get();
    std::pair<std::string, std::string> files;
    try {
        files = par->parse(argc, argv);
    } catch(std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    try {
        const std::string &e_name = files.first,
                          &f_name = files.second;

        Align::DictionaryFactory* df = Align::DictionaryFactory::get_instance();
        const Align::Dictionary* dict = df->get_dictionary(e_name, f_name);
        Align::Candidates c(*dict);
        c.collect();

        Align::SequenceContainer sc(&c);
        sc.initial_sequences()
          .expand_sequences();

        const Align::Dictionary* rdict = df->get_dictionary(f_name, e_name);
        Align::Candidates rc(*rdict);
        Align::SequenceContainer rsc(&rc);
        rsc.initial_sequences()
           .expand_sequences();
        Align::Hypothesis *result  = sc.get_result(),
                   *rresult = rsc.get_result();
        rresult->reverse();
        result->munch(rresult);

        sc.merge_sequences()
          .collect_scores()
          .get_topranking();

        for (Align::Sequence* seq : *result)
            std::cout << *seq << std::endl;
    }
    catch(std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

