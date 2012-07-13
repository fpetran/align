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

Pair::Pair(const WordToken& s, const WordToken& t)
    : _source(s), _target(t) {
}

int Pair::slot() const {
    return _source.position();
}

int Pair::target_slot() const {
    return _target.position();
}

bool Pair::operator==(const Pair& that) const {
    return this->_source == that._source
        && this->_target == that._target;
}

const Pair& Pair::reverse() {
    WordToken tmp = _source;
    _source = _target;
    _target = tmp;
    return *this;
}

bool Pair::targets_close(const Pair& that) const {
    // TODO parametrize monotony constraint
    return
            this->_source.position() != that._source.position()
        &&  this->_source.position() < that._source.position()
        &&  this->_target.position() < that._target.position()
        &&     abs(this->_target.position() - that._target.position())
            <= Params::get()->closeness();
}
bool Pair::both_close(const Pair& that) const {
    return
        this->targets_close(that)
     && abs(this->_source.position() - that._source.position())
           <= Params::get()->closeness();
}

const WordToken& Pair::source() const { return _source; }
const WordToken& Pair::target() const { return _target; }

///////////////////////////////// Sequence /////////////////////////////////////

void Sequence::set_score(const float& s) {
    _score = s;
}
const float& Sequence::get_score() const {
    return _score;
}

Sequence::Sequence(const Dictionary& dict) {
    _length = 0;
    _dict = &dict;
}

Sequence::Sequence(const Dictionary& dict, const Pair& p) {
    _length = 1;
    _dict = &dict;
    this->add(p);
}

Sequence::Sequence(const Dictionary& dict, const Pair& p1, const Pair& p2) {
    _length = 2;
    _dict = &dict;
    this->add(p1);
    this->add(p2);
    _slot = p1.slot();
}

void Sequence::add(const Pair& p) {
    if (_length == 0)
        _slot = p.slot();
    _list.push_back(p);
    _back_slot = p.slot();
    ++_length;
}

bool Sequence::add_if_close(const Pair& p) {
    if (_length == 0) {
        add(p);
        return true;
    }

    if (this->last_pair().targets_close(p)
     && !this->has_target(p)) {
        add(p);
        return true;
    }

    return false;
}

void Sequence::merge(const Sequence& that) {
    for (Sequence::iterator pp = that._list.begin();
            pp != that._list.end(); ++pp)
        this->_list.push_back(*pp);
    _back_slot = _list.back().slot();
    _length += that.length();
}

const Sequence& Sequence::reverse() {
    _dict = DictionaryFactory::get_instance()
            ->get_dictionary(_dict->get_f()->filename(),
                             _dict->get_e()->filename());
    for (Pair& pair : _list)
        pair.reverse();

    return *this;
}

int Sequence::slot() const
    { return _slot; }
    //{ return _list.front().slot(); }

int Sequence::back_slot() const
    { return _back_slot; }
    //{ return _list.back().slot(); }

bool Sequence::operator==(const Sequence& that) const {
    auto this_it = this->begin(),
         that_it = that.begin();
    while (this_it != this->end() && that_it != that.end()) {
        if (*this_it != *that_it)
            return false;
        ++this_it;
        ++that_it;
    }
    return true;
}
bool Sequence::has_target(int target_pos) {
    for (Sequence::iterator pp = _list.begin();
            pp != _list.end(); ++pp)
        if (pp->target_slot() == target_pos)
            return true;
    return false;
}

bool Sequence::has_target(const Pair& other) {
    return this->has_target(other.target_slot());
}

int Sequence::length() const {
    return _length;
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

