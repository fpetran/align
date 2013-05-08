// Copyright 2012 Florian Petran
// representations for texts, and word types and tokens
#ifndef ALIGN_TEXT_HH
#define ALIGN_TEXT_HH

#include<string>
#include<fstream>
#include<list>
#include<vector>
#include<map>

#include"string_impl.h"
#include"params.h"

class Sequence;
class Text;

/**
 * base class for tokens/types
 *  access text ptr of a word
 **/
class Word {
    public:
        const Text& get_text() const;

    protected:
        explicit Word(const Text*);
        const Text* _text;

    private:
        Word() {}
};

class WordType;

/**
 * a specific word at a specific position in the text
 *
 * access position in text
 * access string representation
 **/
class WordToken : public Word {
    friend class Text;
    friend class Pair;
    public:
        ~WordToken() {};
        inline const string_impl& get_str() const {
            return *_string_realization;
        }
        inline int position() const {
            return _position;
        }
        bool operator==(const WordToken&) const;
        inline bool operator!=(const WordToken& other) const {
            return !(*this == other);
        }
        inline bool operator<(const WordToken& other) const {
            return this->_position < other._position;
        }
        inline const WordType& get_type() const {
            return *_type;
        }

        inline bool close_to(const WordToken& other) const {
            return this->_position != other._position
                && abs(this->_position - other._position)
                   <= Params::get()->closeness();
        }

        inline void add_to_sequence(Sequence* seq) const {
            _sequences->push_back(seq);
        }

        inline std::list<Sequence*>* get_sequences() const {
            return _sequences;
        }

        void remove_from(const Sequence* seq) const;

    protected:
        WordToken(const Text* txt,
                  std::list<Sequence*>* seqs,
                  const string_impl* str,
                  int pos,
                  const WordType* type);
        WordToken();

    private:
        int _position;
        const WordType* _type;
        const string_impl* _string_realization;
        mutable std::list<Sequence*> *_sequences;
};

/**
 * a WordType
 *  access frequency of its tokens
 *  access a list of the token realizations
 **/
class WordType : public Word {
    friend class Text;
    public:
        inline const std::list<WordToken>& get_tokens() const {
            return _tokens;
        }
        inline int frequency() const {
            return _frequency;
        }
        inline bool operator<(const WordType& other) const {
            // this may be a problem. what if a type doesn't have a
            // token yet? XXX
            // the reason for the comparison operator is inserting
            // in maps as keys, which currently only happens in
            // dictionaries, so for now, it should be okay. needs
            // revising though.
            return this->_tokens.front() < other._tokens.front();
        }
        bool operator==(const WordType&) const;
        //< tests if any of the associated tokens' string
        //< representations match
    protected:
        explicit WordType(const Text*);
        const WordType& add_token(const WordToken&);
    private:
        std::list<WordToken> _tokens;
        int _frequency;
};

/**
 * Represents a Text as a container.
 *
 * owns WordTokens string ptrs
 *
 * Words are stored in a vector of words, at the
 * same time there is a map that stores the indexes of each word.
 *
 **/
class Text {
    friend class Dictionary;
    friend class DictionaryFactory;
    public:
        const WordToken& operator[](int) const;// NOLINT[readability/function]
        //< lookup of a word by index
        const WordToken& at(int) const;        // NOLINT[readability/function]
        //< range checked lookup

        inline int length() const {
            return _length;
        }

        inline const std::string& filename() const {
            return _fname;
        }

        typedef std::vector<WordToken>::const_iterator iterator;
        inline Text::iterator begin() const {
            return _words.begin();
        }
        inline Text::iterator end() const {
            return _words.end();
        }

    protected:
        explicit Text(const std::string&);
        ~Text();

        std::map<string_impl, WordType*> _types;

    private:
        Text();
        Text(const Text&);
        const Text& operator=(const Text&);

        void open(const std::string&);

        std::string _fname;
        std::vector<WordToken> _words;
        std::map<string_impl, string_impl*> string_ptrs;
        //< these are copied between tokens, but owned by Text
        int _length;
};

#endif // ALIGN_TEXT_HH
