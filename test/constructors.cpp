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
// constructors.cpp - Testing out the various constructor options
//

#include <boost/multi_array.hpp>
#include <boost/core/lightweight_test.hpp>
#include <algorithm>
#include <list>

void check_shape(double const&, std::size_t*, int*, unsigned long)
{
}

template <typename Array>
void
    check_shape(
        Array const& A
      , std::size_t* sizes
      , int* strides
      , unsigned long num_elements
    )
{
    BOOST_TEST_EQ(A.num_elements(), num_elements);
    BOOST_TEST_EQ(A.size(), *sizes);
    BOOST_TEST((
        std::equal(sizes, sizes + A.num_dimensions(), A.shape())
    ));
    BOOST_TEST((
        std::equal(strides, strides + A.num_dimensions(), A.strides())
    ));
    check_shape(A[0], ++sizes, ++strides, num_elements / A.size());
}

bool equal(double const& a, double const& b)
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

int main(int, char*[])
{
    typedef boost::multi_array<double,3>::size_type size_type;
    boost::array<size_type,3> sizes = {{3, 3, 3}};
    int strides[] = {9, 3, 1};
    size_type num_elements = 27;

    // Default multi_array constructor
    {
        boost::multi_array<double,3> A;
    }

    // Constructor 1, default storage order and allocator
    {
        boost::multi_array<double,3> A(sizes);
        check_shape(A, &sizes[0], strides, num_elements);

        double* ptr = 0;
        boost::multi_array_ref<double,3> B(ptr, sizes);
        check_shape(B, &sizes[0], strides, num_elements);

        double const* cptr = ptr;
        boost::const_multi_array_ref<double,3> C(cptr, sizes);
        check_shape(C, &sizes[0], strides, num_elements);
    }

    // Constructor 1, fortran storage order and user-supplied allocator
    {
        typedef boost::multi_array<
            double
          , 3
          , std::allocator<double>
        >::size_type size_type;
        size_type num_elements = 27;
        int col_strides[] = {1, 3, 9};

        boost::multi_array<
            double
          , 3
          , std::allocator<double>
        > A(sizes, boost::fortran_storage_order());
        check_shape(A, &sizes[0], col_strides, num_elements);

        double* ptr = 0;
        boost::multi_array_ref<double,3> B(
            ptr
          , sizes
          , boost::fortran_storage_order()
        );
        check_shape(B, &sizes[0], col_strides, num_elements);

        double const* cptr = ptr;
        boost::const_multi_array_ref<double,3> C(
            cptr
          , sizes
          , boost::fortran_storage_order()
        );
        check_shape(C, &sizes[0], col_strides, num_elements);
    }

    // Constructor 2, default storage order and allocator
    {
        typedef boost::multi_array<double,3>::size_type size_type;
        size_type num_elements = 27;

        boost::multi_array<double,3>::extent_gen extents;
        boost::multi_array<double,3> A(extents[3][3][3]);
        check_shape(A, &sizes[0], strides, num_elements);

        double* ptr = 0;
        boost::multi_array_ref<double,3> B(ptr, extents[3][3][3]);
        check_shape(B, &sizes[0], strides, num_elements);

        double const* cptr = ptr;
        boost::const_multi_array_ref<double,3> C(cptr, extents[3][3][3]);
        check_shape(C, &sizes[0], strides, num_elements);
    }

    // Copy Constructors
    {
        typedef boost::multi_array<double,3>::size_type size_type;
        size_type num_elements = 27;
        std::vector<double> vals(27, 4.5);

        boost::multi_array<double,3> A(sizes);
        A.assign(vals.begin(),vals.end());
        boost::multi_array<double,3> B(A);
        check_shape(B, &sizes[0], strides, num_elements);
        BOOST_TEST((::equal(A, B)));

        double ptr[27];
        boost::multi_array_ref<double,3> C(ptr, sizes);
        A.assign(vals.begin(), vals.end());
        boost::multi_array_ref<double,3> D(C);
        check_shape(D, &sizes[0], strides, num_elements);
        BOOST_TEST_EQ(C.data(), D.data());

        double const* cptr = ptr;
        boost::const_multi_array_ref<double,3> E(cptr, sizes);
        boost::const_multi_array_ref<double,3> F(E);
        check_shape(F, &sizes[0], strides, num_elements);
        BOOST_TEST_EQ(E.data(), F.data());
    }

    // Conversion construction
    {
        typedef boost::multi_array<double,3>::size_type size_type;
        size_type num_elements = 27;
        std::vector<double> vals(27, 4.5);

        boost::multi_array<double,3> A(sizes);
        A.assign(vals.begin(), vals.end());
        boost::multi_array_ref<double,3> B(A);
        boost::const_multi_array_ref<double,3> C(A);
        check_shape(B, &sizes[0], strides, num_elements);
        check_shape(C, &sizes[0], strides, num_elements);
        BOOST_TEST_EQ(B.data(), A.data());
        BOOST_TEST_EQ(C.data(), A.data());

        double ptr[27];
        boost::multi_array_ref<double,3> D(ptr, sizes);
        D.assign(vals.begin(), vals.end());
        boost::const_multi_array_ref<double,3> E(D);
        check_shape(E, &sizes[0], strides, num_elements);
        BOOST_TEST_EQ(E.data(), D.data());
    }

    // Assignment Operator
    {
        typedef boost::multi_array<double,3>::size_type size_type;
        size_type num_elements = 27;
        std::vector<double> vals(27, 4.5);

        boost::multi_array<double,3> A(sizes), B(sizes);
        A.assign(vals.begin(), vals.end());
        B = A;
        check_shape(B, &sizes[0], strides, num_elements);
        BOOST_TEST((::equal(A, B)));
        double ptr1[27];
        double ptr2[27];
        boost::multi_array_ref<double,3> C(ptr1, sizes), D(ptr2, sizes);
        C.assign(vals.begin(), vals.end());
        D = C;
        check_shape(D, &sizes[0], strides, num_elements);
        BOOST_TEST((::equal(C, D)));
    }

    // subarray value_type is multi_array
    {
        typedef boost::multi_array<double,3> array;
        typedef array::size_type size_type;
        size_type num_elements = 27;
        std::vector<double> vals(num_elements, 4.5);

        boost::multi_array<double,3> A(sizes);
        A.assign(vals.begin(), vals.end());

        typedef array::subarray<2>::type subarray;
        subarray B = A[1];
        subarray::value_type C = B[0];

        // should comparisons between the types work?
        BOOST_TEST((::equal(A[1][0], C)));
        BOOST_TEST((::equal(B[0], C)));
    }

    return boost::report_errors();
}

