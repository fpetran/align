// Copyright 2012 Florian Petran
#include<list>
#include<string>

#include<gtest/gtest.h>

#include"align.h"
#include"params.h"
#include"string_impl.h"

class AlignTest_Fixture : public testing::Test {
    protected:
        virtual void SetUp() {
            Params::get()->set_dict_base( ALIGN_DICT_BASE );
            std::string e =
                static_cast<std::string>(ALIGN_FILE_BASE);
            std::string f = e;

            e += "/test_e.txt";
            f += "/test_f.txt";

            _dict.open( e.c_str(), f.c_str() );
            //_f = _dict.get_f();
            //_e = _dict.get_e();


            // fill expected translations collection
            fill_entry( &tr_exp, 8, 19, 29, 39 );
            fill_entry( &tr_exp, 9, 20, 34, 38 );
            fill_entry( &tr_exp, 10, 21, 9, 13 );
            fill_entry( &tr_exp, 12, 22, 26, 27 );
            fill_entry( &tr_exp, 13, 23, 27, 34 );
        }
        virtual void TearDown() {
        }

        Dictionary _dict;
        Translations tr_exp;
        void fill_entry( Translations* tr,
                int src,
                int one, int two, int three ) {
            TranslationsEntry ex_entry;
            ex_entry.push_back( one );
            ex_entry.push_back( two );
            ex_entry.push_back( three );
            tr->insert( make_pair( src, new TranslationsEntry(ex_entry) ) );
        }
};

class AlignTest : public AlignTest_Fixture {
};

namespace {
    void printWord(const Word& w) {
        printString(w.get_str());
    }
}

TEST_F(AlignTest, CandidatesTest) {
    Candidates c(_dict);
    c.collect();

    Translations::const_iterator act = c.begin(), exp = tr_exp.begin();

    while( act != c.end() && exp != tr_exp.end() ) {
        EXPECT_EQ( act->first, exp->first );
        EXPECT_EQ( *(act->second), *(exp->second) );
        ++act; ++exp;
    }

}

TEST_F(AlignTest, SequenceTestInital) {
    Candidates c(_dict);
    c.collect();

    SequenceContainer sc(c);
    sc.make(BreakAfterInitial);

    // it's possibly a huge hack to read all values into
    // an array, but otoh i don't want to implement a getter
    // for internal data structures just for the unit test
    // expected values
    int expected_sequence[12] =
    {
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
    int actual_sequence[12];
    int i = 0;
    for (SequenceContainer::iterator seq = sc.begin();
            seq != sc.end(); ++seq)
        for (Sequence::iterator pair = seq->begin();
                pair != seq->end(); ++pair) {
            EXPECT_TRUE( i < 12 );
            actual_sequence[i] = pair->slot();
            actual_sequence[i+1] = pair->target_slot();
            i += 2;
        }

    for (int ii = 0; ii < 12; ++ii)
        EXPECT_EQ( expected_sequence[ii], actual_sequence[ii] );
}

TEST_F(AlignTest, SequenceTestExpand) {
    Candidates c(_dict);
    c.collect();

    SequenceContainer sc(c);
    sc.make(BreakAfterExpand);

    int expected_sequence[14] = {
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

    int actual_sequence[14];
    int i = 0;

    for (SequenceContainer::iterator seq = sc.begin();
            seq != sc.end(); ++seq)
        for (Sequence::iterator pair = seq->begin();
                pair != seq->end(); ++pair) {
            EXPECT_TRUE( i < 14 );
            actual_sequence[i] = pair->slot();
            actual_sequence[i+1] = pair->target_slot();
            i += 2;
        }

    for (int ii = 0; ii < 13; ++ii)
        EXPECT_EQ( expected_sequence[ii], actual_sequence[ii] );

}

TEST_F(AlignTest, SequenceTestMerge) {
    Candidates c(_dict);
    c.collect();

    SequenceContainer sc(c);
    sc.make(BreakAfterMerge);

    int expected_one[10] = {
        8, 19,
        9, 20,
        10, 21,
        12, 22,
        13, 23
    };

    int expected_two[4] = {
        12, 26,
        13, 27
    };


    // lots of hardcoded assumptions here. we should
    // have two sequences in sc with the content as
    // above
    SequenceContainer::iterator seq = sc.begin();
    int actual_one[10];
    int i = 0;

    for (Sequence::iterator pair = seq->begin();
            pair != seq->end(); ++pair) {
        EXPECT_TRUE( i < 10 );
        actual_one[i] = pair->slot();
        actual_one[i+1] = pair->target_slot();
        i += 2;
    }
    ++seq;

    int actual_two[4];
    i = 0;
    for (Sequence::iterator pair = seq->begin();
            pair != seq->end(); ++pair) {
        EXPECT_TRUE( i < 4 );
        actual_two[i] = pair->slot();
        actual_two[i+1] = pair->target_slot();
        i += 2;
    }
    ++seq;

    EXPECT_TRUE( seq == sc.end() );

    for (int ii = 0; ii < 9; ++ii)
        EXPECT_EQ( expected_one[ii], actual_one[ii] );
    for (int ii = 0; ii < 3; ++ii)
        EXPECT_EQ( expected_two[ii], actual_two[ii] );

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
