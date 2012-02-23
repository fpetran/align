// Copyright 2012 Florian Petran
#ifndef ALIGN_ALIGN_HH
#define ALIGN_ALIGN_HH

#include<cstdlib>

#include<list>
#include<map>
#include<vector>

#include"text.h"


class PairFactory;

class Pair {
    friend class PairFactory;
    friend class Sequence;
    public:
        bool is_close_to(const Pair&);
    protected:
        Pair(const Word&, int, const Word&, int);
        int slot();
    private:
        Word _source, _target;
        int _fi, _ei;
};

/**
 * Produce alignment pairs over two texts.
 *
 * ok, so it's not strictly a factory in the sense of
 * GoF - sue me. it exists to simplify the ctor calls for
 * Pair with params that are common to all Pairs.
 *
 **/
class PairFactory {
    public:
        explicit PairFactory(const Dictionary&);
        Pair make_pair(int, int);
    private:
        const Dictionary* _dict;
};

class Sequence {
    public:
        Sequence(const Dictionary&, const Pair&);
        //< construct an initial Sequence over two texts from one pair
        Sequence(const Dictionary&, const Pair&, const Pair&);
        //< construct a Sequence over two pairs

        void add(const Pair&);
        //< add a pair to the sequence
        void merge(const Sequence&);
        //< merge another sequence to this
        int slot();


    private:
        std::list<Pair> _list;
        const Dictionary* _dict;
};

class Candidates {
    public:
        explicit Candidates(const Dictionary&);
        ~Candidates();

        void remove(const Sequence&);
        //< remove a sequence from the candidates collection

    private:
        PairFactory* pair_factory;
        std::list<Sequence> _list;

        std::map<int, std::vector<int> > _translations;

        const Dictionary* _dict;

        void collect_candidates();
        //< collect all translation candidates
        void initial_sequences();
        //< construct initial bigrams of pairs
        void expand_sequences();
        //< expand the sequences backwards
};

#endif // ALIGN_ALIGN_HH
