// Copyright 2002 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

#ifndef BOOST_INDEX_GEN_RG071801_HPP
#define BOOST_INDEX_GEN_RG071801_HPP

#include <boost/multi_array/index_range.hpp>
#include <boost/multi_array/range_list.hpp>
#include <boost/multi_array/types.hpp>
#include <boost/array.hpp>
#include <algorithm> 
#include <cstddef>

namespace boost { namespace detail { namespace multi_array {

    template <int NumRanges, int NumDims>
    struct index_gen
    {
     private:
        typedef ::boost::detail::multi_array::index index;
        typedef ::boost::detail::multi_array::size_type size_type;
        typedef index_range<index,size_type> range;

     public:
        template <int Dims, int Ranges>
        struct gen_type
        {
            typedef index_gen<Ranges,Dims> type;
        };

        typedef typename range_list_generator<
            range
          , NumRanges
        >::type range_list;
        range_list ranges_;

        index_gen()
        {
        }

        template <int ND>
        index_gen(
            index_gen<NumRanges-1,ND> const& rhs
          , range const& r
        )
        {
            std::copy(
                rhs.ranges_.begin()
              , rhs.ranges_.end()
              , this->ranges_.begin()
            );
            *this->ranges_.rbegin() = r;
        }

        index_gen<NumRanges+1,NumDims+1>
        operator[](range const& r) const
        {
            index_gen<NumRanges+1,NumDims+1> tmp;
            std::copy(
                this->ranges_.begin()
              , this->ranges_.end()
              , tmp.ranges_.begin()
            );
            *tmp.ranges_.rbegin() = r;
            return tmp;
        }

        index_gen<NumRanges+1,NumDims>
        operator[](index idx) const
        {
            index_gen<NumRanges+1,NumDims> tmp;
            std::copy(
                this->ranges_.begin()
              , this->ranges_.end()
              , tmp.ranges_.begin()
            );
            *tmp.ranges_.rbegin() = range(idx);
            return tmp;
        }

        static index_gen<0,0> indices()
        {
            return index_gen<0,0>();
        }
    };
}}} // namespace boost::detail::multi_array

#endif // BOOST_INDEX_GEN_RG071801_HPP

