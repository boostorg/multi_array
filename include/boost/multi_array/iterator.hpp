// Copyright 2002 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

#ifndef ITERATOR_RG071801_HPP
#define ITERATOR_RG071801_HPP

//
// iterator.hpp - implementation of iterators for the
// multi-dimensional array class
//

#include <boost/multi_array/base.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/tti/detail/dnullptr.hpp>
#include <algorithm>
#include <cstddef>
#include <iterator>

namespace boost { namespace detail { namespace multi_array {

    /////////////////////////////////////////////////////////////////////////
    // iterator components
    /////////////////////////////////////////////////////////////////////////

    template <class T>
    struct operator_arrow_proxy
    {
        operator_arrow_proxy(T const& px) : value_(px)
        {
        }

        T* operator->() const
        {
            return &this->value_;
        }

        // This function is needed for MWCW and BCC, which won't call
        // operator-> again automatically per 13.3.1.2 para 8
        operator T*() const
        {
            return &this->value_;
        }

        mutable T value_;
    };

    template <
        typename T
      , typename TPtr
      , typename NumDims
      , typename Reference
      , typename IteratorCategory
    >
    class array_iterator;

    template <
        typename T
      , typename TPtr
      , typename NumDims
      , typename Reference
      , typename IteratorCategory
    >
    class array_iterator
      : public iterator_facade<
            array_iterator<T,TPtr,NumDims,Reference,IteratorCategory>
          , typename associated_types<T,NumDims>::value_type
          , IteratorCategory
          , Reference
        >
      , private value_accessor_generator<T,NumDims>::type
    {
        friend class ::boost::iterator_core_access;
        typedef detail::multi_array::associated_types<T,NumDims> access_t;

        typedef iterator_facade<
            array_iterator<T,TPtr,NumDims,Reference,IteratorCategory>
          , typename detail::multi_array::associated_types<
                T
              , NumDims
            >::value_type
          , boost::random_access_traversal_tag
          , Reference
        > facade_type;

        typedef typename access_t::index index;
        typedef typename access_t::size_type size_type;

#if defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
     public:
#else
        template <typename, typename, typename, typename, typename>
        friend class array_iterator;
#endif 

        index idx_;
        TPtr base_;
        size_type const* extents_;
        index const* strides_;
        index const* index_base_;

     public:
        // Typedefs to circumvent ambiguities between parent classes
        typedef typename facade_type::reference reference;
        typedef typename facade_type::value_type value_type;
        typedef typename facade_type::difference_type difference_type;

        array_iterator()
        {
        }

        array_iterator(
            index idx
          , TPtr base
          , size_type const* extents
          , index const* strides
          , index const* index_base
        ) : idx_(idx)
          , base_(base)
          , extents_(extents)
          , strides_(strides)
          , index_base_(index_base)
        {
        }

        template <typename OPtr, typename ORef, typename Cat>
        array_iterator(
            array_iterator<T,OPtr,NumDims,ORef,Cat> const& rhs
          , typename boost::enable_if_convertible<
                OPtr
              , TPtr
            >::type* = BOOST_TTI_DETAIL_NULLPTR
        ) : idx_(rhs.idx_)
          , base_(rhs.base_)
          , extents_(rhs.extents_)
          , strides_(rhs.strides_)
          , index_base_(rhs.index_base_)
        {
        }

        // RG - we make our own operator->
        operator_arrow_proxy<reference> operator->() const
        {
            return operator_arrow_proxy<reference>(this->dereference());
        }

        reference dereference() const
        {
            typedef typename value_accessor_generator<
                T
              , NumDims
            >::type accessor;
            return accessor::access(
                boost::type<reference>()
              , this->idx_
              , this->base_
              , this->extents_
              , this->strides_
              , this->index_base_
            );
        }

        void increment()
        {
            ++this->idx_;
        }

        void decrement()
        {
            --this->idx_;
        }

        template <typename IteratorAdaptor>
        bool equal(IteratorAdaptor& rhs) const
        {
            std::size_t const N = NumDims::value;
            return (this->idx_ == rhs.idx_) && (this->base_ == rhs.base_) && (
                (this->extents_ == rhs.extents_) || std::equal(
                    this->extents_
                  , this->extents_ + N
                  , rhs.extents_
                )
            ) && (
                (this->strides_ == rhs.strides_) || std::equal(
                    this->strides_
                  , this->strides_ + N
                  , rhs.strides_
                )
            ) && (
                (this->index_base_ == rhs.index_base_) || std::equal(
                    this->index_base_
                  , this->index_base_ + N
                  , rhs.index_base_
                )
            );
        }

        template <typename DifferenceType>
        void advance(DifferenceType n)
        {
            this->idx_ += n;
        }

        template <class IteratorAdaptor>
        typename facade_type::difference_type
            distance_to(IteratorAdaptor& rhs) const
        {
            return rhs.idx_ - this->idx_;
        }
    };
}}} // namespace boost::detail::multi_array

#endif // ITERATOR_RG071801_HPP

