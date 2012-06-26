// Copyright 2012 Florian Petran
#include<map>
#include<iostream>

#include"align.h"
#include"params.h"
#include"containers.h"

Candidates::Candidates(const Dictionary& dict) {
    _dict = &dict;
    pair_factory = new PairFactory(dict);
}

Candidates::~Candidates() {
    delete pair_factory;
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

    do {
        pairs_added = 0;
        for (std::list<Sequence>::iterator seq = _list.begin();
                seq != _list.end(); ++seq) {

            Translations::iterator next_slot =
                _translations.find(seq->slot());
            ++next_slot;

            while (next_slot != _translations.end()
                    && ! next_slot->second->empty()) {
                if (seq->back_slot() - next_slot->first >= Params::get()->closeness())
                    break;
                ++next_slot;
            }

            if (next_slot == _translations.end())
                continue;

            if (next_slot->second->empty()
                    || next_slot->first - seq->back_slot() > Params::get()->closeness())
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

#if 0
// this is total bs
// scoring needs to be moved to its own class,
// probably scoring methods should be functors
// or something
namespace {
    enum {
        SEQ_LENGTH = 1,
        INDEX_DIFF,
        BI_SIM
    } scoring_methods;
}

void Candidates::assign_scores() {
    std::vector<float> score_max;

    for( std::list<Sequence>::const_iterator seq = _list.begin(); seq != _list.end(); ++seq ) {
        for( std::list<Pair>::const_iterator p = seq->begin(); p != seq->end(); ++p )
            ;
    }
}
#endif
