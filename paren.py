import math

def lg(x):
    return math.log(x, 2)

# can do this in parallel
def seg_prefix(marks):
    counts = [0, 0]
    for x in marks:
        print x
        yield counts[x]
        counts[x] += 1

# expects parentheses in the form of 1 - ( and 0 - )
def match_parentheses(paren):
    n = len(paren)
    paren = enumerate(paren)
    # to convert 0-basing to 1-basing -> i + 1
    marks = [mark_parenth(i, p) for (i,p) in paren]
    seg_pref = seg_prefix(marks)
    new = paren
    for i in range(n):
        
def mark_parenth(i, p):
    # Flag each odd-( and even-) as 0
    # This corresponds to XOR when ( is 1 and ) is 0,
    # and even is 0 and odd is 1:
    #           i
    #       even odd
    # ---------------
    # x  )    0   1
    #    (    1   0
    return p ^ (i % 2)

def map_parenth(x):
    return (ord(x) - ord('(')) ^ 1

if __name__ == '__main__':
    paren = '(()())'
    print paren
    paren = map(map_parenth, paren)
    print ''.join(map(str, paren))
    print match_parentheses(paren)
