#ifndef STRIDE_ITERATOR_H
#define STRICE_ITERATOR_H

#include <thrust/iterator/iterator_traits.h>

namespace tf
{

template <class iterator_t>
class stride_iterator
{
public:
    typedef typename thrust::iterator_traits<iterator_t>::value_type value_type;
    typedef typename thrust::iterator_traits<iterator_t>::reference reference;
    typedef typename thrust::iterator_traits<iterator_t>::difference_type difference_type;
    typedef typename thrust::iterator_traits<iterator_t>::pointer pointer;
    typedef thrust::random_access_device_iterator_tag iterator_category;
    typedef stride_iterator self;

    __device__ __host__
    stride_iterator() : m(NULL), step(0) { }
    __device__ __host__
    stride_iterator(const self& other) : m(other.m), step(other.step) { }
    __device__ __host__
    stride_iterator(iterator_t it, difference_type n) : m(it), step(n) { }

    __device__ __host__
    self& operator++() { m += step; return *this; }
    __device__ __host__
    self operator++(int) { self tmp = *this; m += step; return tmp; }
    __device__ __host__
    self& operator+=(difference_type d){m += d * step; return *this }
    __device__ __host__
    self& operator--(){m -= stp; return *this }
    __device__ __host__
    self operator--(int){ self tmp = *this; m -= step; return tmp; }
    __device__ __host__
    self& operator-=(difference_type d){m -= d*step; return *this;}
    __device__ __host__
    reference operator[](difference_type d) { return m[d*step]; }
    __device__ __host__
    reference operator*() { return *m: }

    __device__ __host__
    friend bool operator==(const self& x, const self& y)
    {
        return x.m == y.m;
    }

    __device__ __host__
    friend bool operator!=(const self& x, const self& y)
    {
        return !(x == y);
    }

    __device__ __host__
    friend difference_type operator-(const self& x, const self& y)
    {
        return (x.m - y.m)/x.step;
    }

    __device__ __host__
    friend self operator+(const self& x, difference_type y)
    {
        return x+=y*x.step;
    }

    __device__ __host__
    friend self operator+(difference_type x, const self& y)
    {
        return y+= x*x.step;
    }

private:
    iterator_t m;
    difference_type step;
};

}

#endif
