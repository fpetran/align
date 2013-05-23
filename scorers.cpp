// Copyright 2012 Florian Petran
#include"scorers.h"

#include<cmath>

#include<vector>

#include"bi-sim.h"

using std::vector;

namespace Align {

ScoringMethods::ScoringMethods() {
    // register all scoring methods here
    this->push_back(new LengthScorer);
    this->push_back(new IndexdiffScorer);
    this->push_back(new BisimScorer);
}

ScoringMethods::~ScoringMethods() {
    for (vector<Scorer*>::iterator sc = this->begin();
            sc != this->end(); ++sc)
        delete *sc;
}

Scorer::Scorer() : _max(0.0) {}

float LengthScorer::operator()(const Sequence& seq) {
    _max = (_max > seq.length()) ? _max : seq.length();
    return seq.length();
}

float IndexdiffScorer::operator()(const Sequence& seq) {
    float result = 0.0;

    float s_len = seq.first_pair().source().get_text().length();
    float t_len = seq.first_pair().target().get_text().length();

    for (auto pair = seq.cbegin(); pair != seq.cend(); ++pair)
        result += fabs((pair->slot() / s_len)
                     - (pair->target_slot() / t_len));

    result = 1 - (result / seq.length());

    _max = (_max > result) ? _max : result;

    return result;
}

float BisimScorer::operator()(const Sequence& seq) {
    float result = 0.0;

    for (auto pair = seq.cbegin(); pair != seq.cend(); ++pair)
        result +=
                bi_sim::bi_sim(pair->source().get_str(),
                               pair->target().get_str()) / seq.length();

    _max = (_max > result) ? _max : result;

    return result;
}
}

