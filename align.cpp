// Copyright 2012 Florian Petran
#include<map>
#include<iostream>

#include"align.h"
#include"params.h"
#include"containers.h"

Candidates::Candidates(const Dictionary& dict) {
    _dict = &dict;
    pair_factory = new PairFactory(dict);

    // this sucks. candidates constructor shouldn't do
    // the alignment work. but for now, i'll leave it
    // until i've found a better place for it.
    // TODO XXX FIXME
    //
    // in principle, candidates ctor should collect candidates
    // sequences init, expanding and merging should then be
    // done in a separate sequence container object
    //collect_candidates();
    //initial_sequences();
    //expand_sequences();
    // repeat same process in reverse
    //merge_sequences();
    // collect scores
    // remove all but topranking
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

std::list<Sequence>::const_iterator SequenceContainer::begin() const {
    return _list.begin();
}
std::list<Sequence>::const_iterator SequenceContainer::end() const {
    return _list.end();
}

void SequenceContainer::make(const BreakAfterPhase br_phase) {
    initial_sequences();
    if( br_phase == BreakAfterInitial )
        return;
    expand_sequences();
    if( br_phase == BreakAfterExpand )
        return;
    merge_sequences();
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
                        t2 = you->second->erase( t2 );
                    }
                }
            }
        ++me;
        ++you;
    }
}

void SequenceContainer::expand_sequences() {
    unsigned int pairs_added;
    std::list<Sequence>::iterator current = _list.begin();

    do {
        pairs_added = 0;
        // look at the next slot from the end of the sequence
        unsigned int next_slot = current->back_slot() + 1;

        // unless that has no candidate
        // or it exceeds max_cand_diff TODO
        while (next_slot < _translations.size()
                && _translations[next_slot]->size() == 0)
            ++next_slot;

        if (next_slot < _translations.size()
          && next_slot - current->back_slot() <= Params::get()->closeness()) {
            // go through all translations for next_candidate
            TranslationsEntry::iterator it =
                _translations[next_slot]->begin();

            while (it != _translations[next_slot]->end()) {
                Pair p = pair_factory->make_pair( next_slot, *it );

                if (current->last_pair().both_close(p)) {
                    // TODO if pairs_added >= 1
                    // make a copy of original sequence
                    // add pair to sequence
                    current->add(p);
                    ++pairs_added;
                    it = _translations[next_slot]->erase( it );
                }
                else
                    ++it;
            }
        }

        ++current;
        if (current == _list.end())
            current = _list.begin();
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
    std::list<Sequence>::iterator current = _list.begin();

    while (true) {
        combined = 0;
        std::list<Sequence>::iterator next = ++current;
        while (next != _list.end() &&
                next->slot() <= current->back_slot())
            ++next;

        if (next != _list.end() &&
                current->last_pair().both_close(next->first_pair())) {
            current->merge(*next);
            next = _list.erase(next);
            ++combined;
        }

        ++current;

        /*
        if (abs(next->slot() - current->back_slot()) > Params::get()->closeness()) {
            int slot = current->slot();
            while (slot == current->slot()) {
                ++current;
            }
        }
        */

        if (current == _list.end() || next == _list.end()) {
            if (combined == 0)
                break;
            else
                current = _list.begin();
        }

    }
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
