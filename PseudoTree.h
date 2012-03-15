#pragma once
#ifndef PSEUDO_TREE_H
#define PSEUDO_TREE_H

namespace tree_functions
{

// TODO: look at alignment, maybe use cuda small vector types
template<typename T, typename AssociativeOperator, typename UnaryOperator, T ID1, T ID2>
class PseudoTree
{
public:
    // static member classes passed as templates?
    static AssociativeOperator op1;
    static AssociativeOperator op2;
    static UnaryOperator inv;


    __device__ __host__
    PseudoTree() : L(0), M(0), B(0), E(0), R(ID1), A(ID1), F(ID2) { ; }

    __device__ __host__
    // ESSENTIAL that parenth doesnt take a value other than 0 or 1
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
    
    // templated constructor with block type parameter that adds them sequentially?
    __device__ __host__
    template <typename AssociativeOperator, typename UnaryOperator>
    PseudoTree merge(const PseudoTree& a, const PseudoTree& b, AssociativeOperator op1, AssociativeOperator op2, UnaryOperator inv)
    {
        int L = a.L + b.L;
        int M = min(a.M, b.M);
        int B = a.B? a.B : b.B;
        int E = b.E? b.E : a.E;
        T R = op1(a.R, b.R);
        T A = (M == a.M)? op1(a.A, b.R) : b.A;
        T F_prime = 0;//(E == 1 && b.B == -1)? OP2(F, A) : F;
        T H = 0;//OP1(OP1(A, b.R), Inv1(b.A));
        T new_F = 0;//(new_M == M)? OP2(F_prime, OP1(H, b.F)) : OP2(OP1(Inv1(H), F_prime), b.F);
        return PseudoTree(new_L, new_M, new_B, new_E, new_R, new_A, new_F);
    }

//private:
    // could use cuda int4 here?
    int L, M, B, E;
    T R, A, F;

    __device__ __host__
    PseudoTree(int l, int m, int b, int e, T r, T a, T f) : L(l), M(m), B(b), E(e), R(r), A(a), F(f) { ; }
};

}
#endif
