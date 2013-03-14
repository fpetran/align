// 2013 Florian Petran
#include"bi-sim.h"
#include<gtest/gtest.h>

class Bisim_Test : public testing::Test {
};

TEST_F(Bisim_Test, CorrectValues) {
    // test values from Kondrak/Dorr 2004: 5
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

