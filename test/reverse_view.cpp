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
// reverse_view.cpp - a small test of creating a view with negative strides
//

#include <boost/multi_array.hpp>
#include <boost/array.hpp>
#include <boost/core/lightweight_test.hpp>

int main(int, char*[])
{
    // One-dimensional array with views
    double data[] = {1, 2, 3, 4};
    double rdata[] = {4, 3, 2, 1};

    typedef boost::multi_array<double,1> array;
    array A(boost::extents[4]);
    A.assign(data, data + 4);

    typedef array::index_range range;
    array::array_view<1>::type B = A[boost::indices[range(3, -1, -1)]];

    for (boost::multi_array_types::size_type i = 0; i != B.size(); ++i)
    {
        BOOST_TEST_EQ(B[i], rdata[i]);
    }

    return boost::report_errors();
}

