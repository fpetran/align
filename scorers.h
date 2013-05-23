// Copyright 2012 Florian Petran
#ifndef SCORERS_H_
#define SCORERS_H_
#include<vector>
#include"containers.h"

namespace Align {
class Scorer;

/// A container of scoring methods.
/** This class exists so that clients do not need
 *  to know what scoring methods there are, but can
 *  just iterate over this container.
 *
 *  In order to add a client defined scoring method,
 *  you need to define a class that's derived from
 *  Scorer, construct it on the heap, and push it onto
 *  ScoringMethods.
 **/
class ScoringMethods : public std::vector<Scorer*> {
    public:
        ScoringMethods();
        ~ScoringMethods();
        ScoringMethods(const ScoringMethods&) = delete;
        const ScoringMethods& operator=(const ScoringMethods&) = delete;
};

/// pure virtual base for all scoring methods
/** A scoring method is a functor that calculates the
 *  raw (unnormalized) score for a sequence. The base provides
 *  storage and a getter for the maximum of the respective
 *  score.
 *  Custom scorers need to implement the
 *  Scorer::name() and Scorer::operator(), obviously.
 *  where name() serves debugging purposes and may return
 *  and empty char ptr. operator() contains the scoring
 *  logic. Additionally, it needs to update the _max
 *  member after each run, which is used to calculate
 *  averages later.
 **/
class Scorer {
    public:
        Scorer();
        virtual ~Scorer() = default;
        virtual float operator()(const Sequence&) = 0;
        /// returns the maximum value for this score over all sequences
        /// seen so far
        inline virtual float get_max() final {
            return _max;
        }
        virtual const char* name() = 0;
    protected:
        float _max;
};

/// simple scorer that favors long sequences
class LengthScorer : public Scorer {
    float operator()(const Sequence&);
    const char* name() {
        return "length";
    }
};


/// score average index difference between e and f tokens
class IndexdiffScorer : public Scorer {
    float operator()(const Sequence&);
    const char* name() {
        return "i-diff";
    }
};

/// score average bi_sim between e and f strings
class BisimScorer : public Scorer {
    float operator()(const Sequence&);
    const char* name() {
        return "bi_sim";
    }
};
}

#endif  // SCORERS_H_

