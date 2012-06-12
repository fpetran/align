// Copyright 2012 Florian Petran
#include"text.h"

#include"containers.h"

#include<string>
#include<vector>

using std::ifstream;
using std::string;

Dictionary::Dictionary(const char* e, const char* f) {
    open(e, f);
}


void Dictionary::get_dict_fname(const string& e_name, const string& f_name) {
    string line = Params::get().dict_base();
    line += "/INDEX";

    char c_line[255];
    ifstream index_file;
    index_file.open(line);

    if (!index_file.is_open())
        throw std::runtime_error(std::string("Index file not found: ") + line);

    while (!index_file.eof()) {
        index_file.getline(c_line, 255);
        line = c_line;

        int e_pos = line.find(e_name);
        int f_pos = line.find(f_name);

        if (e_pos != 0 && f_pos != 0 && e_pos < f_pos)
            break;
    }

    index_file.clear();
    index_file.close();

    _dict_fname = Params::get().dict_base();
    _dict_fname += line.substr(0, line.find(":"));
}

namespace {
    inline string basename(const std::string& str) {
        return(str.substr(str.rfind("/") + 1, str.length() - 1));
    }
}

void Dictionary::open(const char* e_name, const char* f_name) {
    get_dict_fname(basename(e_name), basename(f_name));

    _e = new Text(e_name);
    _f = new Text(f_name);

    string_impl line;
    char c_line[255];

    ifstream dict_file;
    dict_file.open(_dict_fname);

    if (!dict_file.is_open())
        throw std::runtime_error(
                std::string("Dictionary file not found: ") + _dict_fname);

    while (!dict_file.eof()) {
        dict_file.getline(c_line, 255);

        line = c_line;

        string_size div = string_find(line, " = ");
        if (div == string_npos)
            continue;

        string_impl sword, tword;

        extract(line, 0, div, &sword);
        lower_case(&sword);

        extract(line, div + 3, line.length(), &tword);
        lower_case(&tword);

        _storage[ _e->_wordlist[sword] ].push_back( _f->_wordlist[tword] );
    }


    dict_file.close();
}

Dictionary::~Dictionary() {
    delete _e;
    delete _f;
}

TranslationsEntry* Dictionary::lookup(const Word& lemma) const {
    if (lemma._text != _e)
        throw std::runtime_error("Text of word to look up doesn't match e");

    TranslationsEntry *occurrences = new TranslationsEntry;

    for( std::list<Word>::const_iterator tr = _storage.at(lemma).begin();
            tr != _storage.at(lemma).end(); ++tr )
        for( TranslationsEntry::const_iterator ii = _f->index(*tr).begin();
                ii != _f->index(*tr).end(); ++ii )
            occurrences->push_back( *ii );

    return occurrences;
}

bool Dictionary::has(const Word& lemma) const {
    if (lemma._text != _e)
        throw std::runtime_error("Text of word to look up doesn't match e");

    return _storage.count(lemma) >= 1;
}
