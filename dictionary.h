// Copyright 2012 Florian Petran
#ifndef ALIGN_DICTIONARY_HH
#define ALIGN_DICTIONARY_HH

#include<string>
#include<map>
#include<utility>
#include<list>
#include<algorithm>

#include"text.h"

namespace Align {

class Dictionary;

/**
 * holds the index to the dictionary files
 * constructs dictionaries for a text pair on demand or returns it
 * constructs texts
 * owns all Text ptrs which in turn own their Word ptrs
 *
 **/
class DictionaryFactory {
    public:
        static DictionaryFactory* get_instance();
        const Dictionary* get_dictionary(const std::string&,
                                         const std::string&);
        //!< read a dictionary from the file if its first requested,
        //!< otherwise just return the pointer to the dictionary entry

        Text* get_text(const std::string&);
        //!< create a new text with the given file name on first request,
        //!< or just return the text pointer otherwise

        ~DictionaryFactory();
    private:
        std::string locate_dictionary_file(const std::string&,
                                           const std::string&);
        std::string index_filename;
        std::map<std::pair<std::string, std::string>, Dictionary*>
            dictionaries;
        std::map<std::string, Text*> texts;
        static DictionaryFactory* _instance;
        DictionaryFactory();
};


/**
 * A directional bi-lingual/-textual dictionary
 *
 * lookup word translations
 * build the Text class
 *
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
 **/
class Dictionary {
    friend class DictionaryFactory;
    public:
        const std::list<WordType>& lookup(const WordToken&) const;
        //!< look up a WordToken, and get a list to its translations
        //!< translations may be multiple types which might in turn
        //!< have multiple locations in the target file

        bool has(const WordToken&) const;
        //!< check if the dictionary has an entry for word

        inline const Text* get_e() const { return _e; }
        //!< return the source text for this dictionary
        inline const Text* get_f() const { return _f; }
        //!< return the target text for this dictionary

    protected:
        explicit Dictionary(const std::string&);
        Dictionary();
        ~Dictionary() {}

        inline void set_texts(Text* e, Text* f) {
            _e = e;
            _f = f;
        };

        void open(const std::string&);
        void read(std::ifstream*);

    private:
        Dictionary(const Dictionary&);
        const Dictionary& operator=(const Dictionary&);

        DictionaryFactory* factory;
        Text *_e, *_f;
        std::map<WordType, std::list<WordType>> _storage;
        const std::list<WordType> empty_entry;
        //!< an empty dictionary entry
};
}

#endif  // ALIGN_DICTIONARY_HH

