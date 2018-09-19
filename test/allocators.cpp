// Copyright 2014 Glen Joseph Fernandes.
// glenfe at live dot com

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

//
// allocators.cpp - testing the code for allocators
//

#include <boost/multi_array.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config.hpp>
#include <memory>
#include <cstddef>

template <typename T>
class stateful : public std::allocator<T>
{
 public:
    template <typename U>
    struct rebind
    {
        typedef stateful<U> other;
    };

    stateful(int value) : state(value)
    {
    }

    template <typename U>
    stateful(stateful<U> const& other) : state(other.state)
    {
    }

    int state;
};

template <typename T1, typename T2>
bool operator==(stateful<T1> const& a, stateful<T2> const& b)
{
    return a.state == b.state;
}

template <typename T1, typename T2>
bool operator!=(stateful<T1> const& a, stateful<T2> const& b)
{
    return !(a == b);
}

#if !defined(BOOST_NO_CXX11_ALLOCATOR)
template <typename T>
class minimal
{
 public:
    typedef T value_type;

#if defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
    minimal()
    {
    }
#else
    minimal() = default;
#endif

    template <typename U>
    minimal(minimal<U> const&)
    {
    }

    T* allocate(std::size_t size)
    {
        void* p = ::operator new(size * sizeof(T));
        return static_cast<T*>(p);
    }

    void deallocate(T* ptr, std::size_t)
    {
        void* p = ptr;
        ::operator delete(p);
    }
};

template <typename T1, typename T2>
bool operator==(minimal<T1> const& a, minimal<T2> const& b)
{
    return true;
}

template <typename T1, typename T2>
bool operator!=(minimal<T1> const& a, minimal<T2> const& b)
{
    return !(a == b);
}
#endif // BOOST_NO_CXX11_ALLOCATOR

void check_shape(double const&, std::size_t*, int*, unsigned long)
{
}

template <typename T>
void
    check_shape(
        T const& data
      , std::size_t* sizes
      , int* strides
      , unsigned long elements
    )
{
    BOOST_TEST(data.num_elements() == elements);
    BOOST_TEST(data.size() == *sizes);
    BOOST_TEST((
        std::equal(sizes, sizes + data.num_dimensions(), data.shape())
    ));
    BOOST_TEST((
        std::equal(strides, strides + data.num_dimensions(), data.strides())
    ));
    check_shape(data[0], ++sizes, ++strides, elements / data.size());
}

int main(int, char*[])
{
    {
        int strides[] = {9, 3, 1};
        boost::array<
            boost::multi_array<double,3>::size_type
          , 3
        > sizes = {{3, 3, 3}};
        boost::multi_array<
            double
          , 3
          , stateful<double>
        >::size_type num_elements = 27;
        boost::multi_array<
            double
          , 3
          , stateful<double>
        >::extent_gen extents;
        stateful<double> alloc(5);
        boost::multi_array<
            double
          , 3
          , stateful<double>
        > data(extents[3][3][3], alloc);
        check_shape(data, &sizes[0], strides, num_elements);
    }

#if !defined(BOOST_NO_CXX11_ALLOCATOR)
    {
        int strides[] = {9, 3, 1};
        boost::array<
            boost::multi_array<double,3>::size_type
          , 3
        > sizes = {{3, 3, 3}};
        boost::multi_array<
            double
          , 3
          , minimal<double>
        >::size_type num_elements = 27;
        boost::multi_array<
            double
          , 3
          , minimal<double>
        >::extent_gen extents;
        boost::multi_array<
            double
          , 3
          , minimal<double>
        > data(extents[3][3][3]);
        check_shape(data, &sizes[0], strides, num_elements);
    }
#endif // BOOST_NO_CXX11_ALLOCATOR

    return boost::report_errors();
}

