// Copyright 2012 Florian Petran
#ifndef ALIGN_SCORERS_HH
#define ALIGN_SCORERS_HH

#include<vector>
#include"containers.h"

namespace Align {

class Scorer;

/**
 * encapsulates a container of scoring methods
 * this way, the clients don't need to know
 * what scoring methods there are
 *
 **/
class ScoringMethods : public std::vector<Scorer*> {
    public:
        ScoringMethods();
        ~ScoringMethods();
    private:
        ScoringMethods(const ScoringMethods&);
        const ScoringMethods& operator=(const ScoringMethods&);
        void push_back(Scorer*); // NOLINT[readability/function]
        //!< private to prevent clients from adding methods
};

/**
 * pure virtual base for all scoring methods
 *
 * a scoring method is a functor that calculates the
 * raw (unnormalized) score for a sequence. The base provides
 * storage and a getter for the maximum of the respective
 * score.
 *
 * a virtual dtor is not currently needed, since the derived
 * scorers don't define their own data members, no matter what
 * cpplint.py thinks.
 *
 **/
class Scorer { // NOLINT[runtime/virtual]
    public:
        Scorer();
        virtual float operator()(const Sequence&) = 0;
        virtual float get_max();
        //!< returns the maximum value for this score over all sequences
        //!< seen so far
        virtual const char* name() = 0;
    protected:
        float _max;
};

/**
 * simple scorer that favors long sequences
 **/
class LengthScorer : public Scorer {
    float operator()(const Sequence&);
    const char* name() {
        return "length";
    }
};

/**
 * score average index difference between e and f tokens
 **/
class IndexdiffScorer : public Scorer {
    float operator()(const Sequence&);
    const char* name() {
        return "i-diff";
    }
};

/**
 * score average bi_sim between e and f strings
 **/
class BisimScorer : public Scorer {
    float operator()(const Sequence&);
    const char* name() {
        return "bi_sim";
    }
};
}

#endif  // ALIGN_SCORERS_HH

