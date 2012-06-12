// Copyright 2012 Florian Petran
#ifndef ALIGN_TEXT_HH
#define ALIGN_TEXT_HH

#include<string>
#include<fstream>
#include<list>
#include<map>
#include<algorithm>
#include<memory>
#include<stdexcept>

#include"align_config.h"
#include"string_impl.h"
#include"params.h"

class Text;

/**
 * Encapsulates a Word in a light weight class, so that
 * client code doesn't need to take care of memory management.
 *
 * The underlying string is saved as a pointer to a string. Word
 * equality is determined by two Word objects pointing to the same
 * string.
 *
 **/
class Word {
    friend class Dictionary;
    public:
        Word(Text*, std::shared_ptr<string_impl>);
        //< This should be the standard ctor.

        Word() {}
        //< Alternatively, empty ctor, but it needs to be
        //< ensured that open() is called afterwards.

        // big three:
        Word(const Word&);
        ~Word();
        Word& operator=(const Word&);

        // gt/lt operators are needed to insert a word in a map
        // it's not important what criteria they use for ordering
        // though, as Word just encapsulates a smart_ptr
        // accordingly, we're just comparing ptr values here.
        inline bool operator<(const Word& that) const {
            return this->_string < that._string;
        }
        bool operator==(const Word&) const;

        const string_impl& get_str() const;
        //< Return the String associated with the Word.

        int frequency();
        //< Return the frequency of a Word in its Text.

        const std::string& filename() const;

        const Text* get_text() const { return _text; }

    protected:
        const Text* _text;

    private:
        std::shared_ptr<string_impl> _string;
};



typedef std::map<string_impl, Word> Wordlist;
typedef std::list<int> TranslationsEntry;
typedef std::map<int, TranslationsEntry*> Translations;


/**
 * Represents a Dictionary, that is specific to two texts. Note
 * that the Dictionary is directional - Dictionary( e, f ) is not
 * the same as Dictionary( f, e )
 *
 * The filename for the dictionary is first determined using an
 * index file, then that file is opened and a word list constructed
 * from it for each f and e texts. Also note that the Text objects
 * need to be pointers since they are mutually dependent on Dictionary.
 *
 * The actual translation dictionary is stored as a map of vectors. Keys
 * of that map are the e Words, and the vector value stores all f Words.
 *
 * Supposedly, the lookup method can be used to look up a word.
 *
 **/
class Dictionary {
    public:
        Dictionary(const char*, const char*);
        Dictionary() {}
        ~Dictionary();

        void open(const char*, const char*);
        //< open a dictionary file over the two file names provided

        TranslationsEntry* lookup(const Word&) const;
        //< return all indexes in f where there are translations for w
        //< currently, this throws if the word isn't stored

        bool has(const Word&) const;
        //< check if the dictionary has an entry for word

        inline Text* get_e() const { return _e; }
        inline Text* get_f() const { return _f; }

    private:
        void get_dict_fname(const std::string&, const std::string&);
        //< read the dict index file and get the dictionary file from that

        Text *_e, *_f;
        std::string _dict_fname;
        std::map<Word, std::list<Word> > _storage;

        const Dictionary& operator=(const Dictionary&) { return *this; }
        Dictionary(const Dictionary&) {}
};

/**
 * Represents a Text as a container.
 * Words are stored in a vector of words, at the
 * same time there is a map that stores the indexes of each word.
 *
 **/
class Text {
    friend class Dictionary;
    friend class Word;
    public:
        explicit Text(const char*);

        const Word& operator[](int) const;  // NOLINT[readability/function]
        //< lookup of a word by index
        const Word& at(int) const;          // NOLINT[readability/function]
        //< range checked lookup

        inline int length() const { return _length; }

        inline const std::list<int>& index(const Word& w) const {
            return _indexes.at(w);
        }
        inline const int frequency(const Word& w) const {
            return _indexes.at(w).size();
        }

    protected:
        std::string _fname;
        Wordlist _wordlist;

    private:
        void add_to_wordlist(const string_impl&);

        Text();
        //< every time you construct an empty text, god kills a kitten
        Text(const Text&) {}
        const Text& operator=(const Text&) { return *this; }

        std::map<Word, std::list<int> > _indexes;
        std::vector<Word> _words;
        int _length;
};

#endif // ALIGN_TEXT_HH
