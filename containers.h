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
        bool targets_close(const Pair&) const;
        bool both_close(const Pair&) const;
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
        //< slot of the last pair in the sequence

        int length() const;

        void set_score(const float&);
        const float& get_score() const;


        bool has_target(int);
        bool has_target(const Pair&);
        //< checks if a target index is already in
        //< the sequence. POTENTIALLY EXPENSIVE

        const Pair& first_pair() const;
        const Pair& last_pair() const;

        typedef std::list<Pair>::const_iterator iterator;

        Sequence::iterator begin() const;
        Sequence::iterator end() const;

    private:
        Sequence();
        //< if this is accidentally used, lots of data members
        //< will go uninitialized

        std::list<Pair> _list;
        const Dictionary* _dict;

        float _score;
        unsigned int _slot, _back_slot;

        mutable unsigned int _length;
};



#endif // ALIGN_CONTAINERS_HH
