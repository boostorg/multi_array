// Copyright 2002 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

#ifndef BOOST_INDEX_RANGE_RG071801_HPP
#define BOOST_INDEX_RANGE_RG071801_HPP

#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <utility>

// For representing intervals, also with stride.
// A degenerate range is a range with one element.

// Thanks to Doug Gregor for the really cool idea of using the
// comparison operators to express various interval types!

// Internally, we represent the interval as half-open.

namespace boost { namespace detail { namespace multi_array {

    template <typename Index, typename SizeType>
    class index_range
    {
        typedef index_range<Index,SizeType> this_type;

     public:
        typedef Index index;
        typedef SizeType size_type;

     private:
        static index from_start()
        {
            return (std::numeric_limits<index>::min)();
        }

        static index to_end()
        {
            return (std::numeric_limits<index>::max)();
        }

     public:
        index_range()
        {
            this->start_ = this_type::from_start();
            this->finish_ = this_type::to_end();
            this->stride_ = 1;
            this->degenerate_ = false;
        }

        explicit index_range(index pos)
        {
            this->start_ = pos;
            this->finish_ = pos + 1;
            this->stride_ = 1;
            this->degenerate_ = true;
        }

        index_range(index start, index finish, index stride = 1)
          : start_(start)
          , finish_(finish)
          , stride_(stride)
          , degenerate_(false)
        {
        }

        // These are for chaining assignments to an index_range
        index_range& start(index s)
        {
            this->start_ = s;
            this->degenerate_ = false;
            return *this;
        }

        index_range& finish(index f)
        {
            this->finish_ = f;
            this->degenerate_ = false;
            return *this;
        }

        index_range& stride(index s)
        {
            this->stride_ = s;
            return *this;
        }

        index start() const
        {
            return this->start_; 
        }

        index get_start(index low_index_range = this_type::from_start()) const
        {
            return (
                this->start_ == this_type::from_start()
            ) ? low_index_range : this->start_;
        }

        index finish() const
        {
            return this->finish_;
        }

        index get_finish(index high_index_range = this_type::to_end()) const
        {
            return (
                this->finish_ == this_type::to_end()
            ) ? high_index_range : this->finish_;
        }

        index stride() const
        {
            return this->stride_;
        }

        size_type size(index idx) const
        {
            return (
                (this->start_ == this_type::from_start()) || (
                    this->finish_ == this_type::to_end()
                )
            ) ? idx : ((this->finish_ - this->start_) / this->stride_);
        }

        void set_index_range(index start, index finish, index stride = 1)
        {
            this->start_ = start;
            this->finish_ = finish;
            this->stride_ = stride;
        }

        static index_range all()
        {
            return index_range(
                this_type::from_start()
              , this_type::to_end()
              , 1
            );
        }

        bool is_degenerate() const
        {
            return this->degenerate_;
        }

        index_range operator-(index shift) const
        {
            return index_range(
                this->start_ - shift
              , this->finish_ - shift
              , this->stride_
            );
        }

        index_range operator+(index shift) const
        {
            return index_range(
                this->start_ + shift
              , this->finish_ + shift
              , this->stride_
            );
        }

        index operator[](index i) const
        {
            return this->start_ + i * this->stride_;
        }

        index operator()(index i) const
        {
            return this->start_ + i * this->stride_;
        }

        // add conversion to std::slice?

     public:
        index start_, finish_, stride_;
        bool degenerate_;
    };

    // Express open and closed interval end-points using the comparison
    // operators.

    // left closed
    template <typename Index, typename SizeType>
    inline index_range<Index,SizeType>
    operator<=(Index s, index_range<Index,SizeType> const& r)
    {
        return index_range<Index,SizeType>(s, r.finish(), r.stride());
    }

    // left open
    template <typename Index, typename SizeType>
    inline index_range<Index,SizeType>
    operator<(Index s, index_range<Index,SizeType> const& r)
    {
        return index_range<Index,SizeType>(s + 1, r.finish(), r.stride());
    }

    // right open
    template <typename Index, typename SizeType>
    inline index_range<Index,SizeType>
    operator<(index_range<Index,SizeType> const& r, Index f)
    {
        return index_range<Index,SizeType>(r.start(), f, r.stride());
    }

    // right closed
    template <typename Index, typename SizeType>
    inline index_range<Index,SizeType>
    operator<=(index_range<Index,SizeType> const& r, Index f)
    {
        return index_range<Index,SizeType>(r.start(), f + 1, r.stride());
    }
}}} // namespace boost::detail::multi_array

#endif // BOOST_INDEX_RANGE_RG071801_HPP

