// Copyright 2012 Florian Petran
#ifndef ALIGN_H_
#define ALIGN_H_
#include<cstdlib>
#include<list>
#include<utility>
#include<map>
#include"params.h"
#include"text.h"
#include"dictionary.h"
#include"containers.h"
#include"scorers.h"

namespace Align {

class Candidates {
    friend class SequenceContainer;
    public:
        explicit Candidates(const Dictionary&);
        ~Candidates();

        Candidates() = delete;
        Candidates(const Candidates&) = delete;
        const Candidates& operator=(const Candidates&) = delete;

        /// collect all translation candidates
        void collect();

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
        explicit SequenceContainer(Candidates* cand);

        SequenceContainer() = delete;
        SequenceContainer(const SequenceContainer&) = delete;
        const SequenceContainer&
            operator=(const SequenceContainer&) = delete;

        /// construct initial Sequence objects (bigrams of pairs)
        SequenceContainer& initial_sequences();
        /// expand the Sequence at tail end
        SequenceContainer& expand_sequences();
        /// merge Sequence that are close
        SequenceContainer& merge_sequences();
        /// collect confidence scores for all Sequence
        SequenceContainer& collect_scores();
        /// remove all but topranking Sequence
        SequenceContainer& get_topranking();

        inline Hypothesis* get_result() {
            return hypothesis;
        }

    private:
        Hypothesis* hypothesis;

        Params* params;
        Candidates* _candidates;
        const Dictionary* _dict;

        /// contains all scoring methods as functors
        ScoringMethods scoring_methods;
};
}

#endif  // ALIGN_H_

