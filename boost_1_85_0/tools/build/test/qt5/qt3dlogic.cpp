// (c) Copyright Juergen Hunold 2015
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE.txt or copy at
// https://www.bfgroup.xyz/b2/LICENSE.txt)

#define BOOST_TEST_MODULE Qt3DLogic
#include <Qt3DLogic>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (defines)
{
    BOOST_CHECK_EQUAL(BOOST_IS_DEFINED(QT_3DCORE_LIB), true);
    BOOST_CHECK_EQUAL(BOOST_IS_DEFINED(QT_3DLOGIC_LIB), true);
}

BOOST_AUTO_TEST_CASE ( sample_code )
{
    Qt3DLogic::QLogicAspect logicAspect;
}
