#include <limits.h>
#include "pseudo_tree.h"
#include "gtest/gtest.h"
#include <thrust/functional.h>

typedef int value_t;
typedef thrust::plus<value_t> plus;
typedef thrust::maximum<value_t> maximum;
typedef thrust::negate<value_t> negate;
const int plus_id = 0;
const int maximum_id = INT_MIN;
typedef tf::pseudo_tree<value_t, plus, maximum, negate, plus_id, maximum_id> pt;

TEST(pseudo_tree, init)
{
    pt a;
    pt b(1, 5);
    pt c(0, 70);
    // These tuples are straight from the paper...
    // it is only implemented as a test to make sure the values aren't mixed up if
    // the source is modified.
    EXPECT_EQ(a.t, pt::tuple(0,   0,  0,  0, plus_id, plus_id, maximum_id));
    EXPECT_EQ(b.t, pt::tuple(1,   0,  1,  1,       5,       5, maximum_id));
    EXPECT_EQ(c.t, pt::tuple(-1, -1, -1, -1,      70, plus_id, maximum_id));
}

TEST(pseudo_tree, get_result)
{
    pt a;
    EXPECT_EQ(a.get_result(), maximum_id);
}

// The following examples are taken directly from the paper
TEST(pseudo_tree, merge)
{
    pt a(1, 2); pt e(1, 6); pt i(0,-9); pt m(0, 1); pt q(1, 6); pt u(1, 7);
    pt b(1, 3); pt f(1, 4); pt j(0,-6); pt n(1, 1); pt r(0,-6); pt v(0,-7);
    pt c(0,-3); pt g(0,-4); pt k(1, 5); pt o(1,-2); pt s(0,-1); pt w(0,-5);
    pt d(1,-1); pt h(1, 9); pt l(0,-5); pt p(0, 2); pt t(1, 5); pt x(0,-2);

    pt abcd( 2, 0, 1, 1,  1, 1, 5);
    pt efgh( 2, 0, 1, 1, 15,15,10);
    pt ijkl(-2,-2,-1,-1,-15, 0, 5);
    pt mnop( 0,-1,-1,-1,  2, 1,-1);
    pt qrst( 0,-1, 1, 1,  4, 5, 7);
    pt uvwx(-2,-2, 1,-1, -7, 0,14);

    EXPECT_EQ((a + b + c + d).t, abcd.t);
    EXPECT_EQ((e + f + g + h).t, efgh.t);
    EXPECT_EQ((i + j + k + l).t, ijkl.t);
    EXPECT_EQ((m + n + o + p).t, mnop.t);
    EXPECT_EQ((q + r + s + t).t, qrst.t);
    EXPECT_EQ((u + v + w + x).t, uvwx.t);

    pt abcdefgh( 4, 0, 1, 1, 16,16,11);
    pt ijklmnop(-2,-3,-1,-1,-13, 1, 4);
    pt qrstuvwx(-2,-2, 1,-1,-3, 0,14);

    EXPECT_EQ((abcd + efgh).t, abcdefgh.t);
    EXPECT_EQ((ijkl + mnop).t, ijklmnop.t);
    EXPECT_EQ((qrst + uvwx).t, qrstuvwx.t);

    EXPECT_EQ((abcdefgh + ijklmnop).t, pt::tuple(2,0,1,-1,3,3,16));
    EXPECT_EQ(((abcdefgh + ijklmnop) + qrstuvwx).t, pt::tuple(0, 0, 1, -1, 0, 0, 16));
}
