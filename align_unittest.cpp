// Copyright 2012 Florian Petran
#include<list>
#include<string>
#include<vector>

#include<gtest/gtest.h>

#include"align.h"
#include"align_config.h"
#include"string_impl.h"

/*
 * customer test
 * test example alignment with two texts
 * each phase has its own test, with the expected results
 * hardcoded there.
 * TODO make better example texts - the one used right now are
 *      real anselm texts.
 */


class AlignTest_Fixture : public testing::Test {
    protected:
        virtual void SetUp() {
            Params* params = Params::get();
            params->set_dict_base(ALIGN_DICT_BASE);
            DictionaryFactory* df = DictionaryFactory::get_instance();

            std::string e =
                static_cast<std::string>(ALIGN_FILE_BASE);
            std::string f = e;

            e += "/test_e.txt";
            f += "/test_f.txt";

            _dict = df->get_dictionary(e, f);

            c = new Candidates(*_dict);
            c->collect();
            sc = new SequenceContainer(c);

        }
        virtual void TearDown() {
            delete c;
            delete sc;
        }

        // data members
        const Dictionary* _dict;
        Candidates* c;
        SequenceContainer* sc;

        void read_seq_to_vec(std::vector<int>* vec) {
            for (const Sequence& seq : *sc)
                for (const Pair& pair : seq) {
                    vec->push_back(pair.source().position());
                    vec->push_back(pair.target().position());
                }
        }
};

class AlignTest : public AlignTest_Fixture {
};

namespace {
    void printWord(const WordToken& w) {
        printString(w.get_str());
    }
}

TEST_F(AlignTest, CandidatesTest) {
    std::map<int,std::list<int>> tr_exp;
    tr_exp[8] = { 19, 29, 39 };
    tr_exp[9] = { 20, 34, 38 };
    tr_exp[10] = { 21, 9, 13 };
    tr_exp[12] = { 22, 26, 27 };
    tr_exp[13] = { 23, 27, 34 };

    std::map<int,std::list<int>> tr_actual;

    for (Candidates::iterator act = c->begin(); act != c->end(); ++act)
        for (std::list<WordToken>::iterator act_tr = act->second.begin();
                act_tr != act->second.end(); ++act_tr)
            tr_actual[act->first.position()].push_back(act_tr->position());

    EXPECT_EQ(tr_exp.size(), tr_actual.size());
    EXPECT_EQ(tr_exp, tr_actual);
}

TEST_F(AlignTest, SequenceTestInital) {
    sc->make(BreakAfterPhase::Initial);

    std::vector<int> expected_sequence {
        8, 19,
        9, 20,
        //---
        10, 21,
        12, 22,
        //---
        12, 26,
        13, 27
    };

    // put actual indexes values into array too
    std::vector<int> actual_sequence;
    read_seq_to_vec(&actual_sequence);

    EXPECT_EQ(expected_sequence.size(), actual_sequence.size());
    EXPECT_EQ(expected_sequence, actual_sequence);
}

TEST_F(AlignTest, SequenceTestExpand) {
    sc->make(BreakAfterPhase::Expand);

    std::vector<int> expected_sequence {
        8, 19,
        9, 20,
        //--
        10, 21,
        12, 22,
        13, 23,
        //--
        12, 26,
        13, 27
    };

    std::vector<int> actual_sequence;
    read_seq_to_vec(&actual_sequence);

    EXPECT_EQ(expected_sequence.size(), actual_sequence.size());
    EXPECT_EQ(expected_sequence, actual_sequence);
}

TEST_F(AlignTest, SequenceTestMerge) {
    sc->make(BreakAfterPhase::Merge);

    std::vector<int> expected_one {
        8, 19,
        9, 20,
        10, 21,
        12, 22,
        13, 23
    };

    std::vector<int> expected_two {
        12, 26,
        13, 27
    };


    // lots of hardcoded assumptions here. we should
    // have two sequences in sc with the content as
    // above
    SequenceContainer::iterator seq = sc->begin();
    std::vector<int> actual_one;

    for (const Pair& pair : *seq) {
        actual_one.push_back(pair.slot());
        actual_one.push_back(pair.target_slot());
    }
    ++seq;

    std::vector<int> actual_two;
    for (const Pair& pair : *seq) {
        actual_two.push_back(pair.slot());
        actual_two.push_back(pair.target_slot());
    }
    ++seq;

    EXPECT_EQ(seq, sc->end());

    EXPECT_EQ(expected_one.size(), actual_one.size());
    EXPECT_EQ(expected_one, actual_one);
    EXPECT_EQ(expected_two.size(), actual_two.size());
    EXPECT_EQ(expected_two, actual_two);
}

TEST_F(AlignTest, TestAll) {
    sc->make();

    std::vector<int> expected {
        8, 19,
        9, 20,
        10, 21,
        12, 22,
        13, 23
    };
    std::vector<int> actual;
    read_seq_to_vec(&actual);
    EXPECT_EQ(expected.size(), actual.size());
    EXPECT_EQ(expected, actual);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
