// Copyright 2012 Florian Petran
#include"bi-sim.h"
#include<vector>
#include<algorithm>

using std::vector;
using std::max;

namespace bi_sim {
    typedef unsigned int uint;
    typedef vector<num_ty> mat_ty;

    inline num_ty id(uint x, uint y) {
        return x == y ? 1 : 0;
    }

    num_ty bi_sim(const string_impl& w1, const string_impl& w2) {
        uint m = w1.length(),
             n = w2.length();
        string_impl x = w1,
                    y = w2;
        lower_case(&x);
        lower_case(&y);
        char_impl x1 = x[0],
                  y1 = y[0];
        upper_case(&x1);
        upper_case(&y1);

        x = x1 + x;
        y = y1 + y;

        // initialize and fill matrix
        vector<vector<num_ty>> f;
        f.resize(m+1);
        for (uint i = 0; i <= m; ++i) {
            f[i].resize(n+1);
            f[i][0] = 0.0;
        }
        for (uint i = 0; i <= n; ++i)
            f[0][i] = 0.0;

        // calculate bi_sim
        for (uint i = 1; i <= m; ++i)
            for (uint j = 1; j <= n; ++j)
                f[i][j] = max(max(f[i-1][j], f[i][j-1]),
                              f[i-1][j-1]
                            + id(x[i-1], y[j-1])
                            + id(x[i], y[j]));

        return f[m][n] / (2 * max(m, n));
    }
}  // namespace bi_sim

