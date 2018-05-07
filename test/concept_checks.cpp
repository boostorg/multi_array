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
// concept_checks.cpp -
//   make sure the types meet concept requirements
//

#include <boost/multi_array.hpp>
#include <boost/multi_array/concept_checks.hpp>
#include <boost/concept_check.hpp>
#include <boost/cstdlib.hpp>
#include <boost/array.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config/workaround.hpp>

int main(int, char*[])
{
    int const ndims = 3;
    typedef boost::multi_array<int,ndims> array;
    typedef boost::multi_array_ref<int,ndims> array_ref;
    typedef boost::const_multi_array_ref<int,ndims> const_array_ref;
    typedef array::array_view<ndims>::type array_view;
    typedef array::const_array_view<ndims>::type const_array_view;
    typedef array::subarray<ndims>::type subarray;
    typedef array::const_subarray<ndims>::type const_subarray;

#if BOOST_WORKAROUND(BOOST_MSVC, < 1600)
    // MSVC 9.0 currently ICEs on these.
#else
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::ConstMultiArrayConcept<array,ndims>
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::ConstMultiArrayConcept<array_ref,ndims>
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::ConstMultiArrayConcept<
            const_array_ref
          , ndims
        >
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::ConstMultiArrayConcept<array_view,ndims>
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::ConstMultiArrayConcept<
            const_array_view
          , ndims
        >
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::ConstMultiArrayConcept<subarray,ndims>
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::ConstMultiArrayConcept<
            const_subarray
          , ndims
        >
    ));

    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::MutableMultiArrayConcept<array,ndims>
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::MutableMultiArrayConcept<array_ref,ndims>
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::MutableMultiArrayConcept<
            array_view
          , ndims
        >
    ));
    BOOST_CONCEPT_ASSERT((
        boost::multi_array_concepts::MutableMultiArrayConcept<subarray,ndims>
    ));
#endif // MSVC 9.0

    return boost::report_errors();
}

