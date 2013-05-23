// Copyright 2012 Florian Petran
#ifndef DICTIONARY_H_
#define DICTIONARY_H_
#include<string>
#include<map>
#include<utility>
#include<list>
#include<algorithm>
#include"text.h"

namespace Align {

class Dictionary;

/// Construct Dictionary objects.
/** Reads the dictionary index file and the Dictionary objects
 *  from their files.
 *  Holds the index to Dictionary objects already created, and
 *  will create a new one if it is requested. Constructs the
 *  Text objects as well. Owns all Text pointers.
 **/
class DictionaryFactory {
    public:
        static DictionaryFactory* get_instance();
        /// read a dictionary from the file if its first requested,
        /// otherwise just return the pointer to the dictionary entry
        const Dictionary* get_dictionary(const std::string&,
                                         const std::string&);
        /// create a new text with the given file name on first request,
        /// or just return the text pointer otherwise
        Text* get_text(const std::string&);
        ~DictionaryFactory();
    private:
        std::string locate_dictionary_file(const std::string&,
                                           const std::string&);
        std::string index_filename;
        typedef std::pair<std::string, std::string> textpair;
        /// an index mapping the text pair to their Dictionary objects
        std::map<textpair, Dictionary*> dictionaries;
        /// an index mapping the filenames to the Text objects
        std::map<std::string, Text*> texts;
        static DictionaryFactory* _instance;
        DictionaryFactory();
};


/// A directional bi-lingual/-textual dictionary
/** Represents a Dictionary, that is specific to two texts. Note
 *  that the Dictionary is directional - Dictionary( e, f ) is not
 *  the same as Dictionary( f, e )
 *
 *  lookup word translations
 *  build the Text class
 *
 *  The filename for the dictionary is first determined using an
 *  index file, then that file is opened and a word list constructed
 *  from it for each f and e texts. Also note that the Text objects
 *  need to be pointers since they are mutually dependent on Dictionary.
 *
 *  The actual translation dictionary is stored as a map of vectors. Keys
 *  of that map are the e Words, and the vector value stores all f Words.
 **/
class Dictionary : private std::map<WordType, std::list<WordType> > {
    friend class DictionaryFactory;
    public:
        const Dictionary& operator=(const Dictionary&) = delete;
        Dictionary(const Dictionary&) = delete;

        /// look up a WordToken, and get a list to its translations
        /// translations may be multiple types which might in turn
        /// have multiple locations in the target file
        const std::list<WordType>& lookup(const WordToken&) const;
        /// check if the dictionary has an entry for word
        bool has(const WordToken&) const;
        /// return the source text for this dictionary
        inline const Text* get_e() const { return _e; }
        /// return the target text for this dictionary
        inline const Text* get_f() const { return _f; }

    protected:
        explicit Dictionary(const std::string&);
        Dictionary();

        inline void set_texts(Text* e, Text* f) {
            _e = e;
            _f = f;
        };

        void open(const std::string&);
        void read(std::ifstream*);

    private:
        DictionaryFactory* factory;
        Text *_e, *_f;
        /// an empty dictionary entry
        const std::list<WordType> empty_entry;
};
}
#endif  // DICTIONARY_H_

