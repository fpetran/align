// Copyright 2012 Florian Petran
// contains low-level containers for alignment elements
#ifndef CONTAINERS_H_
#define CONTAINERS_H_
#include<list>
#include<map>
#include<iostream>
#include<string>
#include"text.h"
#include"dictionary.h"
#include"params.h"

namespace Align {

/// An aligned pair of WordTokens in e and f texts.
/** wraps a number of WordToken related functions
 *  such as the closeness checks, and the slots of the
 *  WordTokens. Also provides a convenience function
 *  to reverse the alignment direction.
 **/
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

/// An aligned sequence of pairs
class Sequence : std::list<Pair> {
    friend class Hypothesis;
    public:
        Sequence() = delete;
        /// add a pair to the sequence
        void add(const Pair&);
        /// add a pair if its target is close to our last target,
        /// return whether it was added or not
        bool add_if_close(const Pair&);
        /// merge another sequence to this
        void merge(const Sequence&);
        /// reverse e and f
        const Sequence& reverse();
        /// starting slot of the sequence
        /// i.e. the position of its first target
        int slot() const;
        /// slot of the last pair in the sequence
        /// i.e. the position of its last target
        int back_slot() const;
        int length() const;
        /// set the confidence score of this Sequence
        void set_score(const float&);
        const float& get_score() const;

        bool operator==(const Sequence&) const;
        bool has_target(int target_position);
        /// checks if a target index is already in
        /// the sequence. POTENTIALLY EXPENSIVE
        bool has_target(const Pair&);

        const Pair& first_pair() const;
        const Pair& last_pair() const;

        typedef std::list<Pair>::const_iterator const_iterator;
        inline Sequence::const_iterator cbegin() const {
            return this->cbegin();
        }
        inline Sequence::const_iterator cend() const {
            return this->cend();
        }
        typedef std::list<Pair>::iterator iterator;
        inline Sequence::iterator begin() {
            return this->begin();
        }
        inline Sequence::iterator end() {
            return this->end();
        }

    protected:
        explicit Sequence(const Dictionary&);
        /// construct an initial Sequence over two texts from one pair
        Sequence(const Dictionary&, const Pair&);
        /// construct a Sequence over two pairs
        Sequence(const Dictionary&, const Pair&, const Pair&);
        Sequence(const Sequence&);
        const Sequence& operator=(const Sequence&);
        /// only hypothesis may call the dtor, because it
        /// owns the pointers
        ~Sequence();

    private:
        const Dictionary* _dict;
        float _score;
        int _slot, _back_slot;
        /// cache for _length
        mutable int _length;
};

class SequenceContainer;

/// an alignment hypothesis consisting of multiple sequences.
/** also the Sequence factory.
 *
 *  what this class needs to do:
 *  - add and remove sequences
 *  - iterate over sequences
 *  - track/own sequence pointers on the heap stored for the WordToken
 *  - reverse e/f direction
 *  - merge with another hypothesis
 */
class Hypothesis : private std::list<Sequence*> {
    friend class SequenceContainer;
    public:
        typedef std::list<Sequence*>::iterator iterator;
        typedef std::list<Sequence*>::const_iterator const_iterator;
        inline iterator begin() {
            return this->begin();
        };
        inline const_iterator cbegin() const {
            return this->cbegin();
        };
        inline iterator end() {
            return this->end();
        };
        inline const_iterator cend() const {
            return this->cend();
        }

        Sequence* new_sequence(const Pair& p);
        iterator remove_sequence(iterator pos);
        iterator remove_sequence(Sequence* seq);

        /// reverse the alignment direction of this
        /// Hypothesis
        const Hypothesis& reverse();
        /// munch up another hypothesis, adding all sequence
        /// to this and removing them from the other hypothesis
        const Hypothesis& munch(Hypothesis* other);

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
};
}

std::ostream& operator<<(std::ostream& strm, const Align::Pair& pair);
std::ostream& operator<<(std::ostream& strm, const Align::Sequence& seq);

#endif  // CONTAINERS_H_

