// Copyright 2012 Florian Petran
#include"text.h"

#include<cctype>

#include<string>
#include<list>
#include<fstream>
#include<stdexcept>
#include<utility>

#include"string_impl.h"

using std::string;
using std::list;
using std::ifstream;
using std::pair;
using std::out_of_range;
using std::runtime_error;

//////////////////////////////// base Word ////////////////////////////////////

Word::Word(const Text* text) {
    _text = text;
}

const Text& Word::get_text() const {
    return *_text;
}

/////////////////////////////// WordToken /////////////////////////////////////

WordToken::WordToken(const Text* text,
                     list<Sequence*> * seqs,
                     const string_impl* s,
                     const int pos, const WordType* type)
    : Word(text), _position(pos)  {
    _string_realization = s;
    _type = type;
    _sequences = seqs;
}

bool WordToken::operator==(const WordToken& other) const {
    return
           this->_type == other._type
        && this->_position == other._position;
}

void WordToken::remove_from(const Sequence* seq) const {
    auto si = _sequences->begin();
    while (si != _sequences->end())
        if (*si == seq)
            si = _sequences->erase(si);
        else
            ++si;
}

/////////////////////////////// WordType //////////////////////////////////////
WordType::WordType(const Text* text)
    : Word(text), _frequency(0) {}

const WordType& WordType::add_token(const WordToken& token) {
    _tokens.push_back(token);
    ++_frequency;
    return *this;
}

bool WordType::operator==(const WordType& other) const {
    for (const WordToken& this_tok : this->_tokens)
        for (const WordToken& other_tok : other._tokens)
            if (this_tok.get_str() == other_tok.get_str())
                return true;
    return false;
}

////////////////////////////////// Text ///////////////////////////////////////
const WordToken& Text::operator[](int index) const {
    return _words[index];
}

const WordToken& Text::at(int index) const {
    if (index >= _length || index < 0)
        throw out_of_range("Text lookup out of range");
    return this->operator[](index);
}

Text::Text(const string& fname) {
    open(fname);
}

Text::~Text() {
    for (pair<const string_impl, string_impl*>& sp : string_ptrs)
        delete sp.second;

    for (pair<const string_impl, WordType*>& wt : _types)
        delete wt.second;
}

void Text::open(const string& fname) {
    ifstream file;

    file.open(fname);
    if (!file.is_open())
        throw runtime_error(string("Text file not found: ") + fname);

    int pos = 0;
    char c_line[256];
    while (!file.eof()) {
        file.getline(c_line, 256);

        string_impl line = c_line;

        lower_case(&line);

        if (_types.find(line) == _types.end())
            _types[line] = new WordType(this);
        if (string_ptrs.find(line) == string_ptrs.end())
            string_ptrs[line] = new string_impl(line);
        list<Sequence*> *seqs = new list<Sequence*>();
        WordToken tok = WordToken(this,
                                  seqs,
                                  string_ptrs[line],
                                  pos,
                                  _types[line]);
        _types[line]->add_token(tok);
        _words.push_back(tok);

        ++pos;
    }

    _length = pos;
    _fname = fname;

    file.clear();
    file.close();
}

