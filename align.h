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
    friend class AlignMake;
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

class AlignMake {
    public:
        explicit AlignMake(Candidates* cand);
        ~AlignMake();

        AlignMake() = delete;
        AlignMake(const AlignMake&) = delete;
        const AlignMake&
            operator=(const AlignMake&) = delete;

        /// construct initial Sequence objects (bigrams of pairs)
        AlignMake& initial_sequences();
        /// expand the Sequence at tail end
        AlignMake& expand_sequences();
        /// merge Sequence that are close
        AlignMake& merge_sequences();
        /// collect confidence scores for all Sequence
        AlignMake& collect_scores();
        /// remove all but topranking Sequence
        AlignMake& get_topranking();

        inline Hypothesis* get_result() {
            return hypothesis;
        }
        /// return the address of Scorer container, so that users
        /// can push_back() custom Scorer methods.
        inline ScoringMethods* scorers() {
            return &scoring_methods;
        }

    private:
        Hypothesis* hypothesis;

        Candidates* _candidates;
        const Dictionary* _dict;
        /// contains all scoring methods as functors
        ScoringMethods scoring_methods;
};
}  // namespace Align

#endif  // ALIGN_H_

