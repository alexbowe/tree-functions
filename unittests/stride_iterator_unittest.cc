#include <limits.h>
#include "stride_iterator.h"
#include "gtest/gtest.h"
#include <thrust/device_vector.h>

TEST(stride_iterator, device_vector)
{
    int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
    thrust::device_vector<int> v(a, a+8);

    tf::stride_iterator<thrust::device_vector<int>::iterator> st(v.begin(), 2);
    
    EXPECT_EQ(st[0], 1);
    EXPECT_EQ(st[1], 3);
    EXPECT_EQ(st[2], 5);
    EXPECT_EQ(st[3], 7);
    //EXPECT_EQ(st[4], 0); // make an exception handling unit test
}

TEST(stride_iterator, begin_end)
{
    int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
    thrust::device_vector<int> v(a, a+8);

    tf::stride_iterator<thrust::device_vector<int>::iterator> begin(v.begin(), 2);
    tf::stride_iterator<thrust::device_vector<int>::iterator> end(v.end(), 2);

    EXPECT_TRUE(begin < end);
    begin+=5;
    EXPECT_FALSE(begin < end);
}
