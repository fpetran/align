// Copyright 2012 Florian Petran
#include"align.h"
#include<map>
#include<vector>

PairFactory::PairFactory(const Dictionary& dict) {
    _dict = &dict;
}

Pair PairFactory::make_pair(int ei, int fi) {
    return Pair(_dict->get_e()->at(ei), ei, _dict->get_f()->at(fi), fi);
}

Pair::Pair(const Word& s, int s_pos, const Word& t, int t_pos) {
    _source = s;
    _ei = s_pos;
    _target = t;
    _fi = t_pos;
}

int Pair::slot() {
    return _ei;
}

bool Pair::is_close_to(const Pair& that) {
    return
            abs(this->_ei - that._ei) <= 2  // XXX magic number = evil
        &&  this->_ei != that._ei;
}

Sequence::Sequence(const Dictionary& dict, const Pair& p) {
    _dict = &dict;
    this->add(p);
}

Sequence::Sequence(const Dictionary& dict, const Pair& p1, const Pair& p2) {
    _dict = &dict;
    this->add(p1);
    this->add(p2);
}

void Sequence::add(const Pair& p) {
    _list.push_back(p);
}

int Sequence::slot() {
    return _list.front().slot();
}

Candidates::Candidates(const Dictionary& dict) {
    _dict = &dict;
    pair_factory = new PairFactory(dict);

    collect_candidates();
    initial_sequences();

    // construct initial sequences with one pair each
    /*
    Text *e = dict.get_e(), *f = dict.get_f();
    std::vector<int> t;
    for( int i = 0; i < e->length(); ++i ) {
        Word w = e->at(i);
        try {
            t = dict.lookup( w );
            for( unsigned int j = 0; j < t.size(); ++j )
                _list.push_back( Sequence( dict, Pair( w, i, f->at(t[j]), t[j] ) ) );
        }
        catch( std::out_of_range ) {
            ;
        }
    }
    */

    // expand sequences
    // merge sequences
    // do reverse alignments
    // assign score
}

Candidates::~Candidates() {
    delete pair_factory;
}

void Candidates::collect_candidates() {
    for ( int i = 0; i < _dict->get_e()->length(); ++i ) {
        Word w = _dict->get_e()->at(i);

        if (!_dict->has(w))
            continue;

        _translations.insert(make_pair(i, _dict->lookup(w)));
    }
}

void Candidates::initial_sequences() {
    Text *e = _dict->get_e();

    std::map<int, std::vector<int> >::iterator
        me = _translations.begin(), you = me;
    ++you;

    while (me != _translations.end() && you != _translations.end()) {
        // skip words with no candidate
        // XXX introduce max skip here
        while (!_dict->has(e->at(you->first)))
            ++you;

        for (std::vector<int>::iterator t1 = me->second.begin();
                t1 != me->second.end(); ++t1) {
            Pair p1 = pair_factory->make_pair(me->first, *t1);
            for (std::vector<int>::iterator t2 = you->second.begin();
                    t2 != you->second.end(); ++t2) {
                Pair p2 = pair_factory->make_pair(you->first, *t2);
                if (p1.is_close_to(p2))
                    _list.push_back(Sequence(*_dict, p1, p2));
            }
        }
        ++me;
        ++you;
    }
}

void Candidates::expand_sequences() {
    int pairs_added = 0;
    std::list<Sequence>::iterator current = _list.begin();

    while (true) {
        int next_candidate = current->slot();

        for (std::vector<int>::iterator it = _translations[next_candidate].begin();
                it != _translations[next_candidate].end(); ++it) {
            Pair p = pair_factory.make_pair( next_candidate, *it );

            if( current->is_close_to(p) )
                ;
        }

        if (current == _list.end())
            current = _list.begin();
        if (pairs_added == 0)
            break;
    }

    // at this point, we can clear the translation candidates
    _translations.clear();
}
