// Copyright 2012 Florian Petran
#include<fstream>
#include<list>
#include<string>
#include<gtest/gtest.h> // NOLINT[build/include_order]
#include"align_config.h"
#include"text.h"
#include"dictionary.h"

// fixture for all texts
class WordTest_Fixture : public testing::Test {
    protected:
        virtual void SetUp() {
            params->set_dict_base(ALIGN_TEST_DICT);
            _dict = df->get_dictionary(ALIGN_TEST_E, ALIGN_TEST_F);

            _e = _dict->get_e();
            e_first = get_first_line(ALIGN_TEST_E);

            _f = _dict->get_f();
            f_first = get_first_line(ALIGN_TEST_F);
        }
        virtual void TearDown() {
            // called after each test
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

        // Objects declared here can be used by all tests
        Align::Params* params = Align::Params::get();
        Align::DictionaryFactory* df = Align::DictionaryFactory::get_instance();

        /// first lines from e and f, read conventionally
        string_impl f_first, e_first;
        const Align::Dictionary* _dict;
        const Align::Text *_e, *_f;
};


class WordTest : public WordTest_Fixture {
};

TEST_F(WordTest, Comparison) {
    // test comparison operator
    // self equality
    EXPECT_TRUE((*_e).at(0) == (*_e).at(0));
    // text inequality
    EXPECT_FALSE((*_e).at(0) == (*_f).at(0));

    // tokens at different positions should be inequal, but
    // their types should be equal if the string representations match
    // plus, they should point to the same type anyway.
    // pegraben
    Align::WordToken tok_f1 = _f->at(17),
                     tok_f2 = _f->at(25);
    EXPECT_TRUE(tok_f1 != tok_f2);
    EXPECT_TRUE(tok_f1.get_type() == tok_f2.get_type());
    EXPECT_TRUE(&tok_f1.get_type() == &tok_f2.get_type());

    Align::WordToken tok_e1 = _e->at(0),
                     tok_e2 = _e->at(16);
    EXPECT_TRUE(tok_e1 != tok_e2);
    EXPECT_TRUE(tok_e1.get_type() == tok_e2.get_type());
    EXPECT_TRUE(&tok_e1.get_type() == &tok_e2.get_type());
}


TEST_F(WordTest, ReadCorrect) {
    // test if the first word from texts were read correctly from the file
    EXPECT_TRUE(f_first == (*_f).at(0).get_str());
    EXPECT_TRUE(e_first == (*_e).at(0).get_str());
}

TEST_F(WordTest, TextPtr) {
    // test if all words point to the same text -- e
    for (int i = 1; i < _e->length(); ++i)
        EXPECT_TRUE(&(*_e)[i-1].get_text() == &(*_e)[i].get_text());
    for (int i = 1; i < _f->length(); ++i)
        EXPECT_TRUE(&(*_f)[i-1].get_text() == &(*_f)[i].get_text());
}

TEST_F(WordTest, LookupTest) {
    // test if the lookup works correctly
    // test two positions with the same token: aa
    // aa has 3 occurrences of translations in f
    // for each token, the translations returned should be the same
    // other tokens with 3 translations:
    // 8 (aa), 9 (ab), 12 (ae), 13 (af), 17 (aa)
    Align::WordToken tok_e1 = _e->at(8),
                     tok_e2 = _e->at(17);

    const std::list<Align::WordType> &trans1 = _dict->lookup(tok_e1),
                                     &trans2 = _dict->lookup(tok_e2);
    int num_trans_1 = 0;
    for (const Align::WordType& tr_type : trans1)
        num_trans_1 += tr_type.get_tokens().size();
    int num_trans_2 = 0;
    for (const Align::WordType& tr_type : trans2)
        num_trans_2 += tr_type.get_tokens().size();

    EXPECT_EQ(3, num_trans_1);
    EXPECT_EQ(3, num_trans_2);
    EXPECT_EQ(trans1, trans2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
