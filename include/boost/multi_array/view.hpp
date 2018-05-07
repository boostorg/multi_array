// Copyright 2002 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

#ifndef BOOST_MULTI_ARRAY_VIEW_RG071301_HPP
#define BOOST_MULTI_ARRAY_VIEW_RG071301_HPP

//
// view.hpp - code for creating "views" of array data.
//

#include <boost/multi_array/base.hpp>
#include <boost/multi_array/concept_checks.hpp>
#include <boost/multi_array/iterator.hpp>
#include <boost/multi_array/storage_order.hpp>
#include <boost/multi_array/subarray.hpp>
#include <boost/multi_array/algorithm.hpp>
#include <boost/array.hpp>
#include <boost/limits.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>

#if !defined(BOOST_NO_SFINAE)
#include <boost/core/enable_if.hpp>
#if defined(BOOST_NO_CXX11_HDR_TYPE_TRAITS)
#include <boost/type_traits/is_integral.hpp>
#else
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <type_traits>
#endif
#endif

namespace boost { namespace detail { namespace multi_array {

    // TPtr = T const* defaulted in base.hpp
    template <typename T, std::size_t NumDims, typename TPtr>
    class const_multi_array_view
      : public boost::detail::multi_array::multi_array_impl_base<T,NumDims>
    {
        typedef boost::detail::multi_array::multi_array_impl_base<
            T
          , NumDims
        > super_type;

     public:
        typedef typename super_type::value_type value_type;
        typedef typename super_type::const_reference const_reference;
        typedef typename super_type::const_iterator const_iterator;
        typedef typename super_type::const_reverse_iterator
            const_reverse_iterator;
        typedef typename super_type::element element;
        typedef typename super_type::size_type size_type;
        typedef typename super_type::difference_type difference_type;
        typedef typename super_type::index index;
        typedef typename super_type::extent_range extent_range;

        // template typedefs
        template <std::size_t NDims>
        struct const_array_view
        {
            typedef boost::detail::multi_array::const_multi_array_view<
                T
              , NDims
            > type;
        };

        template <std::size_t NDims>
        struct array_view
        {
            typedef boost::detail::multi_array::multi_array_view<
                T
              , NDims
            > type;
        };

        template <typename OPtr>
        const_multi_array_view(
            const_multi_array_view<T,NumDims,OPtr> const& other
        ) : base_(other.base_)
          , origin_offset_(other.origin_offset_)
          , num_elements_(other.num_elements_)
          , extent_list_(other.extent_list_)
          , stride_list_(other.stride_list_)
          , index_base_list_(other.index_base_list_)
        {
        }

        template <typename BaseList>
#if defined(BOOST_NO_SFINAE)
        void
#else
        typename boost::disable_if<
#if defined(BOOST_NO_CXX11_HDR_TYPE_TRAITS)
            boost::is_integral<BaseList>
#else
            typename boost::mpl::if_<
                std::is_integral<BaseList>
              , boost::mpl::true_
              , boost::mpl::false_
            >::type
#endif
          , void
        >::type
#endif // BOOST_NO_SFINAE
            reindex(BaseList const& values)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<BaseList>));
            boost::detail::multi_array::copy_n(
                values.begin()
              , this->num_dimensions()
              , this->index_base_list_.begin()
            );
            this->origin_offset_ = this->calculate_indexing_offset(
                this->stride_list_
              , this->index_base_list_
            );
        }

        void reindex(index value)
        {
            this->index_base_list_.assign(value);
            this->origin_offset_ = this->calculate_indexing_offset(
                this->stride_list_
              , this->index_base_list_
            );
        }

        size_type num_dimensions() const
        {
            return NumDims;
        }

        size_type size() const
        {
            return this->extent_list_.front();
        }

        size_type max_size() const
        {
            return this->num_elements();
        }

        bool empty() const
        {
            return this->size() == 0;
        }

        size_type const* shape() const
        {
            return this->extent_list_.data();
        }

        index const* strides() const
        {
            return this->stride_list_.data();
        }

        T const* origin() const
        {
            return this->base_ + this->origin_offset_;
        }

        size_type num_elements() const
        {
            return this->num_elements_;
        }

        index const* index_bases() const
        {
            return this->index_base_list_.data();
        }

        template <typename IndexList>
        element const& operator()(IndexList indices) const
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<IndexList>));
            return super_type::access_element(
                boost::type<element const&>()
              , indices
              , this->origin()
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        // Only allow const element access
        const_reference operator[](index idx) const
        {
            return super_type::access(
                boost::type<const_reference>()
              , idx
              , this->origin()
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        // see generate_array_view in base.hpp
        template <int NDims>
        typename const_array_view<NDims>::type
            operator[](
                boost::detail::multi_array::index_gen<
                    NumDims
                  , NDims
                > const& indices
            ) const
        {
            typedef typename const_array_view<NDims>::type return_type;
            return super_type::generate_array_view(
                boost::type<return_type>()
              , indices
              , this->shape()
              , this->strides()
              , this->index_bases()
              , this->origin()
            );
        }

        const_iterator begin() const
        {
            return const_iterator(
                *this->index_bases()
              , this->origin()
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        const_iterator end() const
        {
            return const_iterator(
                *this->index_bases() + (index)*this->shape()
              , this->origin()
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(this->end());
        }

        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(this->begin());
        }

        template <typename OPtr>
        bool
            operator==(
                const_multi_array_view<T,NumDims,OPtr> const& rhs
            ) const
        {
            return std::equal(
                this->extent_list_.begin()
              , this->extent_list_.end()
              , rhs.extent_list_.begin()
            ) ? std::equal(this->begin(), this->end(), rhs.begin()) : false;
        }

        template <typename OPtr>
        bool
            operator<(
                const_multi_array_view<T,NumDims,OPtr> const& rhs
            ) const
        {
            return std::lexicographical_compare(
                this->begin()
              , this->end()
              , rhs.begin()
              , rhs.end()
            );
        }

        template <typename OPtr>
        bool
            operator!=(
                const_multi_array_view<T,NumDims,OPtr> const& rhs
            ) const
        {
            return !(*this == rhs);
        }

        template <typename OPtr>
        bool
            operator>(
                const_multi_array_view<T,NumDims,OPtr> const& rhs
            ) const
        {
            return rhs < *this;
        }

        template <typename OPtr>
        bool
            operator<=(
                const_multi_array_view<T,NumDims,OPtr> const& rhs
            ) const
        {
            return !(*this > rhs);
        }

        template <typename OPtr>
        bool
            operator>=(
                const_multi_array_view<T,NumDims,OPtr> const& rhs
            ) const
        {
            return !(*this < rhs);
        }

#if defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
     public: // should be protected
#else
     protected:
        template <typename, std::size_t>
        friend class multi_array_impl_base;

        template <typename, std::size_t, typename>
        friend class const_multi_array_view;
#endif

        // This constructor is used by
        // multi_array_impl_base::generate_array_view to create strides
        template <typename ExtentList, typename Index>
        const_multi_array_view(
            TPtr base
          , ExtentList const& extents
          , boost::array<Index,NumDims> const& strides
        ) : base_(base), origin_offset_(0)
        {
            this->index_base_list_.assign(0);

            // Get the extents and strides
            boost::detail::multi_array::copy_n(
                extents.begin()
              , NumDims
              , this->extent_list_.begin()
            );
            boost::detail::multi_array::copy_n(
                strides.begin()
              , NumDims
              , this->stride_list_.begin()
            );

            // Calculate the array size
            this->num_elements_ = std::accumulate(
                this->extent_list_.begin()
              , this->extent_list_.end()
              , size_type(1)
              , std::multiplies<size_type>()
            );
        }

        typedef boost::array<size_type,NumDims> size_list;
        typedef boost::array<index,NumDims> index_list;

        TPtr base_;
        index origin_offset_;
        size_type num_elements_;
        size_list extent_list_;
        index_list stride_list_;
        index_list index_base_list_;

     private:
        // const_multi_array_view cannot be assigned to (no deep copies!)
        const_multi_array_view&
            operator=(const_multi_array_view const& other);
    };

    template <typename T, std::size_t NumDims>
    class multi_array_view : public const_multi_array_view<T,NumDims,T*>
    {
        typedef const_multi_array_view<T,NumDims,T*> super_type;

     public: 
        typedef typename super_type::value_type value_type;
        typedef typename super_type::reference reference;
        typedef typename super_type::iterator iterator;
        typedef typename super_type::reverse_iterator reverse_iterator;
        typedef typename super_type::const_reference const_reference;
        typedef typename super_type::const_iterator const_iterator;
        typedef typename super_type::const_reverse_iterator
            const_reverse_iterator;
        typedef typename super_type::element element;
        typedef typename super_type::size_type size_type;
        typedef typename super_type::difference_type difference_type;
        typedef typename super_type::index index;
        typedef typename super_type::extent_range extent_range;

        // template typedefs
        template <std::size_t NDims>
        struct const_array_view
        {
            typedef boost::detail::multi_array::const_multi_array_view<
                T
              , NDims
            > type;
        };

        template <std::size_t NDims>
        struct array_view
        {
            typedef boost::detail::multi_array::multi_array_view<
                T
              , NDims
            > type;
        };

        // Assignment from other ConstMultiArray types.
        template <typename ConstMultiArray>
        multi_array_view& operator=(ConstMultiArray const& other)
        {
            BOOST_CONCEPT_ASSERT((
                boost::multi_array_concepts::ConstMultiArrayConcept<
                    ConstMultiArray
                  , NumDims
                >
            ));

            // make sure the dimensions agree
            BOOST_ASSERT(other.num_dimensions() == this->num_dimensions());
            BOOST_ASSERT(
                std::equal(
                    other.shape()
                  , other.shape() + this->num_dimensions()
                  , this->shape()
                )
            );
            // iterator-based copy
            std::copy(other.begin(), other.end(), this->begin());
            return *this;
        }

        multi_array_view& operator=(multi_array_view const& other)
        {
            if (&other != this)
            {
                // make sure the dimensions agree
                BOOST_ASSERT(
                    other.num_dimensions() == this->num_dimensions()
                );
                BOOST_ASSERT(
                    std::equal(
                        other.shape()
                      , other.shape() + this->num_dimensions()
                      , this->shape()
                    )
                );
                // iterator-based copy
                std::copy(other.begin(), other.end(), this->begin());
            }

            return *this;
        }

        element* origin()
        {
            return this->base_ + this->origin_offset_;
        }

        template <class IndexList>
        element& operator()(IndexList const& indices)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<IndexList>));
            return super_type::access_element(
                boost::type<element&>()
              , indices
              , this->origin()
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        reference operator[](index idx)
        {
            return super_type::access(
                boost::type<reference>()
              , idx
              , this->origin()
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        // see generate_array_view in base.hpp
        template <int NDims>
        typename array_view<NDims>::type 
            operator[](
                boost::detail::multi_array::index_gen<
                    NumDims
                  , NDims
                > const& indices
            )
        {
            typedef typename array_view<NDims>::type return_type;
            return super_type::generate_array_view(
                boost::type<return_type>()
              , indices
              , this->shape()
              , this->strides()
              , this->index_bases()
              , this->origin()
            );
        }

        iterator begin()
        {
            return iterator(
                *this->index_bases()
              , this->origin()
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        iterator end()
        {
            return iterator(
                *this->index_bases() + (index)*this->shape()
              , this->origin()
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        reverse_iterator rbegin()
        {
            return reverse_iterator(this->end());
        }

        reverse_iterator rend()
        {
            return reverse_iterator(this->begin());
        }

        // Using declarations don't seem to work for g++
        // These are the proxies to work around this.

        element const* origin() const
        {
            return super_type::origin();
        }

        template <class IndexList>
        element const& operator()(IndexList const& indices) const
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<IndexList>));
            return super_type::operator()(indices);
        }

        const_reference operator[](index idx) const
        {
            return super_type::operator[](idx);
        }

        // see generate_array_view in base.hpp
        template <int NDims>
        typename const_array_view<NDims>::type
            operator[](
                boost::detail::multi_array::index_gen<
                    NumDims
                  , NDims
                > const& indices
            ) const
        {
            return super_type::operator[](indices);
        }

        const_iterator begin() const
        {
            return super_type::begin();
        }

        const_iterator end() const
        {
            return super_type::end();
        }

        const_reverse_iterator rbegin() const
        {
            return super_type::rbegin();
        }

        const_reverse_iterator rend() const
        {
            return super_type::rend();
        }

#if defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
     public: // should be private
#else
     private:
        template <typename, std::size_t> friend class multi_array_impl_base;
#endif

        // constructor used by multi_array_impl_base::generate_array_view to
        // generate array views
        template <typename ExtentList, typename Index>
        multi_array_view(
            T* base
          , ExtentList const& extents
          , boost::array<Index,NumDims> const& strides
        ) : super_type(base, extents, strides)
        {
        }
    };
}}} // namespace boost::detail::multi_array

namespace boost {

    //
    // traits classes to get array_view types
    //
    template <typename Array, int N>
    class array_view_gen
    {
        typedef typename Array::element element;

     public:
        typedef boost::detail::multi_array::multi_array_view<element,N> type;
    };

    template <typename Array, int N>
    class const_array_view_gen
    {
        typedef typename Array::element element;

     public:
        typedef boost::detail::multi_array::const_multi_array_view<
            element
          , N
        > type;  
    };
} // namespace boost

#endif // BOOST_MULTI_ARRAY_VIEW_RG071301_HPP

