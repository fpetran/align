// Copyright 2012 Florian Petran
#include"dictionary.h"

#include<string>
#include<stdexcept>
#include<utility>
#include<map>
#include<list>

#include"containers.h"
#include"params.h"

using std::ifstream;
using std::string;
using std::pair;
using std::list;
using std::map;
using std::runtime_error;

namespace Align {

////////////////////////// DictionaryFactory //////////////////////////////////

DictionaryFactory* DictionaryFactory::_instance = new DictionaryFactory();

DictionaryFactory* DictionaryFactory::get_instance() {
    return _instance;
}

DictionaryFactory::~DictionaryFactory() {
    for (pair<const pair<string, string>, Dictionary*>& dict_entry
            : dictionaries)
        delete dict_entry.second;
    for (pair<const string, Text*>& text_entry : texts)
        delete text_entry.second;
}

DictionaryFactory::DictionaryFactory()
    : index_filename("") { }

namespace {
    inline string basename(const string& str) {
        return(str.substr(str.rfind("/") + 1, str.length() - 1));
    }
}

const Dictionary* DictionaryFactory::get_dictionary(const string& e,
                                                    const string& f) {
    pair<string, string> transl_pair = make_pair(basename(e),
                                                 basename(f));

    map<pair<string, string>, Dictionary*>::iterator
        dict_entry = dictionaries.find(transl_pair);

    if (dict_entry == dictionaries.end()) {
        if (index_filename == "")
            index_filename  = Params::get()->dict_base() + "/INDEX";
        dictionaries[transl_pair] = new Dictionary();
        dictionaries[transl_pair]->set_texts(get_text(e), get_text(f));
        dictionaries[transl_pair]->open(locate_dictionary_file(basename(e),
                                                               basename(f)));
        return dictionaries[transl_pair];
    }

    return dict_entry->second;
}

Text* DictionaryFactory::get_text(const string& fname) {
    map<string, Text*>::iterator text_entry = texts.find(fname);

    if (text_entry == texts.end()) {
        texts[fname] = new Text(fname);
        return texts[fname];
    }

    return text_entry->second;
}


string DictionaryFactory::locate_dictionary_file(const string& e_name,
                                                 const string& f_name) {
    ifstream index_file;
    index_file.open(index_filename);

    if (!index_file.is_open())
        throw runtime_error("Index file not found!");

    char c_line[255];
    string line;
    while (!index_file.eof()) {
        index_file.getline(c_line, 255);
        line = c_line;

        size_t e_pos = line.find(e_name);
        size_t f_pos = line.find(f_name);

        if (e_pos != string::npos && f_pos != string::npos && e_pos < f_pos)
            break;
    }

    index_file.clear();
    index_file.close();

    if (line == "")
        throw runtime_error(static_cast<string>("Dictionary entry for ")
                          + e_name
                          + " -> "
                          + f_name
                          + " not found in index file!");

    string filename = Params::get()->dict_base()
                    + line.substr(0, line.find(":"));
    return filename;
}

/////////////////////////// Dictionary ////////////////////////////////////////

Dictionary::Dictionary() {
    factory = DictionaryFactory::get_instance();
}

Dictionary::Dictionary(const string& fname) {
    factory = DictionaryFactory::get_instance();
    open(fname);
}

void Dictionary::open(const string& fname) {
    ifstream dict_file;
    dict_file.open(fname);

    if (!dict_file.is_open())
        throw runtime_error(
                static_cast<string>("Dictionary file not found: ") + fname);

    read(&dict_file);

    dict_file.clear();
    dict_file.close();
}

void Dictionary::read(ifstream* file) {
    string_impl line;
    char c_line[255];

    while (!file->eof()) {
        file->getline(c_line, 255);
        line = c_line;

        // comment line in dictionaries are supposed to start
        // with ###
        if (string_find(line, "###") == 0)
            continue;

        string_size div = string_find(line, " = ");
        if (div == string_npos)
            continue;

        string_impl sword, tword;
        extract(line, 0, div, &sword);
        lower_case(&sword);
        extract(line, div + 3, line.length(), &tword);
        lower_case(&tword);

        // i'm a bit unsure what needs to be done if a
        // dictionary entry doesn't occur in the texts, but
        // probably it's safe to assume we don't need a
        // dictionary entry for it then.
        if (_f->_types.find(tword) == _f->_types.end()
         || _e->_types.find(sword) == _e->_types.end())
            continue;
        WordType* ft = _f->_types.at(tword);
        WordType* et = _e->_types.at(sword);

        if (std::find((*this)[*et].begin(),
                      (*this)[*et].end(),
                      *ft) != (*this)[*et].end())
            continue;

        (*this)[*et].push_back(*ft);
    }
}

const list<WordType>& Dictionary::lookup(const WordToken& lemma) const {
    if (&lemma.get_text() != _e)
        throw runtime_error("Text of word to look up doesn't match e");

    // empty list - does it need to be kept separately? will it cause
    // an exception or memory corruption to look up a word that doesn't exit?
    if (!this->has(lemma) || !has_alpha(lemma.get_str()))
        return empty_entry;

    // this needs to be at(), because operator[] doesn't return
    // const. if the range checking proves to be too expensive,
    // change to operator[] and const_cast
    return (*this).at(lemma.get_type());
}


bool Dictionary::has(const WordToken& lemma) const {
    if (&lemma.get_text() != _e)
        throw runtime_error("Text of word to look up doesn't match e");

    return (*this).count(lemma.get_type()) >= 1;
}
}

