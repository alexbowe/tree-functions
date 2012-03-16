#pragma once
#ifndef PSEUDO_TREE_H
#define PSEUDO_TREE_H

namespace tree_functions
{

// TODO: look at alignment, maybe use cuda small vector types
template<typename T, typename AssociativeOperator1, typename AssociativeOperator2, typename UnaryOperator, T ID1, T ID2>
class PseudoTree
{
public:
    // Default Constructor - An identity pseudotree for this operation
    #ifdef __CUDACC__
    __device__ __host__
    #endif
    PseudoTree() : L(0), M(0), B(0), E(0), R(ID1), A(ID1), F(ID2) { ; }

    // ESSENTIAL that parenth doesnt take a value other than 0 or 1
    __device__ __host__
    PseudoTree(bool parenth, T weight) :
    	// ( -> (1,   0,  1,  1, w,   w, id2)
        // ) -> (-1, -1, -1, -1, w, id1, id2)
        L(-1 + 2*parenth),
        M(-1 + parenth),
        B(-1 + 2*parenth),
        E(-1 + 2*parenth),
        R(weight),
        A(parenth? weight : ID1),
        F(ID2)
        { ; }
    
    // TODO: templated constructor with block type parameter that adds them sequentially?
    #ifdef __CUDACC__
    __device__ __host__
    #endif
    static
    PseudoTree merge(const PseudoTree& a, const PseudoTree& b)
    {
        int L = a.L + b.L;
        int M = min(a.M, a.L + b.M);
        int B = a.B? a.B : b.B;
        int E = b.E? b.E : a.E;
        T   R = op1(a.R, b.R);
        T   A = (M == a.M)? op1(a.A, b.R) : b.A;
        T   F_prime = (a.E == 1 && b.B == -1)? op2(a.F, a.A) : a.F;
        T   H = op1(op1(a.A, b.R), inv(b.A));
        T   F = (M == a.M)? op2(F_prime, op1(H, b.F)) : op2(op1(inv(H), F_prime), b.F);
        return PseudoTree(L, M, B, E, R, A, F);
    }

//protected:
    static AssociativeOperator1 op1;
    static AssociativeOperator2 op2;
    static UnaryOperator inv;

    // could use cuda int4 here?
    int L, M, B, E;
    T R, A, F;

    __device__ __host__
    PseudoTree(int l, int m, int b, int e, T r, T a, T f) : L(l), M(m), B(b), E(e), R(r), A(a), F(f) { ; }
};

// Initialise templated operators
template<typename T, typename AssociativeOperator1, typename AssociativeOperator2, typename UnaryOperator, T ID1, T ID2>
AssociativeOperator1 PseudoTree<T, AssociativeOperator1, AssociativeOperator2, UnaryOperator, ID1, ID2>::op1 = AssociativeOperator1();

template<typename T, typename AssociativeOperator1, typename AssociativeOperator2, typename UnaryOperator, T ID1, T ID2>
AssociativeOperator2 PseudoTree<T, AssociativeOperator1, AssociativeOperator2, UnaryOperator, ID1, ID2>::op2 = AssociativeOperator2();

template<typename T, typename AssociativeOperator1, typename AssociativeOperator2, typename UnaryOperator, T ID1, T ID2>
UnaryOperator PseudoTree<T, AssociativeOperator1, AssociativeOperator2, UnaryOperator, ID1, ID2>::inv = UnaryOperator();

}
#endif
