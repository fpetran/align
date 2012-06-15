// Copyright 2012 Florian Petran
// contains low-level containers for alignment elements
#ifndef ALIGN_CONTAINERS_HH
#define ALIGN_CONTAINERS_HH

#include<list>
#include<map>

#include"text.h"
#include"params.h"

class PairFactory;

class Pair {
    friend class PairFactory;
    public:
        bool is_close_to(const Pair&) const;
        const Word& source() const;
        const Word& target() const;
        int slot() const;
        int target_slot() const;
    protected:
        Pair(const Word&, int, const Word&, int);
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
        int slot() const;
        //< starting slot of the sequence
        int back_slot() const;
        const Pair& first_pair();
        const Pair& last_pair();
        std::list<Pair>::const_iterator begin() const;
        std::list<Pair>::const_iterator end() const;

    private:
        std::list<Pair> _list;
        const Dictionary* _dict;
        unsigned int _slot, _back_slot;
};



#endif // ALIGN_CONTAINERS_HH
