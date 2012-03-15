#include <stdio.h>
#include <limits.h>
#include <cassert>
//#include <thrust/transform.h>
#include <thrust/functional.h>
#include "PseudoTree.h"

// Maps ( to 1 and ) to 0
// Asserts that ( or ) are the input.
// Input checking (if you want nice errors) is client responsibility.
inline int map_parenth(char p)
{
    assert('(' == p || ')' == p);
    return (p - '(') ^ 1;
}

template <class PT>
void print_pt(PT pt)
{
    printf("( %d, %d, %d, %d, %d, %d, %d )\n", pt.L, pt.M, pt.B, pt.E, pt.R, pt.A, pt.F);
}

int main(void)
{
    typedef int node_value_t;
    typedef thrust::plus<node_value_t> op1;
    typedef thrust::maximum<node_value_t> op2;
    typedef thrust::negate<node_value_t> inv1;
    const int id1 = 0;
    const int id2 = INT_MIN;

    op1 plus;
    op2 mx;
    inv1 neg;
    typedef tree_functions::PseudoTree<node_value_t, op1, op2, neg, id1, id2> ptree;

    // TODO: Use a transformation mapper with a reduce? reduce is associative
    // if it isnt associative, use a scan
    // this way sequentialness is handled by thrust...
    // then make a mapping function that does the sequential creation of a bitmap block
    printf("%d\n", plus(2, 3));
    //ptree a(0,5);
    //ptree b(1, 7);
    //print_pt<ptree>(a);
    //print_pt<ptree>(b);
    //print_pt<ptree>(a + b);
    return 0;
}
