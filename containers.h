// Copyright 2012 Florian Petran
// contains low-level containers for alignment elements
#ifndef ALIGN_CONTAINERS_HH
#define ALIGN_CONTAINERS_HH

#include<list>
#include<map>

#include<iostream>
#include<string>

#include"text.h"
#include"dictionary.h"
#include"params.h"

namespace Align {

/**
 * An aligned pair of WordTokens in e and f texts.
 */
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

class Hypothesis;

/**
 * An aligned sequence of pairs
 */
class Sequence {
    friend class Hypothesis;
    public:
        Sequence() = delete;
        //!< if this is accidentally used, lots of data members
        //!< will go uninitialized
        void add(const Pair&);
        //!< add a pair to the sequence
        bool add_if_close(const Pair&);
        //!< add a pair if its target is close to our last target,
        //!< return whether it was added or not
        void merge(const Sequence&);
        //!< merge another sequence to this
        const Sequence& reverse();
        //!< reverse e and f

        int slot() const;
        //!< starting slot of the sequence
        int back_slot() const;
        //!< slot of the last pair in the sequence

        int length() const;

        void set_score(const float&);
        const float& get_score() const;


        bool operator==(const Sequence&) const;
        bool has_target(int target_position);
        bool has_target(const Pair&);
        //!< checks if a target index is already in
        //!< the sequence. POTENTIALLY EXPENSIVE

        const Pair& first_pair() const;
        const Pair& last_pair() const;

        typedef std::list<Pair>::const_iterator iterator;

        Sequence::iterator begin() const;
        Sequence::iterator end() const;

    protected:
        explicit Sequence(const Dictionary&);
        Sequence(const Dictionary&, const Pair&);
        //!< construct an initial Sequence over two texts from one pair
        Sequence(const Dictionary&, const Pair&, const Pair&);
        //!< construct a Sequence over two pairs
        Sequence(const Sequence&);
        const Sequence& operator=(const Sequence&);
        ~Sequence();
        //!< only hypothesis may call the dtor, because it
        //!< owns the pointers

    private:
        std::list<Pair> _list;
        const Dictionary* _dict;

        float _score;
        int _slot, _back_slot;

        // mutable isn't threadsafe? research this and possibly fix
        // XXX
        mutable int _length;
};

class SequenceContainer;

/**
 * an alignment hypothesis consisting of multiple sequences.
 * also the Sequence factory.
 *
 * what this class needs to do:
 * - add and remove sequences
 * - iterate over sequences
 * - track/own sequence pointers on the heap stored for the WordToken
 * - reverse e/f direction
 * - merge with another hypothesis
 */
class Hypothesis {
    friend class SequenceContainer;
    public:
        typedef std::list<Sequence*>::iterator iterator;
        typedef std::list<Sequence*>::const_iterator const_iterator;
        inline iterator begin() {
            return _sequences.begin();
        };
        inline const_iterator cbegin() const {
            return _sequences.cbegin();
        };
        inline iterator end() {
            return _sequences.end();
        };
        inline const_iterator cend() const {
            return _sequences.cend();
        }

        Sequence* new_sequence(const Pair& p);
        iterator remove_sequence(iterator pos);
        iterator remove_sequence(Sequence* seq);

        const Hypothesis& reverse();
        const Hypothesis& munch(Hypothesis* other);
        //< munch up another hypothesis, adding all sequence
        //< to this and removing them from the other hypothesis
    protected:
        explicit Hypothesis(const Dictionary& d);
        ~Hypothesis();
        Hypothesis() = delete;
        Hypothesis(const Hypothesis&) = delete;
        const Hypothesis& operator=(const Hypothesis&) = delete;

        inline void set_dict(const Dictionary& d) {
            _dict = &d;
        };

    private:
        const Dictionary* _dict;
        std::list<Sequence*> _sequences;
};
}

std::ostream& operator<<(std::ostream& strm, const Align::Pair& pair);
std::ostream& operator<<(std::ostream& strm, const Align::Sequence& seq);

#endif  // ALIGN_CONTAINERS_HH

