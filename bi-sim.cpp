#include"bi-sim.h"
#include<unicode/uchar.h>


using namespace std;

namespace bi_sim {
    typedef unsigned int uint;
    typedef vector<num_ty> mat_ty;

    inline num_ty id( uint x, uint y ) {
        if( x == y )
            return 1;
        return 0;
    }
    num_ty bi_sim( const String& w1, const String& w2 ) {
        uint m = w1.length(); uint n = w2.length();
        String x = w1; String y = w2;
        x.toLower(); y.toLower();

        x = u_toupper( x[0] ) + x;
        y = u_toupper( y[0] ) + y;

        /*
        /////////////////////
        mat_ty f(n+1);
        for( mat_ty::iterator el = f.begin(); el != f.end(); ++el )
            *el = 0.0;

        num_ty a = 0.0, b, c;
        for( uint i = 1; i <= m; ++i )
            for( uint j = 1; j <= n; ++j )
                f[j] = max(
                        max( b, c ) );
        //////////////////////
        */




        // initialize and fill matrix
        vector<vector<num_ty> > f;
        f.resize( m+1 );
        for( uint i = 0; i <= m; ++i ) {
            f[i].resize( n+1 );
            f[i][0] = 0.0;
        }
        for( uint i = 0; i <= n; ++i )
            f[0][i] = 0.0;

        // calculate bi_sim
        for( uint i = 1; i <= m; ++i )
            for( uint j = 1; j <= n; ++j )
                f[i][j] = max(
                            max( f[i-1][j], f[i][j-1] ),
                            f[i-1][j-1] + id( x[i-1], y[j-1] ) + id( x[i], y[j] ) );

        return f[m][n] / ( 2 * max( m, n ) );

    }
}
