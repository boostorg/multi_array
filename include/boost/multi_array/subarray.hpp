// Copyright 2002 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

#ifndef SUBARRAY_RG071801_HPP
#define SUBARRAY_RG071801_HPP

//
// subarray.hpp - used to implement standard operator[] on multi_arrays
//

#include <boost/multi_array/base.hpp>
#include <boost/multi_array/concept_checks.hpp>
#include <boost/limits.hpp>
#include <boost/type.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>

namespace boost { namespace detail { namespace multi_array {

    //
    // const_sub_array
    //    multi_array's proxy class to allow multiple overloads of operator[]
    //    in order to provide a clean multi-dimensional array interface.
    //
    template <typename T, std::size_t NumDims, typename TPtr>
    class const_sub_array
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

        // Allow default copy constructor as well.

        template <typename OPtr>
        const_sub_array(const_sub_array<T,NumDims,OPtr> const& rhs)
          : base_(rhs.base_)
          , extents_(rhs.extents_)
          , strides_(rhs.strides_)
          , index_base_(rhs.index_base_)
        {
        }

        // const_sub_array always returns const types,
        // regardless of its own constness.
        const_reference operator[](index idx) const
        {
            return super_type::access(
                boost::type<const_reference>()
              , idx
              , this->base_
              , this->shape()
              , this->strides()
              , this->index_bases()
            );
        }

        template <typename IndexList>
        element const& operator()(IndexList const& indices) const
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
              , this->base_
            );
        }

        template <typename OPtr>
        bool operator<(const_sub_array<T,NumDims,OPtr> const& rhs) const
        {
            return std::lexicographical_compare(
                this->begin()
              , this->end()
              , rhs.begin()
              , rhs.end()
            );
        }

        template <typename OPtr>
        bool operator==(const_sub_array<T,NumDims,OPtr> const& rhs) const
        {
            return std::equal(
                this->shape()
              , this->shape() + this->num_dimensions()
              , rhs.shape()
            ) ? std::equal(this->begin(), this->end(), rhs.begin()) : false;
        }

        template <typename OPtr>
        bool operator!=(const_sub_array<T,NumDims,OPtr> const& rhs) const
        {
            return !(*this == rhs);
        }

        template <typename OPtr>
        bool operator>(const_sub_array<T,NumDims,OPtr> const& rhs) const
        {
            return rhs < *this;
        }

        template <typename OPtr>
        bool operator<=(const_sub_array<T,NumDims,OPtr> const& rhs) const
        {
            return !(*this > rhs);
        }

        template <typename OPtr>
        bool operator>=(const_sub_array<T,NumDims,OPtr> const& rhs) const
        {
            return !(*this < rhs);
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

        TPtr origin() const
        {
            return this->base_;
        }

        size_type size() const
        {
            return this->extents_[0];
        }

        size_type max_size() const
        {
            return this->num_elements();
        }

        bool empty() const
        {
            return this->size() == 0;
        }

        size_type num_dimensions() const
        {
            return NumDims;
        }

        size_type const* shape() const
        {
            return this->extents_;
        }

        index const* strides() const
        {
            return this->strides_;
        }

        index const* index_bases() const
        {
            return this->index_base_;
        }

        size_type num_elements() const
        {
            return std::accumulate(
                this->shape()
              , this->shape() + this->num_dimensions()
              , size_type(1)
              , std::multiplies<size_type>()
            );
        }

#if defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
     public:  // Should be protected
#else
     protected:
        template <typename, std::size_t>
        friend class value_accessor_n;

        template <typename, std::size_t, typename>
        friend class const_sub_array;
#endif

        const_sub_array(
            TPtr base
          , size_type const* extents
          , index const* strides
          , index const* index_base
        ) : base_(base)
          , extents_(extents)
          , strides_(strides)
          , index_base_(index_base)
        {
        }

        TPtr base_;
        size_type const* extents_;
        index const* strides_;
        index const* index_base_;

     private:
        // const_sub_array cannot be assigned to (no deep copies!)
        const_sub_array& operator=(const_sub_array const&);
    };

    //
    // sub_array
    //    multi_array's proxy class to allow multiple overloads of operator[]
    //    in order to provide a clean multi-dimensional array interface.
    template <typename T, std::size_t NumDims>
    class sub_array : public const_sub_array<T,NumDims,T*>
    {
        typedef const_sub_array<T,NumDims,T*> super_type;

     public: 
        typedef typename super_type::element element;
        typedef typename super_type::reference reference;
        typedef typename super_type::index index;
        typedef typename super_type::size_type size_type;
        typedef typename super_type::iterator iterator;
        typedef typename super_type::reverse_iterator reverse_iterator;
        typedef typename super_type::const_reference const_reference;
        typedef typename super_type::const_iterator const_iterator;
        typedef typename super_type::const_reverse_iterator
            const_reverse_iterator;

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
        sub_array& operator=(ConstMultiArray const& other)
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

        sub_array& operator=(sub_array const& other)
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

        T* origin()
        {
            return this->base_;
        }

        T const* origin() const
        {
            return this->base_;
        }

        reference operator[](index idx)
        {
            return super_type::access(
                boost::type<reference>()
              , idx
              , this->base_
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

        // RG - rbegin() and rend() written naively to thwart MSVC ICE.
        reverse_iterator rbegin()
        {
            reverse_iterator ri(this->end());
            return ri;
        }

        reverse_iterator rend()
        {
            reverse_iterator ri(this->begin());
            return ri;
        }

        //
        // proxies
        //

        template <typename IndexList>
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
        template <typename, std::size_t>
        friend class value_accessor_n;

#endif
        sub_array(
            T* base
          , size_type const* extents
          , index const* strides
          , index const* index_base
        ) : super_type(base, extents, strides, index_base)
        {
        }
    };
}}} // namespace boost::detail::multi_array

namespace boost {

    //
    // traits classes to get sub_array types
    //
    template <typename Array, int N>
    class subarray_gen
    {
        typedef typename Array::element element;

     public:
        typedef boost::detail::multi_array::sub_array<element,N> type;
    };

    template <typename Array, int N>
    class const_subarray_gen
    {
        typedef typename Array::element element;

     public:
        typedef boost::detail::multi_array::const_sub_array<element,N> type;
    };
} // namespace boost
  
#endif // SUBARRAY_RG071801_HPP

