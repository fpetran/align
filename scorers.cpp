// Copyright 2012 Florian Petran
#include"scorers.h"

#include<vector>

#include"bi-sim.h"

using std::vector;

ScoringMethods::ScoringMethods() {
    // register all scoring methods here
    this->push_back(new LengthScorer);
    this->push_back(new IndexdiffScorer);
    this->push_back(new BisimScorer);
}

ScoringMethods::~ScoringMethods() {
    // so far we don't need a virtual dtor for the scorers
    // because they don't have data members different from
    // the base. it's possible though that msvc++ requires
    // it - we'll see about that.
    for (vector<Scorer*>::iterator sc = this->begin();
            sc != this->end(); ++sc)
        delete *sc;
}

void ScoringMethods::push_back(Scorer* s) {
    //dynamic_cast<vector<Scorer*>*>(this)->push_back(s);
    vector<Scorer*>::push_back(s);
}

Scorer::Scorer() {
    _max = 0.0;
}

float Scorer::get_max() {
    return _max;
}


float LengthScorer::operator()(const Sequence& seq) {
    _max = (_max > seq.length()) ? _max : seq.length();
    return seq.length();
}

float IndexdiffScorer::operator()(const Sequence& seq) {
    float result = 0.0;

    int s_len = seq.first_pair().source().get_text().length();
    int t_len = seq.first_pair().target().get_text().length();

    for (Sequence::iterator pair = seq.begin(); pair != seq.end(); ++pair)
        result += abs((pair->slot() / s_len)
                - (pair->target_slot() / t_len));

    result = 1 - (result / seq.length());

    _max = (_max > result) ? _max : result;

    return result;
}

float BisimScorer::operator()(const Sequence& seq) {
    float result = 0.0;

    for (Sequence::iterator pair = seq.begin(); pair != seq.end(); ++pair)
        result +=
                bi_sim::bi_sim(pair->source().get_str(),
                               pair->target().get_str()) / seq.length();

    _max = (_max > result) ? _max : result;

    return result;
}

