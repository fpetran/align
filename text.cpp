// Copyright 2012 Florian Petran
#include"text.h"

#include<cctype>

#include<string>
#include<fstream>
#include<stdexcept>
#include<utility>

#include"string_impl.h"

using std::string;
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

WordToken::WordToken(const Text* text, const string_impl* s,
                     const int pos, const WordType* type)
    : Word(text), _position(pos)  {
    _string_realization = s;
    _type = type;
}

bool WordToken::operator==(const WordToken& other) const {
    return
           this->_type == other._type
        && this->_position == other._position;
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
    _fname = fname;
    ifstream file;
    _length = 0;

    file.open(_fname);
    if (!file.is_open())
        throw runtime_error(string("Text file not found: ") + fname);

    char c_line[256];
    while (!file.eof()) {
        file.getline(c_line, 256);

        string_impl line = c_line;

        /*
        // a single non alnum taints the string
        // this should be extended to consider
        // numbers as well XXX
        bool is_alnum = true;
        for (string_size i = 0; i < line.length(); ++i)
            is_alnum = is_alnum && check_if_alpha(line[i]);
        if (!is_alnum)
            continue;
        */

        lower_case(&line);

        if (_types.find(line) == _types.end())
            _types[line] = new WordType(this);
        if (string_ptrs.find(line) == string_ptrs.end())
            string_ptrs[line] = new string_impl(line);
        WordToken tok = WordToken(this,
                                  string_ptrs[line],
                                  _length,
                                  _types[line]);
        _types[line]->add_token(tok);
        _words.push_back(tok);

        ++_length;
    }

    file.clear();
    file.close();
}

