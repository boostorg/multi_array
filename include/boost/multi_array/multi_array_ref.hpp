// Copyright 2002 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

#ifndef BOOST_MULTI_ARRAY_REF_RG071801_HPP
#define BOOST_MULTI_ARRAY_REF_RG071801_HPP

//
// multi_array_ref.hpp - code for creating "views" of array data.
//

#include <boost/multi_array/base.hpp>
#include <boost/multi_array/concept_checks.hpp>
#include <boost/multi_array/iterator.hpp>
#include <boost/multi_array/storage_order.hpp>
#include <boost/multi_array/subarray.hpp>
#include <boost/multi_array/view.hpp>
#include <boost/multi_array/algorithm.hpp>
#include <boost/array.hpp>
#include <boost/concept_check.hpp>
#include <boost/functional.hpp>
#include <boost/limits.hpp>
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

namespace boost {

    template <typename T, std::size_t NumDims, typename TPtr = T const*>
    class const_multi_array_ref
      : public detail::multi_array::multi_array_impl_base<T,NumDims>
    {
        typedef detail::multi_array::multi_array_impl_base<
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
        typedef general_storage_order<NumDims> storage_order_type;

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

#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
        // make const_multi_array_ref a friend of itself
        template <typename, std::size_t, typename>
        friend class const_multi_array_ref;
#endif

        // This ensures that const_multi_array_ref types
        // with different TPtr types can convert to each other
        template <typename OPtr>
        const_multi_array_ref(
            const_multi_array_ref<T,NumDims,OPtr> const& other
        ) : base_(other.base_)
          , storage_(other.storage_)
          , extent_list_(other.extent_list_)
          , stride_list_(other.stride_list_)
          , index_base_list_(other.index_base_list_)
          , origin_offset_(other.origin_offset_)
          , directional_offset_(other.directional_offset_)
          , num_elements_(other.num_elements_)
        {
        }

        template <typename ExtentList>
        const_multi_array_ref(TPtr base, ExtentList const& extents)
          : base_(base), storage_(c_storage_order())
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<ExtentList>));
            this->index_base_list_.assign(0);
            this->init_multi_array_ref(extents.begin());
        }

        template <typename ExtentList>
        const_multi_array_ref(
            TPtr base
          , ExtentList const& extents
          , general_storage_order<NumDims> const& so
        ) : base_(base), storage_(so)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<ExtentList>));
            this->index_base_list_.assign(0);
            this->init_multi_array_ref(extents.begin());
        }

        const_multi_array_ref(
            TPtr base
          , detail::multi_array::extent_gen<NumDims> const& ranges
        ) : base_(base), storage_(c_storage_order())
        {
            this->init_from_extent_gen(ranges);
        }

        const_multi_array_ref(
            TPtr base
          , detail::multi_array::extent_gen<NumDims> const& ranges
          , general_storage_order<NumDims> const& so
        ) : base_(base), storage_(so)
        {
            this->init_from_extent_gen(ranges);
        }

        template <class InputIterator>
        void assign(InputIterator begin, InputIterator end)
        {
            BOOST_CONCEPT_ASSERT((InputIteratorConcept<InputIterator>));
            InputIterator in_iter = begin;
            T* out_iter = this->base_;

            for (
                std::size_t copy_count = 0;
                (in_iter != end) && (copy_count < this->num_elements_);
                ++copy_count
            )
            {
                *out_iter++ = *in_iter++;
            }
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
            this->origin_offset_ = this->calculate_origin_offset(
                this->stride_list_
              , this->extent_list_
              , this->storage_
              , this->index_base_list_
            );
        }

        void reindex(index value) {
            this->index_base_list_.assign(value);
            this->origin_offset_ = this->calculate_origin_offset(
                this->stride_list_
              , this->extent_list_
              , this->storage_
              , this->index_base_list_
            );
        }

        template <typename SizeList>
        void reshape(SizeList const& extents)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<SizeList>));
            BOOST_ASSERT(
                this->num_elements_ == std::accumulate(
                    extents.begin()
                  , extents.end()
                  , size_type(1)
                  , std::multiplies<size_type>()
                )
            );
            std::copy(
                extents.begin()
              , extents.end()
              , this->extent_list_.begin()
            );
            this->compute_strides(
                this->stride_list_
              , this->extent_list_
              , this->storage_
            );
            this->origin_offset_ = this->calculate_origin_offset(
                this->stride_list_
              , this->extent_list_
              , this->storage_
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

        // given reshaping functionality, this is the max possible size.
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

        element const* origin() const
        {
            return this->base_ + this->origin_offset_;
        }

        element const* data() const
        {
            return this->base_;
        }

        size_type num_elements() const
        {
            return this->num_elements_;
        }

        index const* index_bases() const
        {
            return this->index_base_list_.data();
        }

        storage_order_type const& storage_order() const
        {
            return this->storage_;
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
                detail::multi_array::index_gen<NumDims,NDims> const& indices
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
            operator==(const_multi_array_ref<T,NumDims,OPtr> const& rhs) const
        {
            return std::equal(
                this->extent_list_.begin()
              , this->extent_list_.end()
              , rhs.extent_list_.begin()
            ) ? std::equal(this->begin(), this->end(), rhs.begin()) : false;
        }

        template <typename OPtr>
        bool operator<(const_multi_array_ref<T,NumDims,OPtr> const& rhs) const
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
            operator!=(const_multi_array_ref<T,NumDims,OPtr> const& rhs) const
        {
            return !(*this == rhs);
        }

        template <typename OPtr>
        bool operator>(const_multi_array_ref<T,NumDims,OPtr> const& rhs) const
        {
            return rhs < *this;
        }

        template <typename OPtr>
        bool
            operator<=(const_multi_array_ref<T,NumDims,OPtr> const& rhs) const
        {
            return !(*this > rhs);
        }

        template <typename OPtr>
        bool
            operator>=(const_multi_array_ref<T,NumDims,OPtr> const& rhs) const
        {
            return !(*this < rhs);
        }

#if defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
     public:
#else
     protected:
#endif
        typedef boost::array<size_type,NumDims> size_list;
        typedef boost::array<index,NumDims> index_list;

        // This is used by multi_array, which is a subclass of this
        void set_base_ptr(TPtr new_base)
        {
            this->base_ = new_base;
        }

        // This constructor supports multi_array's default constructor
        // and constructors from multi_array_ref, subarray, and array_view
        const_multi_array_ref(
            TPtr base
          , storage_order_type const& so
          , index const* index_bases
          , size_type const* extents
        ) : base_(base)
          , storage_(so)
          , origin_offset_(0)
          , directional_offset_(0)
        {
            // If index_bases or extents is null, then initialize
            // the corresponding private data to zeroed lists.
            if (index_bases)
            {
                boost::detail::multi_array::copy_n(
                    index_bases
                  , NumDims
                  , this->index_base_list_.begin()
                );
            }
            else
            {
                std::fill_n(this->index_base_list_.begin(), NumDims, 0);
            }

            if (extents)
            {
                this->init_multi_array_ref(extents);
            }
            else
            {
                boost::array<index,NumDims> extent_list;
                extent_list.assign(0);
                this->init_multi_array_ref(extent_list.begin());
            }
        }

        TPtr base_;
        storage_order_type storage_;
        size_list extent_list_;
        index_list stride_list_;
        index_list index_base_list_;
        index origin_offset_;
        index directional_offset_;
        size_type num_elements_;

     private:
        // const_multi_array_ref cannot be assigned to (no deep copies!)
        const_multi_array_ref& operator=(const_multi_array_ref const& other);

        void
            init_from_extent_gen(
                detail::multi_array::extent_gen<NumDims> const& ranges
            )
        {
            typedef boost::array<index,NumDims> extent_list;

            // get the index_base values
            std::transform(
                ranges.ranges_.begin()
              , ranges.ranges_.end()
              , this->index_base_list_.begin()
              , boost::mem_fun_ref(&extent_range::start)
            );

            // calculate the extents
            extent_list extents;
            std::transform(
                ranges.ranges_.begin()
              , ranges.ranges_.end()
              , extents.begin()
              , boost::mem_fun_ref(&extent_range::size)
            );
            this->init_multi_array_ref(extents.begin());
        }

#if defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
     public:
#else
     protected:
#endif
        // RG - move me!
        template <typename InputIterator>
        void init_multi_array_ref(InputIterator extents_iter)
        {
            BOOST_CONCEPT_ASSERT((InputIteratorConcept<InputIterator>));
            boost::detail::multi_array::copy_n(
                extents_iter
              , this->num_dimensions()
              , this->extent_list_.begin()
            );

            // Calculate the array size
            this->num_elements_ = std::accumulate(
                this->extent_list_.begin()
              , this->extent_list_.end()
              , size_type(1)
              , std::multiplies<size_type>()
            );

            this->compute_strides(
                this->stride_list_
              , this->extent_list_
              , this->storage_
            );
            this->origin_offset_ = this->calculate_origin_offset(
                 this->stride_list_
               , this->extent_list_
               , this->storage_
               , this->index_base_list_
            );
            this->directional_offset_ =
            this->calculate_descending_dimension_offset(
                this->stride_list_
              , this->extent_list_
              , this->storage_
            );
        }
    };

    template <typename T, std::size_t NumDims>
    class multi_array_ref : public const_multi_array_ref<T,NumDims,T*>
    {
        typedef const_multi_array_ref<T,NumDims,T*> super_type;

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

        typedef typename super_type::storage_order_type storage_order_type;
        typedef typename super_type::index_list index_list;
        typedef typename super_type::size_list size_list;

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

        template <typename ExtentList>
        multi_array_ref(T* base, ExtentList const& extents)
          : super_type(base, extents)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<ExtentList>));
        }

        template <typename ExtentList>
        multi_array_ref(
            T* base
          , ExtentList const& extents
          , general_storage_order<NumDims> const& so
        ) : super_type(base, extents, so)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<ExtentList>));
        }

        multi_array_ref(
            T* base
          , detail::multi_array::extent_gen<NumDims> const& ranges
        ) : super_type(base, ranges)
        {
        }

        multi_array_ref(
            T* base
          , detail::multi_array::extent_gen<NumDims> const& ranges
          , general_storage_order<NumDims> const& so
        ) : super_type(base, ranges, so)
        {
        }

        // Assignment from other ConstMultiArray types.
        template <typename ConstMultiArray>
        multi_array_ref& operator=(ConstMultiArray const& other)
        {
            BOOST_CONCEPT_ASSERT((
                multi_array_concepts::ConstMultiArrayConcept<
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

        multi_array_ref& operator=(multi_array_ref const& other)
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
            return super_type::base_ + super_type::origin_offset_;
        }

        element* data()
        {
            return super_type::base_;
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

        // See note attached to generate_array_view in base.hpp
        template <int NDims>
        typename array_view<NDims>::type 
            operator[](
                detail::multi_array::index_gen<NumDims,NDims> const& indices
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

        // rbegin() and rend() written naively to thwart MSVC ICE.
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

        // Using declarations don't seem to work for g++
        // These are the proxies to work around this.

        element const* origin() const
        {
            return super_type::origin();
        }

        element const* data() const
        {
            return super_type::data();
        }

        template <typename IndexList>
        element const& operator()(IndexList const& indices) const
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<IndexList>));
            return super_type::operator()(indices);
        }

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

        // See note attached to generate_array_view in base.hpp
        template <int NDims>
        typename const_array_view<NDims>::type 
            operator[](
                detail::multi_array::index_gen<NumDims,NDims> const& indices
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

     protected:
        // This is only supplied to support multi_array's default constructor
        multi_array_ref(
            T* base
          , storage_order_type const& so
          , index const* index_bases
          , size_type const* extents
        ) : super_type(base, so, index_bases, extents)
        {
        }
    };
} // namespace boost

#endif // BOOST_MULTI_ARRAY_REF_RG071801_HPP

