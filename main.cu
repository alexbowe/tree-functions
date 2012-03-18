#include <stdio.h>
#include <iostream>
#include <limits>
#include <cassert>
//#include <thrust/functional.h>
//#include "PseudoTree.h"
#include <string>
#include <thrust/iterator/transform_iterator.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/transform_scan.h>
#include <thrust/tuple.h>
#include <thrust/device_vector.h>

#include "debug.h"

struct map_parenth : public thrust::unary_function<char,int>
{
    // Maps ( to 1 and ) to 0
    __device__ __host__
    int operator()(char p) const
    {
        return (p - '(') ^ 1;
    }
};

//struct Tree
// Just has two device vectors, maybe more when I support bitmaps...
// provide access using a zip iterator
typedef int block_t;
static const unsigned int block_bytes = sizeof(block_t);
// How many bits? Add 1 if it is signed (digits returns magnitude-bits only).
static const unsigned int block_bits  = std::numeric_limits<block_t>::digits
    + (std::numeric_limits<block_t>::is_signed? 1 : 0);

typedef int value_t;
typedef thrust::tuple<int, int, int, int, value_t, value_t, value_t> ptree;
typedef thrust::plus<value_t> plus;
typedef thrust::maximum<value_t> maximum;
typedef thrust::negate<value_t> negate;
const int plus_id = 0;
const int maximum_id = INT_MIN;

template <value_t id1, value_t id2>
struct make_initial_ptree : public thrust::unary_function<thrust::tuple<block_t,value_t>,ptree>
{
    // ( -> (1,   0,  1,  1, w,   w, id2)
    // ) -> (-1, -1, -1, -1, w, id1, id2)
    template <typename Tuple>
    __host__ __device__
    ptree operator()(Tuple t)
    {
        int p = thrust::get<0>(t);
        int w = thrust::get<1>(t);
        return p? ptree(1, 0, 1, 1, w, w, id2) : ptree(-1, -1, -1, -1, w, id1, id2);
    }
};

template <typename Op1, typename Op2, typename Inv>
struct merge_ptrees : public thrust::binary_function<ptree, ptree, ptree>
{
    __host__ __device__
    ptree operator()(const ptree& a, const ptree& b)
    {
        // Let's give these tuple members nicer names (that coincide with the paper)
        int L1 = thrust::get<0>(a); int L2 = thrust::get<0>(b);
        int M1 = thrust::get<1>(a); int M2 = thrust::get<1>(b);
        int B1 = thrust::get<2>(a); int B2 = thrust::get<2>(b);
        int E1 = thrust::get<3>(a); int E2 = thrust::get<3>(b);
        value_t R1 = thrust::get<4>(a); value_t R2 = thrust::get<4>(b);
        value_t A1 = thrust::get<5>(a); value_t A2 = thrust::get<5>(b);
        value_t F1 = thrust::get<6>(a); value_t F2 = thrust::get<6>(b);

        int L = L1 + L2;
        int M = min(M1, L1 + M2);
        int B = B1? B1 : B2;
        int E = E2? E2 : E1;
        value_t R = op1(R1, R2);
        value_t A = (M == M1)? op1(A1, R2) : A2;
        value_t F_prime = (E1 == 1 && B2 == -1)? op2(F1, A1) : F1;
        value_t H = op1(op1(A1, R2), inv(A2));
        value_t F = (M == M1)? op2(F_prime, op1(H, F2)) : op2(op1(inv(H), F_prime), F2);
        
        return ptree(L, M, B, E, R, A, F);
    }
     
    static Op1 op1;
    static Op2 op2;
    static Inv inv;
};


template <typename Op1, typename Op2, typename Inv>
Op1 merge_ptrees<Op1, Op2, Inv>::op1 = Op1();

template <typename Op1, typename Op2, typename Inv>
Op2 merge_ptrees<Op1, Op2, Inv>::op2 = Op2();

template <typename Op1, typename Op2, typename Inv>
Inv merge_ptrees<Op1, Op2, Inv>::inv = Inv();

int main(void)
{
    std::string s = "(()((()())())(()())(()))";
    thrust::host_vector<block_t> temp(s.begin(), s.end());
    // Later, allocate blocks of bits
    // unsigned int num_blocks = (s.length() + block_bits - 1) / block_bits;
    thrust::device_vector<block_t> p(thrust::make_transform_iterator(temp.begin(), map_parenth()),
                                     thrust::make_transform_iterator(temp.end(),   map_parenth()));

    // Write weight matching function that accepts iterators for chars, iterators for weights
    // input parenths, input level-order weights, output weight array
    value_t weights[] = {2, 3, -3, -1, 6, 4, -4, 9, -9, -6, 5, -5, 1, 1, -2, 2, 6, -6, -1, 4, 7, -7, -5, -2};
    thrust::device_vector<value_t> v(weights, weights + s.length());
    
    TRACE_V((p));
    TRACE_V((v));

    // allocate space for num_blocks pseudotrees
    thrust::device_vector<int>     L_v(s.size());
    thrust::device_vector<int>     M_v(s.size());
    thrust::device_vector<int>     B_v(s.size());
    thrust::device_vector<int>     E_v(s.size());
    thrust::device_vector<value_t> R_v(s.size());
    thrust::device_vector<value_t> A_v(s.size());
    thrust::device_vector<value_t> F_v(s.size());

    //thrust::make_transform_iterator(
    thrust::transform_inclusive_scan(
        thrust::make_zip_iterator(thrust::make_tuple(p.begin(), v.begin())),
        thrust::make_zip_iterator(thrust::make_tuple(p.end(), v.end())),
        thrust::make_zip_iterator(thrust::make_tuple(L_v.begin(), M_v.begin(), B_v.begin(), E_v.begin(), R_v.begin(),
            A_v.begin(), F_v.begin())),
        make_initial_ptree<plus_id, maximum_id>(),
        merge_ptrees<plus, maximum, negate>()
    );

    TRACE_V((L_v));
    TRACE_V((M_v));
    TRACE_V((B_v));
    TRACE_V((E_v));
    TRACE_V((R_v));
    TRACE_V((A_v));
    TRACE_V((F_v));

    return 0;
}
