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
// assign_to_array.cpp - multi_array should be constructible from any other
// array type in the library.
//

#include "generative_tests.hpp"
#include <boost/array.hpp>
#include <boost/cstdlib.hpp>
#include <algorithm>
#include <iostream>

bool equal(int const& a, int const& b)
{
    return a == b;
}

template <typename ArrayA, typename ArrayB>
bool equal(ArrayA const& A, ArrayB const& B)
{
    typename ArrayA::const_iterator ia;
    typename ArrayB::const_iterator ib = B.begin();

    for (ia = A.begin(); ia != A.end(); ++ia, ++ib)
    {
        if (!::equal(*ia, *ib))
        {
            return false;
        }
    }

    return true;
}

template <typename Array>
unsigned int access(Array& A, mutable_array_tag const&)
{
    assign(A);
    return access(A, const_array_tag());
}

template <typename Array>
unsigned int access(Array& A, const_array_tag const&)
{
    typedef boost::multi_array<int,3> array3;
    array3 acopy(A);
    BOOST_TEST((::equal(acopy,A)));
    return 1;
}

int main(int, char*[])
{
    return run_generative_tests();
}

