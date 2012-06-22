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
    for (std::list<Sequence>::const_iterator sl = sc.begin();
            sl != sc.end(); ++sl)
        for (std::list<Pair>::const_iterator sq = sl->begin();
                sq != sl->end(); ++sq) {
            EXPECT_TRUE( i < 12 );
            actual_sequence[i] = sq->slot();
            actual_sequence[i+1] = sq->target_slot();
            i += 2;
        }

    for (int i = 0; i < 12; ++i)
        EXPECT_EQ( expected_sequence[i], actual_sequence[i] );
}

TEST_F(AlignTest, SequenceTestExpand) {
    Candidates c(_dict);
    c.collect();

    SequenceContainer sc(c);
    sc.make(BreakAfterExpand);
    for (std::list<Sequence>::const_iterator sl = sc.begin();
            sl != sc.end(); ++sl) {
        std::cout << "SEQUENCE: ---" << std::endl;
        for (std::list<Pair>::const_iterator sq = sl->begin();
                sq != sl->end(); ++sq) {
            std::cout << "source index: " << sq->slot() << " txt: ";
            printWord( sq->source() );
            std::cout << "target index: " << sq->target_slot() << " txt: ";
            printWord( sq->target() );
        }
        std::cout << "-------------" << std::endl;
    }
}

TEST_F(AlignTest, SequenceTestMerge) {
    Candidates c(_dict);
    c.collect();

    SequenceContainer sc(c);
    sc.make(BreakAfterMerge);

    for (std::list<Sequence>::const_iterator sl = sc.begin();
            sl != sc.end(); ++sl) {
        std::cout << "SEQUENCE: ---" << std::endl;
        for (std::list<Pair>::const_iterator sq = sl->begin();
                sq != sl->end(); ++sq) {
            std::cout << "source index: " << sq->slot() << " txt: ";
            printWord( sq->source() );
            std::cout << "target index: " << sq->target_slot() << " txt: ";
            printWord( sq->target() );
        }
        std::cout << "-------------" << std::endl;
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
