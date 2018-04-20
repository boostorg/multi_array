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
// stl_interaction.cpp - Make sure multi_arrays work with STL containers.
//

#include <boost/multi_array.hpp>
#include <boost/core/lightweight_test.hpp>
#include <algorithm>
#include <vector>

int main(int, char*[])
{
    typedef boost::multi_array_types::index_range range;
    typedef boost::multi_array<int,3> array3;
    typedef boost::multi_array<int,2> array2;
    typedef std::vector<array3> array3vec;

    int data[] =
    {
        0,1,2,3,
        4,5,6,7,
        8,9,10,11,

        12,13,14,15,
        16,17,18,19,
        20,21,22,23
    };

    int const data_size = 24;

    int insert[] =
    {
        99,98,
        97,96,
    };

    int const insert_size = 4;
    array3 myarray(boost::extents[2][3][4]);
    myarray.assign(data, data + data_size);

    array3vec myvec(5, myarray);
    BOOST_TEST(myarray == myvec[1]);

    array3::array_view<2>::type myview = myarray[
        boost::indices[1][range(0, 2)][range(1, 3)]
    ];

    array2 filler(boost::extents[2][2]);
    filler.assign(insert, insert + insert_size);

    // Modify a portion of myarray through a view (myview)
    myview = filler;

    myvec.push_back(myarray);

    BOOST_TEST(myarray != myvec[1]);
    BOOST_TEST(myarray == myvec[5]);

    return boost::report_errors();
}

