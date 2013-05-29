// Copyright 2013 Florian Petran
#include"mkdict.h"
#include<utility>
#include<string>
#include<vector>
#include<stdexcept>
#include"bi-sim.h"
#include"align_config.h"
#include<boost/program_options.hpp> // NOLINT[build/include_order]

using std::vector;
using std::pair;
using std::make_pair;
using std::string;
using std::thread;

namespace po = boost::program_options;

namespace {
struct opts {
    int wordlength_threshold;
    bi_sim::num_ty cognate_threshold;
    vector<string> input_files;
} myopts;

pair<bool, int> get_options(opts* myopts,
                            int argc, char* argv[]) {
    po::options_description desc
        (static_cast<std::string>("pAlign v")
            + ALIGN_VERSION + " dictionary induction\n"
            + "Allowed options");

    desc.add_options()
        ("help,h",
         "display this helpful message")
        ("wordlength,w",
         po::value<int>(&(myopts->wordlength_threshold))
            ->default_value(DICTIONARY_WORDLENGTH_THRESHOLD),
         "Cutoff below which words won't be considered for cognates")
        ("threshold,t",
         po::value<bi_sim::num_ty>(&(myopts->cognate_threshold))
                                  ->default_value(DICTIONARY_COGNATE_THRESHOLD),
         "Minimum bi-sim value for two words to be considered cognates")
        ; //NOLINT
    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("input-file",
         po::value<vector<string>>(&(myopts->input_files)),
         "input file");
    po::positional_options_description p;
    p.add("input-file", -1);
    po::options_description cmdline_opts;
    cmdline_opts.add(desc).add(hidden);

    po::variables_map m;
    po::store(po::command_line_parser(argc, argv).
                  options(cmdline_opts).positional(p).run(), m);
    po::notify(m);

    if (m.count("help")) {
        std::cout << desc << std::endl;
        return make_pair(true, 0);
    }

    if (!m.count("input-file") || myopts->input_files.size() <= 1) {
        std::cout << "Error: Not enough input files specified!" << std::endl
                  << desc << std::endl;
        return make_pair(true, 1);
    }

    return make_pair(false, 0);
}
}  // namespace

int main(int argc, char* argv[]) {
    pair<bool, int> quitcode = get_options(&myopts, argc, argv);
    if (quitcode.first)
        return quitcode.second;

    try {
        DictionaryInducer::FileSet files;
        DictionaryInducer::ResultSet results;

        for (const string& fname : myopts.input_files) {
            files[fname] = new DictionaryInducer::File();
            thread(DictionaryInducer::file_reader,
                   fname, files).detach();
        }

        for (auto e_name = myopts.input_files.begin();
             e_name != myopts.input_files.end(); ++e_name)
            for (auto f_name = e_name + 1;
                 f_name != myopts.input_files.end(); ++f_name)
                thread(DictionaryInducer::fileset_processor,
                       *e_name, *f_name,
                       files, &results,
                       myopts.wordlength_threshold,
                       myopts.cognate_threshold).detach();

        thread(DictionaryInducer::result_outputter, &results).join();

        for (pair<string, DictionaryInducer::File*> f : files)
            delete f.second;
    } catch(std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

