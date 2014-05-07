/*
 Copyright (c) 2014 Glen Joseph Fernandes
 glenfe at live dot com

 Distributed under the Boost Software License,
 Version 1.0. (See accompanying file LICENSE_1_0.txt
 or copy at http://boost.org/LICENSE_1_0.txt)
*/
#ifndef BOOST_MULTI_ARRAY_ALLOCATOR_FILL_HPP
#define BOOST_MULTI_ARRAY_ALLOCATOR_FILL_HPP

#include <boost/config.hpp>
#include <boost/container/allocator_traits.hpp>

namespace boost {
  namespace detail {
    namespace multi_array {
      template<class Allocator, class ForwardIt, class Size>
      void allocator_fill(Allocator& alloc, ForwardIt first,
        Size count)
      {
        ForwardIt current = first;
#if !defined(BOOST_NO_EXCEPTIONS)
        try {
#endif
          for (; count > 0; ++current, --count) {
            boost::container::allocator_traits<Allocator>::
              construct(alloc, &*current);
          }
#if !defined(BOOST_NO_EXCEPTIONS)
        } catch (...) {
          for (; first != current; ++first) {
            boost::container::allocator_traits<Allocator>::
              destroy(alloc, &*first);
          }
          throw;
        }
#endif
      }
    }
  }
}

#endif
