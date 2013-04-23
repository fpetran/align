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

        void collect();
        //< collect all translation candidates

        typedef std::map<WordToken, std::list<WordToken>>::iterator
            iterator;

        inline Candidates::iterator begin() {
            return _translations.begin();
        };
        inline Candidates::iterator end() {
            return _translations.end();
        };
        inline std::list<WordToken>& at(const WordToken& pos) {
            return _translations.at(pos);
        }
        inline std::list<WordToken>& operator[](const WordToken& pos) {
            return _translations[pos];
        }

    protected:
        std::map<WordToken, std::list<WordToken>> _translations;
        const Dictionary* _dict;
};

// turns out I just misunderstood how strongly typed enums are
// supposed to be used, so let's all forget I ever blamed gcc
enum class BreakAfterPhase {
    Initial,
    Expand,
    Merge,
    Never
};

class SequenceContainer {
    public:
        explicit SequenceContainer(Candidates*);

        SequenceContainer() = delete;
        SequenceContainer(const SequenceContainer&) = delete;
        const SequenceContainer&
            operator=(const SequenceContainer&) = delete;

        void make(const BreakAfterPhase = BreakAfterPhase::Never);
        // TODO(fpetran):
        // find another way to ensure the correct sequence
        // of steps. the latter part of the sequence can be
        // varied, and the reverse sequences step occurs before
        // the merge step and should take place outside of the
        // SC
        SequenceContainer& initial_sequences();
        //< construct initial bigrams of pairs
        SequenceContainer& expand_sequences();
        //< expand the sequences at tail end
        SequenceContainer& merge_sequences();
        SequenceContainer& collect_scores();
        SequenceContainer& get_topranking();

        typedef std::list<Sequence>::const_iterator iterator;

        const SequenceContainer& reverse();
        const SequenceContainer& merge(const SequenceContainer&);

        SequenceContainer::iterator begin() const;
        SequenceContainer::iterator end() const;

    private:
        std::list<Sequence> _list;

        Params* params;
        Candidates* _candidates;
        const Dictionary* _dict;

        ScoringMethods scoring_methods;
        //< contains all scoring methods as functors
};


#endif // ALIGN_ALIGN_HH
