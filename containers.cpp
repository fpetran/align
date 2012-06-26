// Copyright 2012 Florian Petran
#include<list>
#include<map>

#include"containers.h"
#include"params.h"

/*
 * optimization notes:
 * Sequence::slot() and Sequence::back_slot() took up a lot of time
 * caching the values in the sequence object did save a little time
 * for slot() but considerably more for back_slot(), relegating it from
 * 30.75% of the time to 11.48% in an intermediate version. time savings
 * for slot() were only from 53.29% to 51.27%
 *
 * reason for this is probably that slot gets called about 10 times as
 * often as back_slot - it's used to move through the sequence list.
 *
 * 1003154053 slot()
 *  983231170 back_slot()
 *
 * also, back_slot is probably more expensive since it has to move through
 * the pairs in the sequence to reach the end of the linked list.
 *
 * other idea:
 *  move caching to back_slot() and create cache upon first request
 *  requires cache_valid storage
 *  Problem: back_slot() is const -> cache variable must be mutable
 */

PairFactory::PairFactory(const Dictionary& dict) {
    _dict = &dict;
}

Pair PairFactory::make_pair(int ei, int fi) {
    return Pair(
            _dict->get_e()->at(ei), ei,
            _dict->get_f()->at(fi), fi
            );
}

Pair::Pair(const Word& s, int s_pos, const Word& t, int t_pos) {
    _source = s;
    _ei = s_pos;
    _target = t;
    _fi = t_pos;
}

int Pair::slot() const {
    return _ei;
}

int Pair::target_slot() const {
    return _fi;
}

bool Pair::targets_close(const Pair& that) const {
    // TODO parametrize monotony constraint
    return
            this->_ei != that._ei
        &&  this->_ei < that._ei
        &&  this->_fi < that._fi
        &&  abs(this->_fi - that._fi) <= Params::get()->closeness();
}
bool Pair::both_close(const Pair& that) const {
    return this->targets_close(that)
        && abs(this->_ei - that._ei) <= Params::get()->closeness();
}

const Word& Pair::source() const { return _source; }
const Word& Pair::target() const { return _target; }

Sequence::Sequence(const Dictionary& dict, const Pair& p) {
    _dict = &dict;
    this->add(p);
    _slot = _back_slot = p.slot();
}

Sequence::Sequence(const Dictionary& dict, const Pair& p1, const Pair& p2) {
    _dict = &dict;
    this->add(p1);
    this->add(p2);
    _slot = p1.slot();
    _back_slot = p2.slot();
}

void Sequence::add(const Pair& p) {
    _list.push_back(p);
    _back_slot = p.slot();
}

void Sequence::merge(const Sequence& that) {
    for (Sequence::iterator pp = that._list.begin();
            pp != that._list.end(); ++pp)
        this->_list.push_back(*pp);
    _back_slot = _list.back().slot();
}

int Sequence::slot() const
    { return _slot; }
    //{ return _list.front().slot(); }

int Sequence::back_slot() const
    { return _back_slot; }
    //{ return _list.back().slot(); }

bool Sequence::has_target(unsigned int target_pos) {
    for (Sequence::iterator pp = _list.begin();
            pp != _list.end(); ++pp)
        if (pp->target_slot() == target_pos)
            return true;
    return false;
}

bool Sequence::has_target( const Pair& other ) {
    return this->has_target(other.target_slot());
}

Sequence::iterator Sequence::begin() const
    { return _list.begin(); }

Sequence::iterator Sequence::end() const
    { return _list.end(); }

const Pair& Sequence::first_pair() const {
    return _list.front();
}

const Pair& Sequence::last_pair() const {
    return _list.back();
}

