NEG_INF = -1e308
import itertools as it

# Handles the psuedotree operations and creation
# based on user provided operators, inverse functions and identities
class PseudoTreeHandler:
    # Use templates in C++ version
    def __init__(self, op1, op2, inv, id1, id2):
        self.op1 = op1
        self.op2 = op2
        self.inv = inv
        self.id1 = id1
        self.id2 = id2

    def make(self, p, w):
        if p is '(':
            return (1, 0, 1, 1, w, w, self.id2)
        elif p is ')':
            return (-1, -1, -1, -1, w, self.id1, self.id2)
        elif p is '':
            return (0, 0, 0, 0, self.id1, self.id1, self.id2)
        assert(p is '(' or p is ')' or p is '') #shouldnt be here
    
    def merge(self, (L1, M1, B1, E1, R1, A1, F1),
                    (L2, M2, B2, E2, R2, A2, F2) ):
        L = L1 + L2
        M = min(M1, L1 + M2)
        B = B2 if B1 == 0 else B1
        E = E1 if E2 == 0 else E2
        R = self.op1(R1, R2)
        A = self.op1(A1, R2) if M == M1 else A2
        F1_prime = self.op2(F1, A1) if E1 == 1 and B2 == -1 else F1
        H = self.op1(self.op1(A1, R2), self.inv(A2))
        F = self.op2(F1_prime, self.op1(H, F2)) if M == M1 else \
                self.op2(self.op1(self.inv(H),F1_prime), F2)
        return (L, M, B, E, R, A, F)

# this is parallelisable too, but chunks are handled sequentially in the paper
def block_pseudotree(P, W, t):
    return reduce(t.merge, (t.make(p, w) for (p, w) in it.izip(P, W)))

if __name__ == '__main__':
    P = '(()((()())())(()())(()))'
    W = [2, 3, -3, -1, 6, 4, -4, 9, -9, -6, 5,
         -5, 1, 1, -2, 2, 6, -6, -1, 5, 7, -7, -5,-2 ]
    assert(sum(W) is 0)
    n = len(P)/2
    # number of processors
    p = 6

    # when using bitmaps, calculate block size with base type in mind
    block_len = 2*n/p
    block_positions = range(0, 2*n, block_len)

    t = PseudoTreeHandler(lambda x,y: x + y,
                          lambda x,y: max(x,y),
                          lambda x:-x,
                          0, NEG_INF)
    
    # this loop easily done in parallel using a parallel map()
    P_blocks = [P[i:i+block_len] for i in block_positions]
    W_blocks = [W[i:i+block_len] for i in block_positions]
    blocks = it.izip(P_blocks, W_blocks)
    p_trees = map(lambda (p,w):block_pseudotree(p,w,t), blocks)

    # could use a parallel reduce
    print reduce(t.merge, p_trees)[-1]

    # not sure, but the two steps above could probably be merged into a single
    # prefix sum
