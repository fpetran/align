// Copyright 2012 Florian Petran
// contains low-level containers for alignment elements
#ifndef ALIGN_CONTAINERS_HH
#define ALIGN_CONTAINERS_HH

#include<list>
#include<map>

#include"text.h"
#include"dictionary.h"
#include"params.h"

class Pair {
    public:
        bool targets_close(const Pair&) const;
        bool both_close(const Pair&) const;

        bool operator==(const Pair&) const;
        inline bool operator!=(const Pair& that) const {
            return !(*this == that);
        };

        const Pair& reverse();

        const WordToken& source() const;
        const WordToken& target() const;
        int slot() const;
        int target_slot() const;

        Pair(const WordToken&, const WordToken&);
    private:
        WordToken _source, _target;
};

class Sequence {
    public:
        explicit Sequence(const Dictionary&);
        Sequence(const Dictionary&, const Pair&);
        //< construct an initial Sequence over two texts from one pair
        Sequence(const Dictionary&, const Pair&, const Pair&);
        //< construct a Sequence over two pairs

        void add(const Pair&);
        //< add a pair to the sequence
        bool add_if_close(const Pair&);
        //< add a pair if its target is close to our last target,
        //< return whether it was added or not
        void merge(const Sequence&);
        //< merge another sequence to this
        const Sequence& reverse();
        //< reverse e and f

        int slot() const;
        //< starting slot of the sequence
        int back_slot() const;
        //< slot of the last pair in the sequence

        int length() const;

        void set_score(const float&);
        const float& get_score() const;


        bool operator==(const Sequence&) const;
        bool has_target(int target_position);
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
        int _slot, _back_slot;

        // mutable isn't threadsafe? research this and possibly fix
        // XXX
        mutable int _length;
};



#endif // ALIGN_CONTAINERS_HH
