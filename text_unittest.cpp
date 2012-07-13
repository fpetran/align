// Copyright 2012 Florian Petran
#include<fstream>
#include<list>
#include<string>

#include<gtest/gtest.h>

#include"align.h"
#include"align_config.h"

// fixture for all texts
class WordTest_Fixture : public testing::Test {
    protected:
        virtual void SetUp() {
            params = Params::get();
            df = DictionaryFactory::get_instance();

            params->set_dict_base(ALIGN_DICT_BASE);
            _dict = df->get_dictionary(ALIGN_TEST_E, ALIGN_TEST_F);

            _e = _dict->get_e();
            e_first = get_first_line(ALIGN_TEST_E);
            pegraben1_e = find_token_for("pegraben", _e);
            pegraben2_e = find_token_for("pegraben", _e,
                                         pegraben1_e->position() + 1);

            _f = _dict->get_f();
            f_first = get_first_line(ALIGN_TEST_F);
            begraben1_f = find_token_for("begraben", _f);
            begraben2_f = find_token_for("begraben", _f,
                                         begraben1_f->position() + 1);
        }
        virtual void TearDown() {
            // called after each test
            //delete _dict;
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

        const WordToken* find_token_for(const string_impl& what, const Text* txt, int startpos = 0) {
            int ii = startpos;
            while (ii < txt->length() && txt->at(ii).get_str() != what)
                ++ii;

            return &txt->at(ii);
        }

        // Objects declared here can be used by all tests
        Params* params;
        DictionaryFactory* df;

        string_impl f_first, e_first;
        //< first lines from e and f, read conventionally

        const WordToken *pegraben1_e, *pegraben2_e, *begraben1_f, *begraben2_f;

        const Dictionary* _dict;
        const Text *_e, *_f;
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
    EXPECT_TRUE(pegraben1_e != pegraben2_e);
    EXPECT_TRUE(pegraben1_e->get_type() == pegraben2_e->get_type());
    EXPECT_TRUE(&pegraben1_e->get_type() == &pegraben2_e->get_type());
    // begraben
    EXPECT_TRUE(begraben1_f != begraben2_f);
    EXPECT_TRUE(begraben1_f->get_type() == begraben2_f->get_type());
    EXPECT_TRUE(&begraben1_f->get_type() == &begraben2_f->get_type());
}


TEST_F(WordTest, ReadCorrect) {
    // test if the first word from texts were read correctly from the file
    EXPECT_TRUE(f_first == (*_f).at(0).get_str());
    EXPECT_TRUE(e_first == (*_e).at(0).get_str());
}

TEST_F(WordTest, TextPtrE) {
    // test if all words point to the same text -- e
    for (int i = 1; i < _e->length(); ++i)
        EXPECT_TRUE(&(*_e)[i-1].get_text() == &(*_e)[i].get_text());
}

TEST_F(WordTest, TextPtrF) {
    // test if all words point to the same text -- f
    for (int i = 1; i < _f->length(); ++i)
        EXPECT_TRUE(&(*_f)[i-1].get_text() == &(*_f)[i].get_text());
}

TEST_F(WordTest, LookupTest) {
    // test if the lookup works correctly
    // test translation pair: pegraben - begraben
    // pegraben - 21(e) : 5459 5571
    // begraben - 65(f) : 8479 8559

    std::list<WordType> trans1 = _dict->lookup(*pegraben1_e);
    std::list<WordType> trans2 = _dict->lookup(*pegraben1_e);
    int num_trans_1 = 0;
    for (const WordType& tr_type : trans1)
        num_trans_1 += tr_type.get_tokens().size();
    int num_trans_2 = 0;
    for (const WordType& tr_type : trans2)
        num_trans_2 += tr_type.get_tokens().size();

    EXPECT_EQ(2, num_trans_1);
    EXPECT_EQ(2, num_trans_2);

    EXPECT_EQ(trans1, trans2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
