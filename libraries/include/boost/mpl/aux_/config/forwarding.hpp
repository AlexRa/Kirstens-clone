
#ifndef BOOST_MPL_AUX_CONFIG_FORWARDING_HPP_INCLUDED
#define BOOST_MPL_AUX_CONFIG_FORWARDING_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/boost/mpl/aux_/config/forwarding.hpp,v $
// $Date: 2008-03-20 14:41:26 -0700 (Thu, 20 Mar 2008) $
// $Revision: 82858 $

#include <boost/mpl/aux_/config/workaround.hpp>

#if    !defined(BOOST_MPL_CFG_NO_NESTED_FORWARDING) \
    && !defined(BOOST_MPL_PREPROCESSING_MODE) \
    && BOOST_WORKAROUND(__BORLANDC__, < 0x600)

#   define BOOST_MPL_CFG_NO_NESTED_FORWARDING

#endif

#endif // BOOST_MPL_AUX_CONFIG_FORWARDING_HPP_INCLUDED
