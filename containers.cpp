// Copyright 2012 Florian Petran
#include<list>
#include<map>
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
    // TODO(fpetran) parametrize monotony constraint
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
    _length = 0;
    _dict = &dict;
    this->add(p);
}

Sequence::Sequence(const Dictionary& dict, const Pair& p1, const Pair& p2) {
    _length = 0;
    _dict = &dict;
    this->add(p1);
    this->add(p2);
    _slot = p1.slot();
}

Sequence::~Sequence() {
    for (Pair& p : _list) {
        p.target().remove_from(this);
        p.source().remove_from(this);
    }
}

const Sequence& Sequence::operator=(const Sequence& that) {
    _dict = that._dict;
    for (Pair& p : _list) {
        p.target().remove_from(this);
        p.source().remove_from(this);
    }
    _length = 0;
    for (const Pair& p : that) {
        this->add(p);
        _slot = p.slot();
        ++_length;
    }
    return *this;
}


void Sequence::add(const Pair& p) {
    if (_length == 0)
        _slot = p.slot();
    _list.push_back(p);
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

void Sequence::merge(const Sequence& that) {
    for (Sequence::iterator pp = that._list.begin();
            pp != that._list.end(); ++pp) {
        this->add(*pp);
        pp->target().remove_from(const_cast<Sequence*>(&that));
        pp->source().remove_from(const_cast<Sequence*>(&that));
    }
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

int Sequence::back_slot() const
    { return _back_slot; }

bool Sequence::operator==(const Sequence& that) const {
    if (this->length() != that.length())
        return false;
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

///////////////////////////////// Hypothesis ///////////////////////////////////

Hypothesis::Hypothesis(const Dictionary& d) {
    _dict = &d;
}

Hypothesis::~Hypothesis() {
    for (Sequence* seq : _sequences)
        delete seq;
}

Sequence* Hypothesis::new_sequence(const Pair& p) {
    Sequence* seq = new Sequence(*_dict, p);
    _sequences.push_back(seq);
    return seq;
}

Hypothesis::iterator Hypothesis::remove_sequence(Hypothesis::iterator pos) {
    if (pos == _sequences.end())
        return pos;
    Sequence* seq = *pos;
    delete seq;
    Hypothesis::iterator newpos = _sequences.erase(pos);
    return newpos;
}

Hypothesis::iterator Hypothesis::remove_sequence(Sequence* seq) {
    Hypothesis::iterator pos;
    for (pos = _sequences.begin(); pos != _sequences.end(); ++pos)
        if (*pos == seq)
            break;
    return this->remove_sequence(pos);
}

const Hypothesis& Hypothesis::reverse() {
    _dict = DictionaryFactory::get_instance()
            ->get_dictionary(_dict->get_f()->filename(),
                             _dict->get_e()->filename());
    for (Sequence* seq : _sequences)
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
        _sequences.insert(++this_seq, *that_seq);
    }

    return *this;
}
}

std::ostream& operator<<(std::ostream& strm, const Align::Pair& pair) {
    strm << "[ "
         << pair.slot() << " (" << pair.source().get_str() << ") -- "
         << pair.target_slot() << " (" << pair.target().get_str() << ") ] ";
    return strm;
}

std::ostream& operator<<(std::ostream& strm, const Align::Sequence& seq) {
    strm << "{ ";
    for (const Align::Pair& pair : seq)
        strm << pair;
    strm << " }";
    return strm;
}

