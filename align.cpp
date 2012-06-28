// Copyright 2012 Florian Petran
#include<map>
#include<vector>

#include<iostream>

#include"align.h"
#include"params.h"
#include"containers.h"

Candidates::Candidates(const Dictionary& dict) {
    _dict = &dict;
    pair_factory = std::shared_ptr<PairFactory>(new PairFactory(dict));
}

Candidates::~Candidates() {
    pair_factory.reset();
}

void Candidates::collect() {
    for ( int i = 0; i < _dict->get_e()->length(); ++i ) {
        Word w = _dict->get_e()->at(i);

        if (!_dict->has(w))
            continue;

        _translations.insert(make_pair(i, _dict->lookup(w)));
    }
}

Translations::const_iterator Candidates::begin() const
    { return _translations.begin(); }
Translations::const_iterator Candidates::end() const
    { return _translations.end(); }

SequenceContainer::SequenceContainer(const Candidates& c) {
    this->pair_factory = c.pair_factory;
    this->_dict = c._dict;
    this->_translations = c._translations;
}


SequenceContainer::~SequenceContainer() {
    pair_factory.reset();
}

SequenceContainer::iterator SequenceContainer::begin() const {
    return _list.begin();
}
SequenceContainer::iterator SequenceContainer::end() const {
    return _list.end();
}

void SequenceContainer::make(const BreakAfterPhase br_phase) {
    initial_sequences();
    if( br_phase == BreakAfterInitial )
        return;
    expand_sequences();
    if( br_phase == BreakAfterExpand )
        return;
    // repeat same process in reverse
    merge_sequences();
    if( br_phase == BreakAfterMerge )
        return;
    collect_scores();
    // collect scores
    // remove all but topranking
}

void SequenceContainer::initial_sequences() {
    Text *e = _dict->get_e();

    Translations::iterator
        me = _translations.begin(), you = me;
    ++you;

    while (me != _translations.end() && you != _translations.end()) {
        // skip words with no candidate
        unsigned int skipped = 0;
        while (!_dict->has(e->at(you->first))) {
            ++you;
            ++skipped;
        }

        if (skipped <= Params::get()->max_skip())
            for (TranslationsEntry::iterator t1 = me->second->begin();
                    t1 != me->second->end(); ++t1) {

                Pair p1 = pair_factory->make_pair(me->first, *t1);
                for (TranslationsEntry::iterator t2 = you->second->begin();
                        t2 != you->second->end(); ++t2) {

                    Pair p2 = pair_factory->make_pair(you->first, *t2);
                    // TODO allow for multiple sequence to be formed
                    if (p1.targets_close(p2)) {
                        _list.push_back(Sequence(*_dict, p1, p2));
                        t2 = you->second->erase(t2);
                        t1 = me->second->erase(t1);
                    }
                }
            }
        ++me;
        ++you;
    }
}

void SequenceContainer::expand_sequences() {
    unsigned int pairs_added;
    // XXX the checks for closeness
    // should use abs and check if
    // next->first > seq->back_slot too

    do {
        pairs_added = 0;
        for (std::list<Sequence>::iterator seq = _list.begin();
                seq != _list.end(); ++seq) {

            Translations::iterator next_slot =
                _translations.find(seq->slot());
            ++next_slot;

            while (next_slot != _translations.end()
                    && !next_slot->second->empty()) {
                if (seq->back_slot() - next_slot->first
                        >= Params::get()->closeness())
                    break;
                ++next_slot;
            }

            if (next_slot == _translations.end()
                    || next_slot->second->empty()
                    || next_slot->first - seq->back_slot()
                        > Params::get()->closeness())
                continue;


            for (TranslationsEntry::iterator tr = next_slot->second->begin();
                    tr != next_slot->second->end(); ++tr) {
                Pair p = pair_factory->make_pair( next_slot->first, *tr );
                if (seq->last_pair().targets_close(p)
                        && ! seq->has_target( p )) {
                    seq->add(p);
                    ++pairs_added;
                    tr = next_slot->second->erase(tr);
                }
            }

        }
    } while (pairs_added != 0);

    // at this point, we can clear the translation candidates
    // also, the TranslationsEntry ptr must be deleted here, otherwise,
    // leakage will probably occur
    // TODO does this invalidate Candidates' _translations? probably so.
    for (Translations::iterator tr = _translations.begin();
            tr != _translations.end(); ++tr ) {
        delete tr->second;
    }
    _translations.clear();
}

void SequenceContainer::merge_sequences() {
    unsigned int combined = 0;

    do {
        combined = 0;

        // can't use our typedef here, because these aren't const
        for (std::list<Sequence>::iterator seq = _list.begin();
                seq != _list.end(); ++seq) {
            std::list<Sequence>::iterator other = seq;

            while (other != _list.end() &&
                    other->slot() <= seq->back_slot())
                ++other;

            if (other == _list.end())
                continue; // break instead?

            if (seq->last_pair().both_close(other->first_pair())) {
                seq->merge(*other);
                other = _list.erase(other);
                ++combined;
            }
        }

    } while (combined != 0);

}

void SequenceContainer::collect_scores() {
    // it's time to settle the score
    std::list<std::vector<float>> scores_all = std::list<std::vector<float>>();

    // collect raw scores for all sequences
    for (SequenceContainer::iterator seq = _list.begin();
            seq != _list.end(); ++seq) {
        scores_all.push_back( std::vector<float>() );
        for (unsigned int ii = 0; ii < scoring_methods.size(); ++ii)
            scores_all.back().push_back((*scoring_methods[ii])( *seq ));
    }

    // normalize scores
    for (std::list<std::vector<float>>::iterator sc = scores_all.begin(); sc != scores_all.end(); ++sc)
        for (unsigned int ii = 0; ii < scoring_methods.size() - 1; ++ii)
            sc->at(ii) /= scoring_methods.at(ii)->get_max();

    // collect overall score from single methods
    std::list<std::vector<float>>::iterator score = scores_all.begin();
    // v-- because nested typedef is const iterator and we modify here!
    std::list<Sequence>::iterator seq = _list.begin();
    while (score != scores_all.end() || seq != _list.end()) {
        float cumul_score = 0;
        for (std::vector<float>::iterator s = score->begin();
                s != score->end();
                ++s)
            cumul_score += *s;
        cumul_score /= scoring_methods.size();
        seq->set_score(cumul_score);
        ++seq; ++score;
    }
}

