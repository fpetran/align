// Copyright 2012 Florian Petran
#include<list>
#include<map>
#include<vector>
#include<stdexcept>

#include"containers.h"
#include"params.h"

using std::runtime_error;

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

namespace Align {

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
    return this->_target.close_to(that._target);
}
bool Pair::both_close(const Pair& that) const {
    return targets_close(that)
        && this->_source.close_to(that._source);
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
    _dict = &dict;
    this->add(p);
}

Sequence::~Sequence() {
    for (Pair& p : *this) {
        p.target().remove_from(this);
        p.source().remove_from(this);
    }
}

const Sequence& Sequence::operator=(const Sequence& that) {
    _dict = that._dict;
    for (Pair& p : *this) {
        p.target().remove_from(this);
        p.source().remove_from(this);
    }
    _length = 0;
    for (auto p = that.cbegin(); p != that.cend(); ++p) {
        this->add(*p);
        _slot = p->slot();
        ++_length;
    }
    return *this;
}

Sequence::operator std::vector<int>() {
    std::vector<int> vec;
    for (auto p = cbegin(); p != cend(); ++p) {
        vec.push_back(p->slot());
        vec.push_back(p->target_slot());
    }
    return vec;
}

Hypothesis::operator std::vector<std::vector<int>>() {
    std::vector<std::vector<int>> vec;
    for (auto s = cbegin(); s != cend(); ++s)
        vec.push_back(std::vector<int>(**s));
    return vec;
}

void Sequence::add(const Pair& p) {
    if (_length == 0)
        _slot = p.slot();
    this->push_back(p);
    _back_slot = p.slot();
    p.source().add_to_sequence(this);
    p.target().add_to_sequence(this);
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

void Sequence::merge(Sequence* that) {
    for (auto pp = that->begin();
            pp != that->end(); ++pp) {
        this->add(*pp);
        pp->target().remove_from(that);
        pp->source().remove_from(that);
    }
    _back_slot = this->back().slot();
    _length += that->length();
}

const Sequence& Sequence::reverse() {
    _dict = DictionaryFactory::get_instance()
             .get_dictionary(_dict->get_f()->filename(),
                             _dict->get_e()->filename());
    for (Pair& pair : *this)
        pair.reverse();

    return *this;
}

int Sequence::slot() const
    { return _slot; }

int Sequence::back_slot() const
    { return _back_slot; }

bool Sequence::operator==(const Sequence& that) const {
    if (this->length() != that.length())
        return false;
    auto this_it = this->cbegin(),
         that_it = that.cbegin();
    while (this_it != this->cend() && that_it != that.cend()) {
        if (*this_it != *that_it)
            return false;
        ++this_it;
        ++that_it;
    }
    return true;
}
bool Sequence::has_target(int target_pos) {
    for (Sequence::iterator pp = this->begin();
            pp != this->end(); ++pp)
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

const Pair& Sequence::first_pair() const {
    return this->front();
}

const Pair& Sequence::last_pair() const {
    return this->back();
}

///////////////////////////////// Hypothesis ///////////////////////////////////

Hypothesis::Hypothesis(const Dictionary& d) {
    _dict = &d;
}

Hypothesis::~Hypothesis() {
    for (Sequence* seq : *this)
        delete seq;
}

Sequence* Hypothesis::new_sequence(const Pair& p) {
    Sequence* seq = new Sequence(*_dict, p);
    this->push_back(seq);
    return seq;
}

Hypothesis::iterator Hypothesis::remove_sequence(Hypothesis::iterator pos) {
    if (pos == this->end())
        return pos;
    Sequence* seq = *pos;
    delete seq;
    Hypothesis::iterator newpos = this->erase(pos);
    return newpos;
}

Hypothesis::iterator Hypothesis::remove_sequence(Sequence* seq) {
    Hypothesis::iterator pos;
    for (pos = this->begin(); pos != this->end(); ++pos)
        if (*pos == seq)
            break;
    return this->remove_sequence(pos);
}

const Hypothesis& Hypothesis::reverse() {
    _dict = DictionaryFactory::get_instance()
             .get_dictionary(_dict->get_f()->filename(),
                             _dict->get_e()->filename());
    for (Sequence* seq : *this)
        seq->reverse();

    return *this;
}

const Hypothesis& Hypothesis::munch(Hypothesis *that) {
    if (this->_dict != that->_dict)
        throw runtime_error("Dictionaries don't match - aborting merge");

    auto this_seq = this->begin();
    for (auto that_seq = that->begin();
         that_seq != that->end();
         ++that_seq) {
        while ((*this_seq)->slot() < (*that_seq)->slot())
            ++this_seq;
        while ((*this_seq)->slot() == (*that_seq)->slot()) {
            if (*this_seq == *that_seq)
                break;
            ++this_seq;
        }
        if (*this_seq == *that_seq)
            continue;
        this->insert(++this_seq, *that_seq);
    }

    return *this;
}
}  // namespace Align

std::ostream& operator<<(std::ostream& strm, const Align::Pair& pair) {
    strm << "[ "
         << pair.slot() << " (" << pair.source().get_str() << ") -- "
         << pair.target_slot() << " (" << pair.target().get_str() << ") ] ";
    return strm;
}

std::ostream& operator<<(std::ostream& strm, const Align::Sequence& seq) {
    strm << "{ ";
    for (auto pair = seq.cbegin(); pair != seq.cend(); ++pair)
        strm << *pair;
    strm << " }";
    return strm;
}

