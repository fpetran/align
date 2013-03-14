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
        list<WordToken> f_tokens;
        for (WordType& f_type : f_types)
            for (const WordToken& f_token : f_type.get_tokens())
                f_tokens.push_back(f_token);

        _translations.push_back(make_pair(word, f_tokens));
    }
}

///////////////////////// SequenceContainer ///////////////////////////////////

SequenceContainer::SequenceContainer(Candidates* c) {
    this->_candidates = c;
    this->_dict = c->_dict;
    this->params = Params::get();
}

/*
const SequenceContainer& SequenceContainer::operator=(
        const SequenceContainer& that) {
    this->_dict = that._dict;
    this->params = Params::get();

    this->_candidates = that._candidates;
    this->_list = that._list;

    return *this;
}
*/

/*
SequenceContainer::SequenceContainer(const SequenceContainer& that) {
    // dict and params can be copied as ptrs, since
    // they're not going to change anyway, dict
    // is owned by DictionaryFactory, and params is
    // a singleton.
    // I'm a bit doubtful as to how candidates will behave,
    // since it will get changed, but OTOH, the original
    // sequence shouldn't change it any more anyways. in any
    // case we need to probably think about ownership semantics
    // with the candidates pointer, which is currently all taken
    // care of by main()
    this->_dict = that._dict;
    this->params = that.params;
    this->_candidates = that._candidates;
}
*/

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

void SequenceContainer::initial_sequences() {
    Candidates::iterator source_second = _candidates->begin(),
                         source_first = source_second++;

    while (source_first != _candidates->end()
        && source_second != _candidates->end()) {
        // skip words with no candidate
        int skipped = 0;
        while (!_dict->has(source_second->first)
            && source_second != _candidates->end()) {
            ++source_second;
            ++skipped;
        }

        WordToken&  e1 = source_first->first,
                    e2 = source_second->first;

        list<WordToken>& e1_translations = source_first->second,
                         e2_translations = source_second->second;

        if (skipped <= params->max_skip())
            for (WordToken& f1 : e1_translations)
                for (WordToken& f2 : e2_translations)
                    // monotony constraint
                    // closeness condition
                    if (f1.position() < f2.position()
                        && f1.close_to(f2))
                        _list.push_back(Sequence(*_dict,
                                                Pair(e1, f1),
                                                Pair(e2, f2)));

        ++source_first;
        if (source_first != _candidates->end()) {
            source_second = source_first;
            ++source_second;
        }
    }
}

void SequenceContainer::expand_sequences() {
    int pairs_added;
    // XXX the checks for closeness
    // should use abs and check if
    // next->first > seq->back_slot too
    do {
        pairs_added = 0;
        for (Sequence& seq : _list) {
            // get next candidates entry for the sequence
            Candidates::iterator next_slot;
            for (next_slot = _candidates->begin();
                 next_slot != _candidates->end(); ++next_slot)
                if (next_slot->first.position()
                    > seq.last_pair().source().position()
                 && !next_slot->second.empty())
                    break;

            if (next_slot == _candidates->end())
                continue;

            for (list<WordToken>::iterator tr = next_slot->second.begin();
                 tr != next_slot->second.end(); ++tr) {
                Pair p(next_slot->first, *tr);
                if (seq.add_if_close(p)) {
                    ++pairs_added;
                    tr = next_slot->second.erase(tr);
                }
            }
        }
    } while (pairs_added != 0);
}

void SequenceContainer::merge_sequences() {
    int combined = 0;

    do {
        combined = 0;
        // can't use our typedef here, because these aren't const
        for (list<Sequence>::iterator seq = _list.begin();
                seq != _list.end(); ++seq) {
            list<Sequence>::iterator other = seq;

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
    list<vector<float>> scores_all = list<vector<float>>();

    // collect raw scores for all sequences
    for (Sequence& seq : _list) {
        scores_all.push_back(vector<float>());
        for (Scorer* scorer : scoring_methods)
            scores_all.back().push_back((*scorer)(seq));
    }

    // normalize scores
    for (auto sc = scores_all.begin(); sc != scores_all.end(); ++sc)
        for (int ii = 0; ii < scoring_methods.size(); ++ii)
            sc->at(ii) /= scoring_methods.at(ii)->get_max();

    // collect overall score from single methods
    auto score = scores_all.begin();
    // v-- because nested typedef is const iterator and we modify here!
    list<Sequence>::iterator seq = _list.begin();
    while (score != scores_all.end() || seq != _list.end()) {
        float cumul_score = 0;
        for (vector<float>::iterator s = score->begin();
                s != score->end();
                ++s)
            cumul_score += *s;
        cumul_score /= scoring_methods.size();
        seq->set_score(cumul_score);
        ++seq;
        ++score;
    }
}

// I probably deserve to be flogged for the abomination that follows
// all this mess could be prevented if a Word did know which sequences
// it appeared in... but it doesn't so far, so prepare for invasion of
// the lambdas
namespace {
    typedef list<list<Sequence>::iterator> it_list;
    typedef std::map<int, it_list> wordmap;

    void fill_maplist(wordmap* maps, list<Sequence>* li,
            std::function<int(Pair)> extractor) {
        for (list<Sequence>::iterator seq = li->begin();
                seq != li->end(); ++seq)
            for (const Pair& pair : *seq) {
                if (maps->count( extractor(pair) ) == 0)
                    (*maps)[extractor(pair)] = it_list();
                (*maps)[extractor(pair)].push_back(seq);
            }
    }
}

void SequenceContainer::get_topranking() {
    // anyway, collect maps for positions in e and f with iterators
    // to the sequences they appear in

    // it needs to be a vector so that we know by how much to resize
    // it afterwards
    vector<list<Sequence>::iterator> to_delete;

    auto mark_deletion = [&to_delete](wordmap* maps) {
        for (wordmap::iterator map = maps->begin(); map != maps->end();
                ++map) {
            while (map->second.size() > 1) {
                it_list::iterator s1 = map->second.begin(),
                                  s2 = map->second.begin();
                ++s2;
                float score1 = (*s1)->get_score(),
                      score2 = (*s2)->get_score();
                if (score1 == score2) {
                    to_delete.push_back(*s1);
                    to_delete.push_back(*s2);
                    map->second.erase(s2);
                    map->second.erase(s1);
                } else if (score1 < score2) {
                    to_delete.push_back(*s1);
                    map->second.erase(s1);
                } else {
                    to_delete.push_back(*s2);
                    map->second.erase(s2);
                }
            }
        }
    };

    wordmap source_maps, target_maps;

    fill_maplist(&source_maps, &_list,
                [](const Pair& p) -> int {
                    return p.slot();
                });
    mark_deletion(&source_maps);

    fill_maplist(&target_maps, &_list,
                [](const Pair& p) -> int {
                    return p.target_slot();
                });
    mark_deletion(&target_maps);

    std::sort(to_delete.begin(), to_delete.end(),
         [](list<Sequence>::iterator a,
            list<Sequence>::iterator b) -> bool {
                return a->slot() > b->slot();
         });
    auto it = std::unique(to_delete.begin(), to_delete.end(),
            [](list<Sequence>::iterator a,
               list<Sequence>::iterator b) {
                    return *a == *b;
            });
    to_delete.resize(it - to_delete.begin());

    for (auto ds = to_delete.begin(); ds != to_delete.end(); ++ds)
        _list.erase(*ds);
}

const SequenceContainer& SequenceContainer::reverse() {
    _dict = DictionaryFactory::get_instance()
        ->get_dictionary(_dict->get_f()->filename(),
                         _dict->get_e()->filename());

    for (Sequence& seq : _list)
        seq.reverse();

    return *this;
}

const SequenceContainer& SequenceContainer::merge(const
        SequenceContainer& that) {
    if (this->_dict != that._dict)
        throw runtime_error("Dictionaries don't match - aborting merge");

    list<Sequence>::iterator this_seq = this->_list.begin();

    for (iterator that_seq = that.begin();
         that_seq != that.end(); ++that_seq) {
        while (this_seq->slot() < that_seq->slot())
            ++this_seq;
        while(this_seq->slot() == that_seq->slot()) {
            if (*this_seq == *that_seq)
                break;
            ++this_seq;
        }
        if (*this_seq == *that_seq)
            continue;
        _list.insert( ++this_seq, *that_seq );
    }

    return *this;
}

