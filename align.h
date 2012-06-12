// Copyright 2012 Florian Petran
#ifndef ALIGN_ALIGN_HH
#define ALIGN_ALIGN_HH

#include<cstdlib>

#include<list>
#include<map>

#include"text.h"
#include"params.h"
#include"containers.h"

class Candidates {
    friend class SequenceContainer;
    public:
        explicit Candidates(const Dictionary&);
        ~Candidates();

        void collect();
        //< collect all translation candidates

        Translations::const_iterator begin() const;
        Translations::const_iterator end() const;

    protected:
        PairFactory* pair_factory;

        Translations _translations;
        const Dictionary* _dict;
};

class SequenceContainer {
    public:
        explicit SequenceContainer(const Candidates&);

        void make();

    private:
        PairFactory* pair_factory;
        std::list<Sequence> _list;
        Translations _translations;

        const Dictionary* _dict;

        void initial_sequences();
        //< construct initial bigrams of pairs
        void expand_sequences();
        //< expand the sequences at tail end
        void merge_sequences();
};

#endif // ALIGN_ALIGN_HH
