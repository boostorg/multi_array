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
// slice.cpp - testing out slicing on a matrices
//

#include "generative_tests.hpp"
#include <boost/array.hpp>
#include <boost/mpl/if.hpp>
#include <boost/config.hpp>

#if defined(BOOST_NO_CXX11_HDR_TYPE_TRAITS)
#include <boost/type_traits/is_same.hpp>
#else
#include <type_traits>
#endif

template <typename Array>
struct view_traits_mutable
{
#if defined(BOOST_MSVC)
    // RG - MSVC can't handle templates nested in templates.  Use traits.
    typedef typename boost::array_view_gen<Array,3>::type array_view3;
    typedef typename boost::array_view_gen<Array,2>::type array_view2;
#else
    typedef typename Array::BOOST_NESTED_TEMPLATE array_view<
        3
    >::type array_view3;
    typedef typename Array::BOOST_NESTED_TEMPLATE array_view<
        2
    >::type array_view2;
#endif
};

template <typename Array>
struct view_traits_const
{
#if defined(BOOST_MSVC)
    // RG - MSVC can't handle templates nested in templates.  Use traits.
    typedef typename boost::const_array_view_gen<Array,3>::type array_view3;
    typedef typename boost::const_array_view_gen<Array,2>::type array_view2;
#else
    typedef typename Array::BOOST_NESTED_TEMPLATE const_array_view<
        3
    >::type array_view3;
    typedef typename Array::BOOST_NESTED_TEMPLATE const_array_view<
        2
    >::type array_view2;
#endif
};

// Meta-program selects the proper view_traits implementation.
template <typename Array, typename ConstTag>
struct view_traits_generator
  : boost::mpl::if_<
#if defined(BOOST_NO_CXX11_HDR_TYPE_TRAITS)
        boost::is_same<ConstTag,const_array_tag>
#else
        std::is_same<ConstTag,const_array_tag>
#endif
      , view_traits_const<Array>
      , view_traits_mutable<Array>
    >
{
};

template <typename Array, typename ViewTraits>
unsigned int test_views(Array& A, ViewTraits const&)
{
    typedef typename Array::index index;
    typedef typename Array::index_range range;
    typename Array::index_gen indices;

    index const idx0 = A.index_bases()[0];
    index const idx1 = A.index_bases()[1];
    index const idx2 = A.index_bases()[2];

    // Standard View
    {
        typename ViewTraits::array_view3 B = A[
            indices
            [range(idx0 + 0, idx0 + 2)]
            [range(idx1 + 1, idx1 + 3)]
            [range(idx2 + 0, idx2 + 4, 2)]
        ];

        for (index i = 0; i != 2; ++i)
        {
            for (index j = 0; j != 2; ++j)
            {
                for (index k = 0; k != 2; ++k)
                {
                    BOOST_TEST_EQ(
                        B[i][j][k]
                      , A[idx0 + i][idx1 + j + 1][idx2 + k * 2]
                    );
                    boost::array<index,3> elmts;
                    elmts[0] = i; elmts[1] = j; elmts[2] = k;
                    BOOST_TEST_EQ(
                        B(elmts)
                      , A[idx0 + i][idx1 + j + 1][idx2 + k * 2]
                    );
                }
            }
        }
    }

    // Degenerate dimensions
    {
        typename ViewTraits::array_view2 B = A[
            indices
            [range(idx0 + 0, idx0 + 2)]
            [idx1+1]
            [range(idx2 + 0, idx2 + 4, 2)]
        ];

        for (index i = 0; i != 2; ++i)
        {
            for (index j = 0; j != 2; ++j)
            {
                BOOST_TEST_EQ(
                    B[i][j]
                  , A[idx0 + i][idx1 + 1][idx2 + j * 2]
                );
                boost::array<index,2> elmts;
                elmts[0] = i; elmts[1] = j;
                BOOST_TEST_EQ(
                    B(elmts)
                  , A[idx0 + i][idx1 + 1][idx2 + j * 2]
                );
            }
        }
    }

    // Flip the third dimension
    {
        typename ViewTraits::array_view3 B = A[
            indices
            [range(idx0 + 0, idx0 + 2)]
            [range(idx1 + 0, idx1 + 2)]
            [range(idx2 + 2, idx2 + 0, -1)]
        ];
#if 0
        typename ViewTraits::array_view3 B = A[
            indices
            [range(idx0 + 0, idx0 + 2)]
            [idx1 + 1]
            [range(idx2 + 0, idx2 + 4, 2)]
        ];
#endif

        for (index i = 0; i != 2; ++i)
        {
            for (index j = 0; j != 2; ++j)
            {
                for (index k = 0; k != 2; ++k)
                {
                    BOOST_TEST_EQ(
                        B[i][j][k]
                      , A[idx0 + i][idx1 + j][idx2 + 2 - k]
                    );
                    boost::array<index,3> elmts;
                    elmts[0] = i; elmts[1] = j; elmts[2] = k;
                    BOOST_TEST_EQ(
                        B(elmts)
                      , A[idx0 + i][idx1 + j][idx2 + 2 - k]
                    );
                }
            }
        }
    }

    return 1;
}

template <typename Array>
unsigned int access(Array& A, mutable_array_tag const&)
{
    assign(A);

    typedef typename view_traits_generator<Array,mutable_array_tag>::type
        m_view_traits;

    typedef typename view_traits_generator<Array,const_array_tag>::type
        c_view_traits;

    unsigned int test_runs = test_views(A, m_view_traits());
    test_runs += test_views(A, c_view_traits());

    Array const& CA = A;
    return test_runs + test_views(CA, c_view_traits());
}

template <typename Array>
unsigned int access(Array& A, const_array_tag const&)
{
    typedef typename view_traits_generator<Array,const_array_tag>::type
        c_view_traits;
    return test_views(A, c_view_traits());
}

int main(int, char*[])
{
    return run_generative_tests();
}

