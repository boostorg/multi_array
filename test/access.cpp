// Copyright 2002 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

// 
// access.cpp - operator[] and operator() tests with various arrays
//    The tests assume that they are working on an Array of shape 2x3x4
//

#include "generative_tests.hpp"
#include <boost/array.hpp>
#include <boost/static_assert.hpp>

template <typename Array>
unsigned int access(Array& A, mutable_array_tag const&)
{
    assign(A);
    unsigned int tests_run = access(A, const_array_tag());

    Array const& CA = A;
    return tests_run + access(CA, const_array_tag());
}

template <typename Array>
unsigned int access(Array& A, const_array_tag const&)
{
    unsigned int const ndims = 3;
    BOOST_STATIC_ASSERT((Array::dimensionality == ndims));
    typedef typename Array::index index;
    index const idx0 = A.index_bases()[0];
    index const idx1 = A.index_bases()[1];
    index const idx2 = A.index_bases()[2];

    // operator[]
    int cnum = 0;
    Array const& CA = A;

    for (index i = idx0; i != idx0 + 2; ++i)
    {
        for (index j = idx1; j != idx1 + 3; ++j)
        {
            for (index k = idx2; k != idx2 + 4; ++k)
            {
                BOOST_TEST_EQ(A[i][j][k], cnum++);
                BOOST_TEST_EQ(CA[i][j][k], A[i][j][k]);
            }
        }
    }

    // operator()
    for (index i2 = idx0; i2 != idx0 + 2; ++i2)
    {
        for (index j2 = idx1; j2 != idx1 + 3; ++j2)
        {
            for (index k2 = idx2; k2 != idx2 + 4; ++k2)
            {
                boost::array<index,ndims> indices;
                indices[0] = i2; indices[1] = j2; indices[2] = k2;
                BOOST_TEST_EQ(A(indices), A[i2][j2][k2]);
                BOOST_TEST_EQ(CA(indices), A(indices));
            }
        }
    }

    return 1;
}

int main(int, char*[])
{
    return run_generative_tests();
}

