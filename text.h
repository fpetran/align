// Copyright 2012 Florian Petran
// representations for texts, and word types and tokens
#ifndef TEXT_H_
#define TEXT_H_
#include<string>
#include<fstream>
#include<list>
#include<vector>
#include<map>
#include"string_impl.h"
#include"params.h"

namespace Align {

class Sequence;
class Text;

/// Base class for WordToken and WordType.
/** Takes care of the storage and access
 *  of the Text ptr via the derived class.
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

/// A specific word at a specific position in the Text.
/** Provides a check for closeness with another WordToken,
 *  and a the methods to remove them from a Sequence, or
 *  add them to a Sequence.
 *
 *  Also provides access to
 *  - the WordType, if needed.
 *  - the string realization of the WordToken.
 *  - All Sequence objects this WordToken is part of.
 *
 *  Since Sequence list, and string realization are both
 *  pointers, only Text class is allowed to construct
 *  WordToken objects, to ensure consistency and prevent
 *  leaks.
 **/
class WordToken : public Word {
    friend class Text;
    public:
        bool operator==(const WordToken&) const;
        bool close_to(const WordToken& other) const;
        void remove_from(const Sequence* seq) const;

        inline const string_impl& get_str() const {
            return *_string_realization;
        }
        inline int position() const {
            return _position;
        }
        inline bool operator!=(const WordToken& other) const {
            return !(*this == other);
        }
        inline bool operator<(const WordToken& other) const {
            return this->_position < other._position;
        }
        inline const WordType& get_type() const {
            return *_type;
        }

        inline void add_to_sequence(Sequence* seq) const {
            _sequences->push_back(seq);
        }

        inline std::list<Sequence*>* get_sequences() const {
            return _sequences;
        }

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

/// A set of WordToken in a particular text.
/** Note that the WordToken objects do not necessarily have the
 *  same string realization, therefore we can't access that here
 *  directly.
 *
 *  Provide access to
 *  - frequency of its tokens (by amount of tokens associated with it).
 *  - all WordToken objects associated with this.
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

        /// test if ANY of the associated WordToken string
        /// representations match
        bool operator==(const WordType&) const;

    protected:
        explicit WordType(const Text*);
        const WordType& add_token(const WordToken&);

    private:
        std::list<WordToken> _tokens;
        int _frequency;
};

/// Represents a Text as container.
/** Provide sequential and random access to WordToken and WordType
 *  objects.
 *  Also owns the pointers to the string realizations of WordToken.
 *  For this reason, the dtor can only be called by the Dictionary
 *  objects associated with this Text.
 **/
class Text : private std::vector<WordToken> {
    friend class Dictionary;
    friend class DictionaryFactory;
    public:
        Text() = delete;
        Text(const Text&) = delete;
        const Text& operator=(const Text&) = delete;

        const WordToken& operator[](int pos) const;
        const WordToken& at(int pos) const;
        inline int length() const {
            return _length;
        }
        inline const std::string& filename() const {
            return _fname;
        }

        /// access is provided via a const_iterator, since
        /// we don't expect to change the text at any point.
        typedef std::vector<WordToken>::const_iterator iterator;
        inline Text::iterator begin() const {
            return this->cbegin();
        }
        inline Text::iterator end() const {
            return this->cend();
        }

    protected:
        explicit Text(const std::string&);
        ~Text();
        std::map<string_impl, WordType*> _types;

    private:
        void open(const std::string&);

        std::string _fname;
        /// these are copied between tokens, but owned by Text
        std::map<string_impl, string_impl*> string_ptrs;
        int _length;
};
}

#endif  // TEXT_H_

