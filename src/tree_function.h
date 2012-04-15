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
    V operator()(device_vector<block_type>& blocks,
                 device_vector<V>& values,
                 unsigned int num_segments = 0)
    {
        typedef typename device_vector<block_type>::iterator block_iterator;
        typedef typename device_vector<V>::iterator value_iterator;

        // How wide is a block (in bits)?
        unsigned int block_width = sizeof(block_type) * CHAR_BIT;

        // How many blocks will we have (may not be at full capacity)
        unsigned int num_blocks = (values.size() + block_width - 1)/block_width;
        // Number of segments defaults to each block going to a thread
        // (possibly not all concurrently)
        num_segments = num_segments? num_segments : num_blocks;
        // Number of blocks in a segment
        unsigned int segment_len = (num_blocks + num_segments - 1)
            / num_segments;
        // Use counting iterators so to know the capacity of the last segment
        counting_iterator<int> segment_counter_start(0);
        counting_iterator<int> segment_counter_end(num_segments);

        // Allocate space for pseudo_trees in non-interleaved format.
        // This will ensure it is aligned correctly, hence access coalesced(?).
        device_vector<int> L_v(num_segments);
        device_vector<int> M_v(num_segments);
        device_vector<int> B_v(num_segments);
        device_vector<int> E_v(num_segments);
        device_vector<V>   R_v(num_segments);
        device_vector<V>   A_v(num_segments);
        device_vector<V>   F_v(num_segments);

        /*
        transform_inclusive_scan(
            // TODO: check size at start, return id? if size is 0
            // Begin tuple iterator - cast so multiple values go to 1 thread
            make_zip_iterator(
                make_tuple(stride_iterator<block_iterator>(blocks.begin(), segment_len),
                           stride_iterator<value_iterator>(values.begin(), segment_len*block_width),
                           segment_counter_start)),
            // End tuple iterator
            make_zip_iterator(
                make_tuple(stride_iterator<block_iterator>(blocks.end(), segment_len),
                           stride_iterator<value_iterator>(values.end(), segment_len*block_width),
                           segment_counter_end)),
            // Output iterator to grid (make it behave as if interleaved)
            make_zip_iterator(
                make_tuple(L_v.begin(), M_v.begin(), B_v.begin(),
                           E_v.begin(), R_v.begin(), A_v.begin(),
                           F_v.begin())),
            // Transform segments initially into pseudo-trees before scanning
            sequential_functor<block_type, V>(values.size(), num_segments, segment_len),
            pt.merge);
        */
        
        // typedef tuple<stride_iterator<block_type*> > IteratorTuple;
        // typedef zip_iterator<IteratorTuple> Ziperator;
        // IteratorTuple tup(stride_iterator<block_type*>(blocks.data(), segment_len));
        // Ziperator z(tup);
 
        return F_v[num_segments - 1];
    }

private:
    template <typename block_type, typename value_type>
    class sequential_functor
    {
        static const unsigned int block_width = sizeof(block_type) * CHAR_BIT;

        const unsigned int last_segment_idx;
        const unsigned int last_segment_width;
        const unsigned int segment_width;

        __device__ __host__
        sequential_functor(unsigned int num_elements, unsigned int num_segments, unsigned int num_blocks_in_segment)
        : last_segment_idx(num_segments - 1),
          last_segment_width(num_blocks_in_segment * block_width),
          segment_width(num_elements%(num_blocks_in_segment * block_width))
        {
        }

        __device__ __host__
        ptree_tuple operator()(block_type block, value_type value, int segment_idx)
        {
            ptree_tuple temp;
            block_type * blocks = &block;
            value_type * values = &value;

            unsigned int num_bits = (segment_idx == last_segment_idx)? last_segment_width : segment_width; 
            
            for (int i = 0; i < num_bits; i++)
            {
                bool p = (blocks[i/block_width] >> (block_width - i%block_width - 1))&1;
                value_type v = values[i];
            
                temp = pt.merge(temp, pt.make_pseudo_tree(p, v));
            }

            return temp;
        }
    };

    ptree pt;
};

}

#endif
