// Copyright 2012 Florian Petran
#ifndef ALIGN_ALIGN_HH
#define ALIGN_ALIGN_HH

#include<cstdlib>

#include<list>
#include<utility>
#include<map>

#include"params.h"
#include"text.h"
#include"dictionary.h"
#include"containers.h"
#include"scorers.h"

class Candidates {
    friend class SequenceContainer;
    public:
        explicit Candidates(const Dictionary&);
        ~Candidates();

        Candidates() = delete;
        Candidates(const Candidates&) = delete;
        const Candidates& operator=(const Candidates&) = delete;

        void collect();
        //< collect all translation candidates

        typedef std::map<WordToken, std::list<WordToken>*>::iterator
            iterator;

        inline Candidates::iterator begin() {
            return _translations.begin();
        };
        inline Candidates::iterator end() {
            return _translations.end();
        };
        inline std::list<WordToken>* at(const WordToken& pos) const {
            return _translations.at(pos);
        }
        inline std::list<WordToken>* operator[](const WordToken& pos) {
            return _translations[pos];
        }

    protected:
        std::map<WordToken, std::list<WordToken>*> _translations;
        const Dictionary* _dict;
};

class SequenceContainer {
    public:
        explicit SequenceContainer(Candidates*);

        SequenceContainer() = delete;
        SequenceContainer(const SequenceContainer&) = delete;
        const SequenceContainer&
            operator=(const SequenceContainer&) = delete;

        SequenceContainer& initial_sequences();
        //< construct initial bigrams of pairs
        SequenceContainer& expand_sequences();
        //< expand the sequences at tail end
        SequenceContainer& merge_sequences();
        SequenceContainer& collect_scores();
        SequenceContainer& get_topranking();

        inline Hypothesis* get_result() {
            return &hypothesis;
        }

    private:
        Hypothesis hypothesis;

        Params* params;
        Candidates* _candidates;
        const Dictionary* _dict;

        ScoringMethods scoring_methods;
        //< contains all scoring methods as functors
};


#endif // ALIGN_ALIGN_HH
