// Copyright 2012 Florian Petran
#include<list>
#include<string>
#include<vector>
#include<map>
#include<gtest/gtest.h> // NOLINT[build/include_order]
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
            Align::Params* params = Align::Params::get();
            params->set_dict_base(ALIGN_TEST_DICT);
            Align::DictionaryFactory* df =
                Align::DictionaryFactory::get_instance();
            _dict = df->get_dictionary(ALIGN_TEST_E, ALIGN_TEST_F);
            c = new Align::Candidates(*_dict);
            c->collect();
            sc = new Align::AlignMake(c);
        }
        virtual void TearDown() {
            delete c;
            delete sc;
        }

        // data members
        const Align::Dictionary* _dict;
        Align::Candidates* c;
        Align::AlignMake *sc;
};

class AlignTest : public AlignTest_Fixture {
};

TEST_F(AlignTest, CandidatesTest) {
    std::map<int, std::list<int>> tr_exp =
    { { 8,  {19, 30, 40} },
      { 9,  {20, 35, 39} },
      { 10, {21,  9, 13} },
      { 12, {22, 27, 28} },
      { 13, {23, 28, 35} },
      { 17, {19, 30, 40} } };
    std::map<int, std::list<int>> tr_actual;

    for (auto act = c->begin(); act != c->end(); ++act)
        for (auto act_tr = act->second->begin();
                act_tr != act->second->end(); ++act_tr)
            tr_actual[act->first.position()].push_back(act_tr->position());

    EXPECT_EQ(tr_exp.size(), tr_actual.size());
    EXPECT_EQ(tr_exp, tr_actual);
}

TEST_F(AlignTest, SequenceTestInital) {
    sc->initial_sequences();

    std::vector<std::vector<int>> expected_sequence = {
        { 8, 19,
          9, 20 },
        { 10, 21,
          12, 22 },
        { 12, 27,
          13, 28 }
    };

    // put actual indexes values into array too
    Align::Hypothesis *result = sc->get_result();
    Align::Hypothesis::iterator seq = result->begin();
    int exp = 0;
    while (seq != result->end()) {
        EXPECT_EQ(expected_sequence[exp], std::vector<int>(**seq));
        ++seq;
        ++exp;
    }
}

TEST_F(AlignTest, SequenceTestExpand) {
    sc->initial_sequences();
    sc->expand_sequences();

    std::vector<std::vector<int>> expected_sequence {
        { 8, 19,
          9, 20 },
        { 10, 21,
          12, 22,
          13, 23 },
        { 12, 27,
          13, 28,
          17, 30 }
    };
    std::vector<std::vector<int>> actual_sequence = *(sc->get_result());

    EXPECT_EQ(expected_sequence.size(), actual_sequence.size());
    EXPECT_EQ(expected_sequence, actual_sequence);
}

TEST_F(AlignTest, SequenceTestMerge) {
    sc->initial_sequences();
    sc->expand_sequences();
    sc->merge_sequences();

    std::vector<std::vector<int>> expected = {
        {  8, 19,
           9, 20,
          10, 21,
          12, 22,
          13, 23 },
        { 12, 27,
          13, 28,
          17, 30 }
    };
    std::vector<std::vector<int>> actual = *(sc->get_result());
    EXPECT_EQ(expected, actual);
}

TEST_F(AlignTest, TestAll) {
    sc->initial_sequences();
    sc->expand_sequences();
    sc->merge_sequences();
    sc->collect_scores();
    sc->get_topranking();

    std::vector<std::vector<int>> expected { {
        8, 19,
        9, 20,
        10, 21,
        12, 22,
        13, 23
    } };
    std::vector<std::vector<int>> actual = *(sc->get_result());
    EXPECT_EQ(expected.size(), actual.size());
    EXPECT_EQ(expected, actual);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
