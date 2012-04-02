#ifndef PSEUDO_TREE_H
#define PSEUDO_TREE_H

#include <thrust/tuple.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

namespace tf
{
    template <typename V, class Op1, class Op2, class Inv, V id1, V id2>
    class pseudo_tree
    {
    public:
        typedef V value_t;
        typedef thrust::tuple<int, int, int, int, V, V, V> tuple;

    private:
        // This operation is NOT commutative, but is associative
        // Which is why we can use a parallel scan, but not a parallel reduce
        static tuple merge(const tuple& a, const tuple& b)
        {
            static Op1 op1;
            static Op2 op2;
            static Inv inv;

            // Let's give these tuple members nicer names
            // (that coincide with the paper)
            int L1 = thrust::get<0>(a); int L2 = thrust::get<0>(b);
            int M1 = thrust::get<1>(a); int M2 = thrust::get<1>(b);
            int B1 = thrust::get<2>(a); int B2 = thrust::get<2>(b);
            int E1 = thrust::get<3>(a); int E2 = thrust::get<3>(b);
            V R1 = thrust::get<4>(a);   V R2 = thrust::get<4>(b);
            V A1 = thrust::get<5>(a);   V A2 = thrust::get<5>(b);
            V F1 = thrust::get<6>(a);   V F2 = thrust::get<6>(b);

            int L = L1 + L2;
            int M = MIN(M1, L1 + M2);
            int B = B1? B1 : B2;
            int E = E2? E2 : E1;
            V R = op1(R1, R2);
            V A = (M == M1)? op1(A1, R2) : A2;
            V F_prime = (E1 == 1 && B2 == -1)? op2(F1, A1) : F1;
            V H = op1(op1(A1, R2), inv(A2));
            V F = (M == M1)? op2(F_prime, op1(H, F2)) :
                                   op2(op1(inv(H), F_prime), F2);

            return tuple(L, M, B, E, R, A, F);
        }

    public:
        // Create a pseudo_tree when the parenth is an empty string
        inline pseudo_tree()
        : t(0, 0, 0, 0, id1, id1, id2)
        {
        }

        // If an open parenth is 1,
        // (behaviour is undefined for other input):
        // ( -> ( 1,  0,  1,  1,      w,   w, id2)
        // ) -> (-1, -1, -1, -1, inv(w), id1, id2)
        // NOTE: inv isn't called here - the weight will already be inverted
        inline pseudo_tree(bool bit, V value)
        : t(-1 + 2*bit, -1 + bit, -1 + 2*bit, -1 + 2*bit, value, bit? value : id1, id2)
        {
        }

        // Convert a 7-tuple
        inline pseudo_tree(const tuple& _t)
        : t(_t)
        {
        }

        inline pseudo_tree(int L, int M, int B, int E, V R, V A, V F)
        : t(L, M, B, E, R, A, F)
        {
        }

        inline V get_result()
        {
            return thrust::get<6>(t);
        }

        inline pseudo_tree operator+(const pseudo_tree& other)
        {
            return pseudo_tree(merge(this->t, other.t));
        }

        const tuple t;
    };
}

#endif
