#include <stdio.h>
#include <iostream>
#include <cassert>
#include <string>
#include <thrust/iterator/transform_iterator.h>
#include <thrust/transform_scan.h>
#include <thrust/tuple.h>
#include <thrust/device_vector.h>

#include "tree_function.h"

struct map_parenth : public thrust::unary_function<char,int>
{
    // Maps ( to 1 and ) to 0
    __device__ __host__
    int operator()(char p) const
    {
        return (p - '(') ^ 1;
    }
};

typedef int block_t;
static const unsigned int block_width = sizeof(block_t) * CHAR_BIT;
typedef thrust::device_vector<block_t> device_bit_vector;
typedef thrust::host_vector<block_t> host_bit_vector;

unsigned int whichblock(unsigned int pos )
{
    return pos / block_width;
}

// Could be optimised... but the paper assumes correct input
template <typename charT>
device_bit_vector make_bit_vector(const std::basic_string<charT>& parenths)
{
    int num_bits = parenths.length();
    int num_blocks = (num_bits + block_width - 1)/block_width;
    host_bit_vector bv(num_blocks, 0);
    
    map_parenth f;
    for (int i = 0 ; i < num_bits; i++)
    {
        // set bit if given a 1
        if (f(parenths[i]) == 1)
            bv[whichblock(i)] |= 1<<(block_width - i%block_width - 1);
    }

    // copy to device and return
    return device_bit_vector(bv.begin(), bv.end());
}

typedef int value_t;
typedef thrust::plus<value_t> plus;
typedef thrust::maximum<value_t> maximum;
typedef thrust::negate<value_t> negate;
const int plus_id = 0;
const int maximum_id = INT_MIN;
typedef tf::tree_function<value_t, plus, maximum, negate, plus_id, maximum_id> max_path_tf;

int main(void)
{
    std::string s = "(()((()())())(()())(()))";
    device_bit_vector p = make_bit_vector(s);

    // Write weight matching function that accepts iterators for chars, iterators for weights
    // input parenths, input level-order weights, output weight array
    value_t weights[] = {2, 3, -3, -1, 6, 4, -4, 9, -9, -6, 5, -5, 1, 1, -2, 2, 6, -6, -1, 4, 7, -7, -5, -2};
    thrust::device_vector<value_t> v(weights, weights + s.length());

    max_path_tf mptf;
    int result = mptf(p, v);

    std::cout << "Result: " << result << std::endl;

    return 0;
}
