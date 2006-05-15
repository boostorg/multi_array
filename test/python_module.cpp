// Copyright Daniel Wallin 2006. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/python/module.hpp>
#include <boost/multi_array/python.hpp>

BOOST_PYTHON_MODULE(python_module)
{
    using namespace boost::multi_array_python;

    wrap_multi_array<float, 1>();
    wrap_multi_array<float, 2>();
    wrap_multi_array<float, 3>();
}


