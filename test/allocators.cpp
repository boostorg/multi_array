/*
 Copyright (c) 2014 Glen Joseph Fernandes
 glenfe at live dot com

 Distributed under the Boost Software License,
 Version 1.0. (See accompanying file LICENSE_1_0.txt
 or copy at http://boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#include <boost/test/minimal.hpp>
#include <boost/multi_array.hpp>
#include <memory>
#include <cstddef>

template<class T>
class stateful
    : public std::allocator<T> {
public:
    template<class U>
    struct rebind {
        typedef stateful<U> other;
    };

    stateful(int value)
        : state(value) {
    }

    template<class U>
    stateful(const stateful<U>& other)
        : state(other.state) {
    }

    int state;
};

template<class T1, class T2>
bool operator==(const stateful<T1>& a,
    const stateful<T2>& b) {
    return a.state == b.state;
}

template<class T1, class T2>
bool operator!=(const stateful<T1>& a,
    const stateful<T2>& b) {
    return !(a == b);
}

#if !defined(BOOST_NO_CXX11_ALLOCATOR)
template<class T>
class minimal {
public:
    typedef T value_type;

#if !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
    minimal() = default;
#else
    minimal() {
    }
#endif

    template<class U>
    minimal(const minimal<T>&) {
    }

    T* allocate(std::size_t size) {
        void* p = ::operator new(size * sizeof(T));
        return static_cast<T*>(p);
    }

    void deallocate(T* ptr, std::size_t) {
        void* p = ptr;
        ::operator delete(p);
    }
};

template<class T1, class T2>
bool operator==(const minimal<T1>& a,
    const minimal<T2>& b) {
    return true;
}

template<class T1, class T2>
bool operator!=(const minimal<T1>& a,
    const minimal<T2>& b) {
    return !(a == b);
}
#endif

void check_shape(const double&, std::size_t*, int*, unsigned int)
{
}

template<class T>
void check_shape(const T& data, std::size_t* sizes, int* strides,
    unsigned int elements)
{
  BOOST_CHECK(data.num_elements() == elements);
  BOOST_CHECK(data.size() == *sizes);
  BOOST_CHECK(std::equal(sizes, sizes + data.num_dimensions(),
      data.shape()));
  BOOST_CHECK(std::equal(strides, strides + data.num_dimensions(),
      data.strides()));
  check_shape(data[0], ++sizes, ++strides, elements / data.size());
}

int test_main(int, char*[])
{
    {
        int strides[] = { 9, 3, 1 };
        boost::array<boost::multi_array<double,
            3>::size_type, 3> sizes = { { 3, 3, 3 } };
        boost::multi_array<double, 3,
            stateful<double> >::size_type num_elements = 27;
        boost::multi_array<double, 3,
            stateful<double> >::extent_gen extents;
        stateful<double> alloc(5);
        boost::multi_array<double, 3,
            stateful<double> > data(extents[3][3][3], alloc);
        check_shape(data, &sizes[0], strides, num_elements);
    }

#if !defined(BOOST_NO_CXX11_ALLOCATOR)
    {
        int strides[] = { 9, 3, 1 };
        boost::array<boost::multi_array<double,
            3>::size_type, 3> sizes = { { 3, 3, 3 } };
        boost::multi_array<double, 3,
            minimal<double> >::size_type num_elements = 27;
        boost::multi_array<double, 3,
            minimal<double> >::extent_gen extents;
        boost::multi_array<double, 3,
            minimal<double> > data(extents[3][3][3]);
        check_shape(data, &sizes[0], strides, num_elements);
    }
#endif

    return boost::exit_success;
}
