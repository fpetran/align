// Copyright 2012 Florian Petran
#include"align.h"

#include<map>
#include<vector>
#include<utility>
#include<algorithm>
#include<list>
#include<stdexcept>

#include"params.h"
#include"text.h"
#include"dictionary.h"
#include"containers.h"
#include"string_impl.h"

using std::list;
using std::vector;
using std::pair;
using std::runtime_error;

//////////////////////////// Candidates ///////////////////////////////////////

Candidates::Candidates(const Dictionary& dict) {
    _dict = &dict;
}

void Candidates::collect() {
    for (const WordToken& word : *_dict->get_e()) {
        if (!_dict->has(word))
            continue;

        list<WordType> f_types = _dict->lookup(word);
        list<WordToken>* f_tokens = new list<WordToken>();
        for (const WordType& f_type : f_types)
            for (const WordToken& f_token : f_type.get_tokens())
                f_tokens->push_back(f_token);

        _translations[word] = f_tokens;
    }
}

Candidates::~Candidates() {
    for (auto tr = _translations.begin();
         tr != _translations.end();
         ++tr)
        delete tr->second;
}

///////////////////////// SequenceContainer ///////////////////////////////////

SequenceContainer::SequenceContainer(Candidates* c) {
    this->_candidates = c;
    this->_dict = c->_dict;
    this->params = Params::get();
}

SequenceContainer::~SequenceContainer() {
    for (Sequence* seq : _list)
        delete seq;
}

SequenceContainer::iterator SequenceContainer::begin() const {
    return _list.begin();
}
SequenceContainer::iterator SequenceContainer::end() const {
    return _list.end();
}

void SequenceContainer::make(const BreakAfterPhase br_phase) {
    initial_sequences();
    if (br_phase == BreakAfterPhase::Initial)
        return;
    expand_sequences();
    if (br_phase == BreakAfterPhase::Expand)
        return;

    merge_sequences();
    if (br_phase == BreakAfterPhase::Merge)
        return;
    //cout << "Collecting scores..." << endl;
    collect_scores();
    //cout << "...Done." << endl;
    //cout << "Discarding all but topranking..." << endl;
    get_topranking();
    //cout << "...Done." << endl;
}

SequenceContainer& SequenceContainer::initial_sequences() {
    for (auto cand1 = _candidates->begin();
         cand1 != _candidates->end(); ++cand1) {
        if (cand1->second->empty())
            continue;

        auto cand2 = cand1; ++cand2;
        int skipped = 0;
        while (skipped <= params->max_skip()
            && cand2->second->empty()
            && cand2 != _candidates->end()) {
            ++skipped;
            ++cand2;
        }

        if (skipped > params->max_skip()
         || cand2 == _candidates->end())
            continue;

        const WordToken& e1 = cand1->first,
                         e2 = cand2->first;
        list<WordToken> *e1_translations = cand1->second,
                        *e2_translations = cand2->second;
        auto f1 = e1_translations->begin(),
             f2 = e2_translations->begin();

        while (f1 != e1_translations->end()) {
            bool f1_used = false;
            f2 = e2_translations->begin();
            while (f2 != e2_translations->end()) {
                if (f1->position() < f2->position()
                    && f1->close_to(*f2)) {
                    _list.push_back(new Sequence(*_dict,
                                                 Pair(e1, *f1),
                                                 Pair(e2, *f2)));
                    f1_used = true;
                    f1 = e1_translations->erase(f1);
                    f2 = e2_translations->erase(f2);
                } else
                    ++f2;
            }
            if (!f1_used)
                ++f1;
        }
    }

    return *this;
}

SequenceContainer& SequenceContainer::expand_sequences() {
    int pairs_added;
    // XXX the checks for closeness
    // should use abs and check if
    // next->first > seq->back_slot too
    do {
        pairs_added = 0;
        for (Sequence* seq : _list) {
            // get next candidates entry for the sequence
            Candidates::iterator next_slot;
            for (next_slot = _candidates->begin();
                 next_slot != _candidates->end(); ++next_slot)
                if (next_slot->first.position()
                    > seq->last_pair().source().position()
                 && !next_slot->second->empty())
                    break;

            if (next_slot == _candidates->end())
                continue;

            for (auto tr = next_slot->second->begin();
                 tr != next_slot->second->end(); ++tr) {
                Pair p(next_slot->first, *tr);
                if (seq->add_if_close(p)) {
                    ++pairs_added;
                    tr = next_slot->second->erase(tr);
                }
            }
        }
    } while (pairs_added != 0);

    return *this;
}

SequenceContainer& SequenceContainer::merge_sequences() {
    int combined = 0;

    do {
        combined = 0;
        // can't use our typedef here, because these aren't const
        for (list<Sequence*>::iterator seq = _list.begin();
                seq != _list.end(); ++seq) {
            list<Sequence*>::iterator other = seq;

            while (other != _list.end() &&
                    (*other)->slot() <= (*seq)->back_slot())
                ++other;

            if (other == _list.end())
                continue; // break instead?

            if ((*seq)->last_pair().both_close((*other)->first_pair())) {
                (*seq)->merge(**other);
                other = _list.erase(other);
                ++combined;
            }
        }
    } while (combined != 0);

    return *this;
}

SequenceContainer& SequenceContainer::collect_scores() {
    // it's time to settle the score
    list<vector<float>> scores_all = list<vector<float>>();

    // collect raw scores for all sequences
    for (Sequence* seq : _list) {
        scores_all.push_back(vector<float>());
        for (Scorer* scorer : scoring_methods)
            scores_all.back().push_back((*scorer)(*seq));
    }

    // normalize scores
    for (auto sc = scores_all.begin(); sc != scores_all.end(); ++sc)
        for (unsigned int ii = 0; ii < scoring_methods.size(); ++ii)
            sc->at(ii) /= scoring_methods.at(ii)->get_max();

    // collect overall score from single methods
    auto score = scores_all.begin();
    // v-- because nested typedef is const iterator and we modify here!
    list<Sequence*>::iterator seq = _list.begin();
    while (score != scores_all.end() || seq != _list.end()) {
        float cumul_score = 0;
        for (auto s = score->begin(); s != score->end(); ++s)
            cumul_score += *s;
        cumul_score /= scoring_methods.size();
        (*seq)->set_score(cumul_score);
        ++seq;
        ++score;
    }

    return *this;
}

SequenceContainer& SequenceContainer::get_topranking() {
    // the lambda removes other seqs with lower or equal
    // scores for a token, and returns true if the score
    // was equal. if it was equal, the sequence we're looking
    // at needs to be removed as well.
    auto remove_others =
        [&](const WordToken& tok, const Sequence* seq) -> bool {
            bool equals = false;
            auto other_seq = tok.get_sequences()->begin();
            while (other_seq != tok.get_sequences()->end()) {
                if (*other_seq == seq) {
                    ++other_seq;
                    continue;
                }
                if ((*other_seq)->get_score() == seq->get_score())
                    equals = true;
                if ((*other_seq)->get_score() <= seq->get_score()) {
                    auto seq_to_remove = other_seq;
                    ++other_seq;
                    _list.remove(*seq_to_remove);
                    tok.remove_from(*seq_to_remove);
                    if (tok.get_sequences()->size() == 0)
                        break;
                } else
                    ++other_seq;
            }
            return equals;
        };

    for (auto seq = _list.begin(); seq != _list.end(); ++seq) {
        if ((*seq)->length() <= 2) {
            seq = _list.erase(seq);
            continue;
        }
        bool delete_me = false;
        for (const Pair& p : **seq) {
            if (remove_others(p.target(), *seq))
                delete_me = true;
            if (remove_others(p.source(), *seq))
                delete_me = true;
        }
        if (delete_me)
            seq = _list.erase(seq);
    }

    return *this;
}

const SequenceContainer& SequenceContainer::reverse() {
    _dict = DictionaryFactory::get_instance()
        ->get_dictionary(_dict->get_f()->filename(),
                         _dict->get_e()->filename());

    for (Sequence* seq : _list)
        seq->reverse();

    return *this;
}

const SequenceContainer& SequenceContainer::merge(const
        SequenceContainer& that) {
    if (this->_dict != that._dict)
        throw runtime_error("Dictionaries don't match - aborting merge");

    list<Sequence*>::iterator this_seq = this->_list.begin();

    for (iterator that_seq = that.begin();
         that_seq != that.end(); ++that_seq) {
        while ((*this_seq)->slot() < (*that_seq)->slot())
            ++this_seq;
        while ((*this_seq)->slot() == (*that_seq)->slot()) {
            if (*this_seq == *that_seq)
                break;
            ++this_seq;
        }
        if (*this_seq == *that_seq)
            continue;
        _list.insert(++this_seq, *that_seq);
    }

    return *this;
}

