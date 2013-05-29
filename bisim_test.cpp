// Copyright 2013 Florian Petran
//
// The test words and values are taken from the bi_sim paper:
//
// Kondrak/Dorr (2006): Automatic identification of confusable
// drug names, p.5
#include"bi-sim.h"
#include<gtest/gtest.h> // NOLINT[build/include_order]

class Bisim_Test : public testing::Test {
};

TEST_F(Bisim_Test, CorrectValues) {
    EXPECT_DOUBLE_EQ(bi_sim::num_ty(0.6875),
            bi_sim::bi_sim("Toradol", "Tramadol"));
    EXPECT_DOUBLE_EQ(bi_sim::num_ty(0.625),
            bi_sim::bi_sim("Toradol", "Tobradex"));
    EXPECT_DOUBLE_EQ(bi_sim::num_ty(0.5714285714285714) ,
            bi_sim::bi_sim("Toradol", "Torecan"));
    EXPECT_DOUBLE_EQ(bi_sim::num_ty(0.5714285714285714) ,
            bi_sim::bi_sim("Toradol", "Stadol"));
    EXPECT_DOUBLE_EQ(bi_sim::num_ty(0.5),
            bi_sim::bi_sim("Toradol", "Torsemide"));
    EXPECT_DOUBLE_EQ(bi_sim::num_ty(0.5),
            bi_sim::bi_sim("Toradol", "Theraflu"));
    EXPECT_DOUBLE_EQ(bi_sim::num_ty(0.5),
            bi_sim::bi_sim("Toradol", "Tegretol"));
    EXPECT_DOUBLE_EQ(bi_sim::num_ty(0.5),
            bi_sim::bi_sim("Toradol", "Taxol"));
}

