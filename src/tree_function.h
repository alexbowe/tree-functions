#ifndef TREE_FUNCTION_H
#define TREE_FUNCTION_H

#include "pseudo_tree.h"
#include "stride_iterator.h"
#include <limits.h>
#include <thrust/tuple.h>
#include <thrust/device_vector.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/transform_scan.h>


namespace tf
{

using namespace thrust;

template <typename V, class Op1, class Op2, class Inv, V id1, V id2>
class tree_function
{

public:
    typedef pseudo_tree<V, Op1, Op2, Inv, id1, id2> ptree;
    typedef typename ptree::tuple ptree_tuple;

    // num_segments is a parameter because there will be an optimal
    // value for this dependent on the hardware. Defaults to let Thrust do this
    // implicitly (that is, 1 block-width of values per thread).
    template <typename block_type>
    __host__
    V operator()(const device_vector<block_type>& blocks,
                 const device_vector<V>& values,
                 int num_segments = 0)
    {
        sequential_step<block_type, V> seq(blocks, values, num_segments);

        // Allocate space for pseudo_trees in non-interleaved format.
        // This will ensure it is aligned correctly, hence access coalesced(?).
        device_vector<int> L_v(seq.num_segments);
        device_vector<int> M_v(seq.num_segments);
        device_vector<int> B_v(seq.num_segments);
        device_vector<int> E_v(seq.num_segments);
        device_vector<V>   R_v(seq.num_segments);
        device_vector<V>   A_v(seq.num_segments);
        device_vector<V>   F_v(seq.num_segments);

        transform_inclusive_scan(
            // INPUT segment indexes in the range [0, num_segments)
            make_counting_iterator(0), make_counting_iterator(seq.num_segments),
            // OUTPUT to Zip iterator means when we output a tuple, each element is
            // redirected to its respective vector.
            make_zip_iterator(make_tuple(L_v.begin(), M_v.begin(), B_v.begin(),
                                         E_v.begin(), R_v.begin(), A_v.begin(),
                                         F_v.begin())),
            // For each segment, transform with the sequential step of the alg.
            // That is, map each bit-value-pair to a tuple, and merge them.
            seq,
            pt);
 
        return F_v[seq.num_segments - 1];
    }

    template <typename block_type, typename value_type>
    struct sequential_step
    {
        static const unsigned int block_width = sizeof(block_type) * CHAR_BIT;

        const device_vector<block_type>& blocks;
        const device_vector<V>& values;

        int num_segments;
        int num_blocks_in_segment;

        __device__ __host__
        sequential_step( const device_vector<block_type>& _blocks,
                         const device_vector<V>& _values,
                         unsigned int _num_segments)
        : blocks(_blocks), values(_values)
        {
            
            // How many blocks will we have (may not be at full capacity)
            unsigned int num_blocks = (values.size() + block_width - 1)/block_width;
            // Number of segments defaults to each block going to a thread
            // (possibly not all concurrently)
            num_segments = _num_segments? _num_segments : num_blocks;

            // Number of blocks in a segment
            num_blocks_in_segment = (num_blocks + num_segments - 1)/num_segments;
        }

        template <typename Integral>
        __device__ __host__
        ptree_tuple operator()(Integral segment_idx)
        {
            ptree_tuple result = ptree::make_pseudo_tree();
/*
            // There are fewer bits in the last segment
            unsigned int num_bits = (segment_idx == num_segments - 1)?
                values.size() % num_segments
                : num_blocks_in_segment * block_width; 
           
            int seg_start = segment_idx * segment_len * block_width;
            for (int i = seg_start; i < num_bits; i++)
            {
                bool p = (blocks[i/block_width] >> (block_width - i%block_width - 1))&1;
                value_type v = values[i];
            
                result = pt(temp, ptree::make_pseudo_tree(p, v));
            }
*/
            return result;
        }
    };

    ptree pt;
};

}

#endif
