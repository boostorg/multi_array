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
// test out storage_order stuff
//

#include <boost/multi_array/storage_order.hpp>
#include <boost/core/lightweight_test.hpp>

int main(int, char*[])
{
    boost::array<std::size_t,5> c_ordering = {{4, 3, 2, 1, 0}};
    boost::array<std::size_t,5> fortran_ordering = {{0, 1, 2, 3, 4}};
    boost::array<bool,5> ascending = {{true, true, true, true, true}};
    boost::general_storage_order<5> c_storage(
        c_ordering.begin()
      , ascending.begin()
    );
    boost::general_storage_order<5> fortran_storage(
        fortran_ordering.begin()
      , ascending.begin()
    );

    BOOST_TEST(
        c_storage == (
            boost::general_storage_order<5>
        )(boost::c_storage_order())
    );
    BOOST_TEST(
        fortran_storage == (
            boost::general_storage_order<5>
        )(boost::fortran_storage_order())
    );

    return boost::report_errors();
}

