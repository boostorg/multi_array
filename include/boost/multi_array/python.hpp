// Copyright Daniel Wallin 2006. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MULTI_ARRAY_PYTHON_060213_HPP
# define BOOST_MULTI_ARRAY_PYTHON_060213_HPP

# include <boost/multi_array.hpp>

# include <boost/mpl/bool.hpp>
# include <boost/mpl/prior.hpp>
# include <boost/mpl/equal_to.hpp>
# include <boost/mpl/long.hpp>
# include <boost/mpl/for_each.hpp>
# include <boost/mpl/range_c.hpp>

# include <boost/python/class.hpp>
# include <boost/python/def.hpp>
# include <boost/python/list.hpp>
# include <boost/python/tuple.hpp>
# include <boost/python/iterator.hpp>
# include <boost/python/make_constructor.hpp>
# include <boost/python/operators.hpp>

# include <boost/lexical_cast.hpp>

namespace boost { namespace multi_array_python {

namespace aux
{

  template <class T>
  std::string tostring(T const& x)
  {
      return boost::lexical_cast<std::string>(x);
  }

  template <class Iter>
  python::list as_list(Iter first, Iter last)
  {
      python::list result;

      for (; first != last; ++first)
          result.append(*first);

      return result;
  }

  //
  // This class template wraps a multi_array iterator, changing
  // it's value_type to be equal to it's reference type.
  //
  // The reason for this is that multi_array::iterator has a proxy
  // type as it's reference type, but the value_type is multi_array.
  //
  // The way BPL exposes iterators this means the contents of the
  // array will be copied every time the iterator is dereferenced.
  //
  // This makes sure the proxies are copied.
  //
  template <class Base>
  struct iterator_wrapper;

  template <class Base>
  struct iterator_wrapper_base
  {
      typedef boost::iterator_adaptor<
          iterator_wrapper<Base>
        , Base
        , typename boost::iterator_reference<Base>::type
        , boost::use_default
        , typename boost::iterator_reference<Base>::type
      > type;
  };

  template <class Base>
  struct iterator_wrapper
    : iterator_wrapper_base<Base>::type
  {
      typedef typename iterator_wrapper_base<Base>::type base_type;

      iterator_wrapper(Base iter)
        : base_type(iter)
      {}
  };

  long adjust_index(long index, long size)
  {
      if (index < 0)
          index += size;

      if (index < 0 || index >= size)
      {
          PyErr_SetString(PyExc_IndexError, "Index out of range");
          python::throw_error_already_set();
      }

      return index;
  }

  template <class T>
  void get_slice(
      PySliceObject* slice
    , T size
    , T& start
    , T& stop
    , T& step
  )
  {
      using python::extract;

      long start_ = slice->start == Py_None ? 0 : extract<long>(slice->start)();
      long stop_ = slice->stop == Py_None ? size : extract<long>(slice->stop)();
      long step_ = slice->step == Py_None ? 1 : extract<long>(slice->step)();

      if (step_ < 1)
      {
          PyErr_SetString(PyExc_IndexError, "Non-positive stride");
          python::throw_error_already_set();
      }

      start_ = adjust_index(start_, size);

      if (stop_ < 0)
          stop_ += (long)size;

      if (stop_ < 0 || stop_ > (long)size)
      {
          PyErr_SetString(PyExc_IndexError, "Index out of range");
          python::throw_error_already_set();
      }

      if (stop_ <= start_)
      {
          PyErr_SetString(PyExc_IndexError, "Zero size slice");
          python::throw_error_already_set();
      }

      start = start_;
      stop = stop_;
      step = step_;
  }

  //
  // A := model of MultiArray
  // Dims := number of dimensions of A
  //
  template <class A, long Dims>
  struct multi_array_concept
  {
      typedef mpl::bool_<Dims == 1> is_rank1;
      typedef typename A::index_range range;
      typedef typename A::size_type size_type; 
      typedef typename A::element element;
      typedef typename A::reference reference;
      typedef typename A::value_type value_type;

      static python::list shape(A const& ar)
      {
          return as_list(ar.shape(), ar.shape() + ar.num_dimensions());
      }

      static python::list strides(A const& ar)
      {
          return as_list(ar.strides(), ar.strides() + ar.num_dimensions());
      }

      static python::list index_bases(A const& ar)
      {
          return as_list(ar.index_bases(), ar.index_bases() + ar.num_dimensions());
      }

      struct get_item_visitor
      {
          template <class T>
          void operator()(T const& x)
          {
              result = python::object(x);
          }

          python::object result;
      };

      static python::object get_item(A& ar, python::object const& index)
      {
          get_item_visitor visitor;
          visit_index(ar, index, visitor);
          return visitor.result;
      }

      //
      // Helper for set_view_aux() below. Recursively iterates through all
      // the elements of a MultiArray, and assigns from values.
      //
      template <class V>
      static void set_view_aux0(V const& view_, python::object const& values, long& index)
      {
          V& view = const_cast<V&>(view_);

          for (typename V::iterator i(view.begin()), e(view.end()); i != e; ++i)
          {
              set_view_aux0(*i, values, index);
          }
      }

      static void set_view_aux0(
          element& ref, python::object const& values, long& index
      )
      {
          ref = python::extract<element>(values[index++]);
      }

      template <class V>
      static void set_view_aux(V const& view, PyObject* values_)
      {
          python::object values(python::borrowed(values_));

          if (values.attr("__len__")() != view.num_elements())
          {
              PyErr_SetString(PyExc_TypeError, "Cannot change array size in assign");
              python::throw_error_already_set();
          }

          long index = 0;
          set_view_aux0(view, values, index);
      }

      struct set_item_visitor
      {
          set_item_visitor(PyObject* value)
            : value(value)
          {}

          void operator()(element& item)
          {
              item = python::extract<element>(value);
          }

          template <class V>
          void operator()(V const& view)
          {
              set_view_aux(view, value);
          }

          PyObject* value;
      };

      static void set_item(A& ar, python::object const& index, PyObject* value)
      {
          set_item_visitor visitor(value);
          visit_index(ar, index, visitor);
      }

      template <class Visitor, class Indices, long  N, long M>
      static void visit_index_step(
          A& src
        , python::object const& index_tuple
        , Indices const& indices
        , boost::array<size_type, Dims>& index_array
        , Visitor& visitor
        , mpl::long_<N>
        , mpl::long_<M>
      )
      {
          python::object index = index_tuple[Dims - N];
          size_type const extent = src.shape()[Dims - N];

          if (PySlice_Check(index.ptr()))
          {
              PySliceObject* slice = (PySliceObject*)index.ptr();

              size_type start;
              size_type stop;
              size_type step;
              get_slice(slice, extent, start, stop, step);

              visit_index_step(
                  src
                , index_tuple
                , indices[range(start,stop,step)]
                , index_array
                , visitor
                , mpl::long_<N - 1>()
                , mpl::long_<M>()
              );
          }
          else
          {
              long idx = adjust_index(python::extract<long>(index), extent);

              index_array[Dims - N] = (size_type)idx;

              visit_index_step(
                  src
                , index_tuple
                , indices[idx]
                , index_array
                , visitor
                , mpl::long_<N - 1>()
                , mpl::long_<M - 1>()
            );           
          }
      }

      template <class Visitor, class Indices, long M>
      static void visit_index_step(
          A& src
        , python::object const&
        , Indices const& indices
        , boost::array<size_type, Dims>&
        , Visitor& visitor
        , mpl::long_<0>
        , mpl::long_<M>
      )
      {
          visitor(src[indices]);
      }

      template <class Visitor, class Indices>
      static void visit_index_step(
          A& src
        , python::object const&
        , Indices const&
        , boost::array<size_type, Dims>& index_array
        , Visitor& visitor
        , mpl::long_<0>
        , mpl::long_<0>
      )
      {
          visitor(src(index_array));
      }

      template <class Visitor>
      static void visit_index(A& src, python::object index, Visitor& visitor)
      {
          python::extract<long> idx(index);

          if (idx.check())
          {
              visitor(src[idx()]);
          }
          else
          {
              if (PySlice_Check(index.ptr()))
                  index = python::make_tuple(index);

              if (index.attr("__len__")() != Dims)
              {
                  PyErr_SetString(PyExc_IndexError, "Index list size mismatch");
                  python::throw_error_already_set();
              }

              boost::array<size_type, Dims> index_array;

              visit_index_step(
                  src
                , index
                , boost::indices
                , index_array
                , visitor
                , mpl::long_<Dims>()
                , mpl::long_<Dims>()
              );
          }
      }

      typedef typename A::iterator base_iterator;
      typedef typename mpl::if_<
          is_rank1, base_iterator, iterator_wrapper<base_iterator>
      >::type iterator;

      static iterator begin(A& ar)
      {
          return ar.begin();
      }

      static iterator end(A& ar)
      {
          return ar.end();
      }

      static python::class_<A>& class_(char const* name)
      {
          using namespace python;

          static python::class_<A> cl(name, no_init);

          cl
              .def("__iter__", python::range(begin, end))
              .def("__len__", &A::size)
              .def("__getitem__", get_item)
              .def("__setitem__", set_item)
              .add_property("shape", shape)
              .add_property("strides", strides)
              .add_property("index_bases", index_bases)
              .add_property("dimensions", &A::num_dimensions)
              .add_property("num_elements", &A::num_elements)
              .def(self == self)
              .def(self <= self)
              .def(self >= self)
              .def(self < self)
              .def(self > self)
              ;

          return cl;
      }
  };

  template <class A>
  struct multi_array_pickle_suite
    : python::pickle_suite
  {
      static python::tuple getinitargs(A const& ar)
      {
          return python::make_tuple(
              as_list(ar.shape(), ar.shape() + ar.num_dimensions())
          );
      }

      static python::tuple getstate(A const& ar)
      {
          return python::make_tuple(
              as_list(ar.data(), ar.data() + ar.num_elements())
          );
      }

      static void setstate(A& ar, python::tuple const& state)
      {
          if (state.attr("__len__")() != 1)
          {
              PyErr_SetObject(PyExc_ValueError,
                  ("expected 1-item tuple in call to __setstate__; got %s"
                   % state).ptr()
              );
              python::throw_error_already_set();
          }

          python::object data = state[0];

          if (data.attr("__len__")() != ar.num_elements())
          {
              PyErr_SetString(PyExc_ValueError, "pickled data size mismatch");
              python::throw_error_already_set();
          }

          for (typename A::size_type i = 0; i < ar.num_elements(); ++i)
          {
              ar.data()[i] = python::extract<typename A::element>(data[i]);
          }
      }
  };

  template <class A, long N>
  struct multi_array_class
  {
      typedef typename A::size_type size_type;
      typedef typename A::value_type value_type;

      static A* init(python::object const& extent_sequence)
      {
          if (extent_sequence.attr("__len__")() != N)
          {
              PyErr_SetString(PyExc_ValueError, "Extents list size mismatch");
              python::throw_error_already_set();
          }

          boost::array<size_type, N> extents;

          for (long i = 0; i < N; ++i)
          {
              long extent = python::extract<long>(extent_sequence[i]);

              if (extent < 1)
              {
                  PyErr_SetString(PyExc_ValueError, "Non-positive extent");
                  python::throw_error_already_set();
              }

              extents[i] = extent;
          }

          return new A(extents);
      }

      static void reshape(A& ar, python::object const& size_sequence)
      {
          if (size_sequence.attr("__len__")() != N)
          {
              PyErr_SetString(PyExc_ValueError, "Size list size mismatch");
              python::throw_error_already_set();
          }

          boost::array<size_type, N> sizes;

          size_type num_elements(1);

          for (long i = 0; i < N; ++i)
          {
              long size = python::extract<long>(size_sequence[i]);

              if (size < 1)
              {
                  PyErr_SetString(PyExc_ValueError, "Non-positive size");
                  python::throw_error_already_set();
              }

              sizes[i] = size;
              num_elements *= size;
          }

          if (num_elements != ar.num_elements())
          {
              PyErr_SetString(PyExc_TypeError, "Cannot change size of array");
              python::throw_error_already_set();
          }

          ar.reshape(sizes);
      }

      struct wrap_subarray_view
      {
          wrap_subarray_view(char const* value_name)
            : value_name(value_name)
          {}

          template <long M>
          void operator()(mpl::integral_c<long, M>) const
          {
              std::string const suffix = tostring(M) + "_" + value_name;

              multi_array_concept<
                  typename A::template array_view<M>::type, M
              >::class_(("view" + suffix).c_str());

              multi_array_concept<
                  typename A::template subarray<M>::type, M
              >::class_(("subarray" + suffix).c_str());
          }

          char const* value_name;
      };

      multi_array_class(char const* value_name = python::type_id<value_type>().name())
      {
          std::string const suffix = tostring(N) + "_" + value_name;

          multi_array_concept<A, N>::class_(("multi_array" + suffix).c_str())
              .def("__init__", python::make_constructor(init))
              .def("reshape", reshape)
              .def_pickle(multi_array_pickle_suite<A>())
              ;

          mpl::for_each<mpl::range_c<long, 1, N + 1> >(
              wrap_subarray_view(value_name)
          );
      }
  };

} // namespace aux

template <class ValueType, long Dims>
struct wrap_multi_array
{
    typedef boost::multi_array<ValueType, Dims> array_type;

    wrap_multi_array(
        char const* value_name = python::type_id<ValueType>().name()
    )
    {
        aux::multi_array_class<array_type, Dims> class_(value_name);
    }
};

}} // namespace boost::multi_array_python

#endif // BOOST_MULTI_ARRAY_PYTHON_060213_HPP

