// Copyright 2012 Florian Petran
#include<iostream>
#include<fstream>
#include<list>
#include<string>

#include<gtest/gtest.h>

#include"align.h"
#include"params.h"
#include"string_impl.h"

// fixture for all texts
class WordTest_Fixture : public testing::Test {
    protected:
        virtual void SetUp() {
            Params::get()->set_dict_base( ALIGN_DICT_BASE );
            _dict.open(ALIGN_TEST_E, ALIGN_TEST_F);
            _f = _dict.get_f();
            _e = _dict.get_e();
            // called before each test
        }
        virtual void TearDown() {
            // called after each test
        }

        // Objects declared here can be used by all tests
        Dictionary _dict;
        Text *_e, *_f;
};


class WordTest : public WordTest_Fixture {
};


TEST_F(WordTest, Comparison) {
    // test comparison operator
    EXPECT_TRUE((*_e)[0] == (*_e)[0]);
    EXPECT_FALSE((*_e)[0] == (*_f)[0]);
    // pegraben
    EXPECT_TRUE((*_e)[5458] == (*_e)[5570]);
    // begraben
    EXPECT_TRUE((*_f)[8478] == (*_f)[8558]);
}

/// get first line from a file
string_impl get_first_line(const char* filename) {
    char c_line[256];

    std::ifstream f;
    f.open(filename);
    f.getline(c_line, 256);
    f.clear();
    f.close();

    string_impl first = c_line;
    lower_case(&first);
    return first;
}

TEST_F(WordTest, ReadCorrect) {
    // test if the first word from texts were read correctly from the file
    string_impl f_first = get_first_line(ALIGN_TEST_F);
    string_impl e_first = get_first_line(ALIGN_TEST_E);

    EXPECT_TRUE( f_first == (*_f)[0].get_str() );
    EXPECT_TRUE( e_first == (*_e)[0].get_str() );
}

TEST_F(WordTest, TextPtrE) {
    // test if all words point to the same text -- e
    for (int i = 1; i < _e->length(); ++i)
        EXPECT_TRUE( (*_e)[i-1].get_text() == (*_e)[i].get_text() );
}

TEST_F(WordTest, TextPtrF) {
    // test if all words point to the same text -- f
    for (int i = 1; i < _f->length(); ++i)
        EXPECT_TRUE( (*_f)[i-1].get_text() == (*_f)[i].get_text() );
}

TEST_F(WordTest, LookupTest) {
    // test if the lookup works correctly
    // test translation pair: pegraben - begraben
    // pegraben - 21(e) : 5459 5571
    // begraben - 65(f) : 8479 8559

    TranslationsEntry* translations = _dict.lookup((*_e)[5458]);
    EXPECT_EQ(translations->size(), 2);

    TranslationsEntry::iterator tr = translations->begin();
    EXPECT_EQ(*tr, 8478);
    ++tr;
    EXPECT_EQ(*tr, 8558);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
