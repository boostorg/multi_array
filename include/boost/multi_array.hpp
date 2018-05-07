// Copyright 2002 The Trustees of Indiana University.

// Copyright 2014 Glen Fernandes // C++11 allocator model support
// glenfe at live dot com

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

#ifndef BOOST_MULTI_ARRAY_RG071801_HPP
#define BOOST_MULTI_ARRAY_RG071801_HPP

//
// multi_array.hpp - contains the multi_array class template
// declaration and definition
//

#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 406)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <boost/multi_array/base.hpp>
#include <boost/multi_array/copy_array.hpp>
#include <boost/multi_array/iterator.hpp>
#include <boost/multi_array/subarray.hpp>
#include <boost/multi_array/multi_array_ref.hpp>
#include <boost/multi_array/algorithm.hpp>
#include <boost/multi_array/allocator_fill.hpp>
#include <boost/array.hpp>
#include <boost/container/allocator_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/tti/detail/dnullptr.hpp>
#include <boost/concept_check.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <vector>

namespace boost { namespace detail { namespace multi_array {

    struct populate_index_ranges
    {
        multi_array_types::index_range
            // RG: underscore on extent_ to stifle strange MSVC warning.
            operator()(
                multi_array_types::index base
              , multi_array_types::size_type extent_
            )
        {
            return multi_array_types::index_range(base, base + extent_);
        }
    };

#if defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
    //
    // Compilers that don't support partial ordering may need help to
    // disambiguate multi_array's templated constructors.  Even vc6/7 are
    // capable of some limited SFINAE, so we take the most-general version
    // out of the overload set with disable_multi_array_impl.
    //
    template <typename T, std::size_t NumDims, typename TPtr>
    char is_multi_array_impl_help(const_multi_array_view<T,NumDims,TPtr>&);

    template <typename T, std::size_t NumDims, typename TPtr>
    char is_multi_array_impl_help(const_sub_array<T,NumDims,TPtr>&);

    template <typename T, std::size_t NumDims, typename TPtr>
    char is_multi_array_impl_help(const_multi_array_ref<T,NumDims,TPtr>&);

    char ( &is_multi_array_impl_help(...) )[2];

    template <typename T>
    struct is_multi_array_impl
    {
        static T x;
        BOOST_STATIC_CONSTANT(
            bool, value = (sizeof((is_multi_array_impl_help)(x)) == 1)
        );
        typedef boost::mpl::bool_<value> type;
    };

    template <bool multi_array = false>
    struct disable_multi_array_impl_impl
    {
        typedef int type;
    };

    template <>
    struct disable_multi_array_impl_impl<true>
    {
        // forming a pointer to a reference triggers SFINAE
        typedef int& type; 
    };

    template <typename T>
    struct disable_multi_array_impl
      : disable_multi_array_impl_impl<is_multi_array_impl<T>::value>
    {
    };

    template <>
    struct disable_multi_array_impl<int>
    {
        typedef int type;
    };
#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING
}}} // namespace boost::detail::multi_array

namespace boost {

    template <typename T, std::size_t NumDims, typename Allocator>
    class multi_array : private Allocator, public multi_array_ref<T,NumDims>
    {
        typedef Allocator allocator_type;
        typedef multi_array_ref<T,NumDims> super_type;

     public:
        typedef typename super_type::value_type value_type;
        typedef typename super_type::reference reference;
        typedef typename super_type::const_reference const_reference;
        typedef typename super_type::iterator iterator;
        typedef typename super_type::const_iterator const_iterator;
        typedef typename super_type::reverse_iterator reverse_iterator;
        typedef typename super_type::const_reverse_iterator
            const_reverse_iterator;
        typedef typename super_type::element element;
        typedef typename super_type::size_type size_type;
        typedef typename super_type::difference_type difference_type;
        typedef typename super_type::index index;
        typedef typename super_type::extent_range extent_range;

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

        multi_array(Allocator const& alloc = Allocator())
          : Allocator(alloc)
          , super_type(
                (T*)initial_base_
              , c_storage_order()
              , /*index_bases = */0
              , /*extents = */0
            )
        {
            this->allocate_space();
        }

        template <typename ExtentList>
        explicit multi_array(
            ExtentList const& extents
          , Allocator const& alloc = Allocator()
#if defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
          , typename boost::mpl::if_<
                boost::detail::multi_array::is_multi_array_impl<ExtentList>
              , int&
              , int
            >::type* = BOOST_TTI_DETAIL_NULLPTR
#endif
        ) : Allocator(alloc), super_type((T*)initial_base_, extents)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<ExtentList>));
            this->allocate_space();
        }

        template <typename ExtentList>
        multi_array(
            ExtentList const& extents
          , general_storage_order<NumDims> const& so
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc), super_type((T*)initial_base_, extents, so)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<ExtentList>));
            this->allocate_space();
        }

        explicit multi_array(
            boost::detail::multi_array::extent_gen<NumDims> const& ranges
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc), super_type((T*)initial_base_, ranges)
        {
            this->allocate_space();
        }

        multi_array(
            boost::detail::multi_array::extent_gen<NumDims> const& ranges
          , general_storage_order<NumDims> const& so
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc), super_type((T*)initial_base_, ranges, so)
        {
            this->allocate_space();
        }

        multi_array(multi_array const& rhs)
          : Allocator(static_cast<Allocator const&>(rhs)), super_type(rhs)
        {
            this->allocate_space();
            boost::detail::multi_array::copy_n(
                rhs.base_
              , rhs.num_elements()
              , this->base_
            );
        }

        //
        // A multi_array is constructible from any multi_array_ref, subarray,
        // or array_view object.  The following constructors ensure that.
        //

        // Due to limited support for partial template ordering, MSVC 6&7
        // confuse the following with the most basic ExtentList constructor.
#if defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
        // More limited support for MSVC
        explicit multi_array(
            const_multi_array_ref<T,NumDims> const& rhs
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, c_storage_order(), rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            // Warning! storage order may change,
            // hence the following copy technique.
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            const_multi_array_ref<T,NumDims> const& rhs
          , general_storage_order<NumDims> const& so
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            // Warning! storage order may change,
            // hence the following copy technique.
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            boost::detail::multi_array::const_sub_array<T,NumDims> const& rhs
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, c_storage_order(), rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            boost::detail::multi_array::const_sub_array<T,NumDims> const& rhs
          , general_storage_order<NumDims> const& so
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            boost::detail::multi_array
            ::const_multi_array_view<T,NumDims> const& rhs
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, c_storage_order(), rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            boost::detail::multi_array
            ::const_multi_array_view<T,NumDims> const& rhs
          , general_storage_order<NumDims> const& so
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }
#else // !defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
        template <typename OPtr>
        multi_array(
            const_multi_array_ref<T,NumDims,OPtr> const& rhs
          , general_storage_order<NumDims> const& so = c_storage_order()
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            // Warning! storage order may change,
            // hence the following copy technique.
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        template <typename OPtr>
        multi_array(
            boost::detail::multi_array
            ::const_sub_array<T,NumDims,OPtr> const& rhs
          , general_storage_order<NumDims> const& so = c_storage_order()
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        template <typename OPtr>
        multi_array(
            boost::detail::multi_array
            ::const_multi_array_view<T,NumDims,OPtr> const& rhs
          , general_storage_order<NumDims> const& so = c_storage_order()
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }
#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING

        // These constructors are necessary
        // because of more exact template matches.
        multi_array(
            multi_array_ref<T,NumDims> const& rhs
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, c_storage_order(), rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            // Warning! storage order may change,
            // hence the following copy technique.
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            multi_array_ref<T,NumDims> const& rhs
          , general_storage_order<NumDims> const& so
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            // Warning! storage order may change,
            // hence the following copy technique.
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            boost::detail::multi_array::sub_array<T,NumDims> const& rhs
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, c_storage_order(), rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            boost::detail::multi_array::sub_array<T,NumDims> const& rhs
          , general_storage_order<NumDims> const& so
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape()) 
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            boost::detail::multi_array::multi_array_view<T,NumDims> const& rhs
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, c_storage_order(), rhs.index_bases(), rhs.shape())
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        multi_array(
            boost::detail::multi_array::multi_array_view<T,NumDims> const& rhs
          , general_storage_order<NumDims> const& so
          , Allocator const& alloc = Allocator()
        ) : Allocator(alloc)
          , super_type(0, so, rhs.index_bases(), rhs.shape()) 
        {
            this->allocate_space();
            std::copy(rhs.begin(), rhs.end(), this->begin());
        }

        // Since assignment is a deep copy, multi_array_ref
        // contains all the necessary code.
        template <typename ConstMultiArray>
        multi_array& operator=(ConstMultiArray const& other)
        {
            Allocator::operator=(
                static_cast<
                    typename ConstMultiArray::allocator_type const&
                >(other)
            );
            super_type::operator=(
                static_cast<
                    typename ConstMultiArray::super_type const&
                >(other)
            );
            return *this;
        }

        multi_array& operator=(multi_array const& other)
        {
            if (&other != this)
            {
                Allocator::operator=(static_cast<Allocator const&>(other));
                super_type::operator=(static_cast<super_type const&>(other));
            }

            return *this;
        }

        template <typename ExtentList>
        multi_array& resize(ExtentList const& extents)
        {
            BOOST_CONCEPT_ASSERT((CollectionConcept<ExtentList>));
            typedef boost::detail::multi_array::extent_gen<NumDims> gen_type;
            gen_type ranges;

            for (int i = 0; i != NumDims; ++i)
            {
                typedef typename gen_type::range range_type;
                ranges.ranges_[i] = range_type(0, extents[i]);
            }

            return this->resize(ranges);
        }

        multi_array&
            resize(
                boost::detail::multi_array::extent_gen<NumDims> const& ranges
            )
        {
            // build a multi_array with the specs given
            multi_array new_array(ranges, this->storage_order());

            // build a view of tmp with the minimum extents

            // Get the minimum extents of the arrays.
            boost::array<size_type,NumDims> min_extents;

            size_type const& (
                *min
            )(size_type const&, size_type const&) = std::min;
            std::transform(
                new_array.extent_list_.begin()
              , new_array.extent_list_.end()
              , this->extent_list_.begin()
              , min_extents.begin()
              , min
            );

            // typedef boost::array<index,NumDims> index_list;
            // Build index_gen objects to create views with the same shape

            // these need to be separate to handle non-zero index bases
            typedef boost::detail::multi_array
            ::index_gen<NumDims,NumDims> index_gen;
            index_gen old_idxes;
            index_gen new_idxes;

            std::transform(
                new_array.index_base_list_.begin()
              , new_array.index_base_list_.end()
              , min_extents.begin()
              , new_idxes.ranges_.begin()
              , detail::multi_array::populate_index_ranges()
            );
            std::transform(
                this->index_base_list_.begin()
              , this->index_base_list_.end()
              , min_extents.begin()
              , old_idxes.ranges_.begin()
              , detail::multi_array::populate_index_ranges()
            );

            // Build same-shape views of the two arrays
            typename multi_array::BOOST_NESTED_TEMPLATE array_view<
                NumDims
            >::type view_old = (*this)[old_idxes];
            typename multi_array::BOOST_NESTED_TEMPLATE array_view<
                NumDims
            >::type view_new = new_array[new_idxes];

            // Set the right portion of the new array
            view_new = view_old;

            using std::swap;
            // Swap the internals of these arrays.
            swap(this->super_type::base_, new_array.super_type::base_);
            swap(this->storage_, new_array.storage_);
            swap(this->extent_list_, new_array.extent_list_);
            swap(this->stride_list_, new_array.stride_list_);
            swap(this->index_base_list_, new_array.index_base_list_);
            swap(this->origin_offset_, new_array.origin_offset_);
            swap(this->directional_offset_, new_array.directional_offset_);
            swap(this->num_elements_, new_array.num_elements_);
            swap(this->base_, new_array.base_);
            swap(this->allocated_elements_, new_array.allocated_elements_);

            return *this;
        }

        bool operator==(multi_array<T,NumDims,Allocator> const& rhs) const
        {
            return (
                static_cast<Allocator const&>(*this) == static_cast<
                    Allocator const&
                >(rhs)
            ) && (
                static_cast<super_type const&>(*this) == static_cast<
                    super_type const&
                >(rhs)
            );
        }

        bool operator!=(multi_array<T,NumDims,Allocator> const& rhs) const
        {
            return (
                static_cast<Allocator const&>(*this) != static_cast<
                    Allocator const&
                >(rhs)
            ) || (
                static_cast<super_type const&>(*this) != static_cast<
                    super_type const&
                >(rhs)
            );
        }

        ~multi_array()
        {
            this->deallocate_space();
        }

     private:
        void allocate_space()
        {
            Allocator& alloc = static_cast<Allocator&>(*this);
            this->base_ = alloc.allocate(this->num_elements());
            this->set_base_ptr(this->base_);
            this->allocated_elements_ = this->num_elements();
            boost::detail::multi_array::allocator_fill(
                alloc
              , this->base_
              , this->allocated_elements_
            );
        }

        void deallocate_space()
        {
            if (this->base_)
            {
                Allocator& alloc = static_cast<Allocator&>(*this);

                for (
                    T* i = this->base_;
                    i != this->base_ + this->allocated_elements_;
                    ++i
                )
                {
                    boost::container::allocator_traits<
                        Allocator
                    >::destroy(alloc, i);
                }

                alloc.deallocate(
                    this->base_
                  , this->allocated_elements_
                );
            }
        }

        typedef boost::array<size_type,NumDims> size_list;
        typedef boost::array<index,NumDims> index_list;

        T* base_;
        size_type allocated_elements_;
        enum {initial_base_ = 0};
    };
} // namespace boost

#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 406)
#pragma GCC diagnostic pop
#endif

#endif // BOOST_MULTI_ARRAY_RG071801_HPP

