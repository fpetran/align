// Copyright 2012 Florian Petran
#include"text.h"
#include<string>
#include<vector>

Word::Word(Text* text, std::shared_ptr<string_impl> str) {
    _text = text;
    _string = str;
}

Word::Word(const Word& that) {
    _text = that._text;
    _string = that._string;
}

Word& Word::operator=(const Word& that) {
    _text = that._text;
    _string = that._string;
    return *this;
}

const std::string& Word::filename() const {
    return _text->_fname;
}

Word::~Word() {
    _string.reset();
}

bool Word::operator==(const Word& other) const {
    return this->_string == other._string;
}

int Word::frequency() {
    return _text->frequency(*this);
}

const Word& Text::operator[](int index) const {
    return _words[ index ];
}

const Word& Text::at(int index) const {
    if (index >= _length)
        throw std::out_of_range("Text lookup out of range");
    return _words[ index ];
}

const string_impl& Word::get_str() const {
    return *_string;
}

Text::Text(const char* fname) {
    _fname = fname;
    std::ifstream file;
    _length = 0;

    file.open(_fname);
    if (!file.is_open())
        throw std::runtime_error(std::string("Text file not found: ") + fname);

    char c_line[256];

    while (!file.eof()) {
        file.getline(c_line, 256);

        bool is_alnum = false;

        for (int i = 0; i < 256; ++i) {
            if (c_line[i] == '\0')
                break;
            is_alnum = is_alnum || isalpha(c_line[i]);
        }

        string_impl line = c_line;
        lower_case(&line);

        if (is_alnum) {
            add_to_wordlist(line);
            _words.push_back(_wordlist.at(line));
        } else {
            _words.push_back(
                    Word(this,
                        std::shared_ptr<string_impl>(new string_impl(line))));
        }


        if (_indexes.find(_words.back()) == _indexes.end())
            _indexes[_words.back()] = std::vector<int>();
        _indexes[_words.back()].push_back(_length);

        ++_length;
    }

    file.clear();
    file.close();
}

void Text::add_to_wordlist(const string_impl& str) {
    if (_wordlist.find(str) == _wordlist.end())
        _wordlist[str] =
            Word(this, std::shared_ptr<string_impl>(new string_impl(str)));
}

