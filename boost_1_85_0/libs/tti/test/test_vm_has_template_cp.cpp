
//  (C) Copyright Edward Diener 2011
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#include "test_vm_has_template_cp.hpp"
#include <boost/detail/lightweight_test.hpp>

int main()
  {
  
#if BOOST_PP_VARIADICS

  BOOST_TEST(BOOST_TTI_HAS_TEMPLATE_GEN(ATPMemberTemplate)<AType>::value);
  BOOST_TEST(BOOST_TTI_HAS_TEMPLATE_GEN(AMemberTemplate)<AType>::value);
  BOOST_TEST(BOOST_TTI_HAS_TEMPLATE_GEN(SomeMemberTemplate)<AnotherType>::value);
  BOOST_TEST(BOOST_TTI_HAS_TEMPLATE_GEN(SimpleTMP)<AnotherType>::value);
  BOOST_TEST(!BOOST_TTI_HAS_TEMPLATE_GEN(TemplateNotExist)<AnotherType>::value);

  BOOST_TEST(HaveCL<AType>::value);
  BOOST_TEST(HaveAnotherMT<AType>::value);
  BOOST_TEST(ATemplateWithParms<AnotherType>::value);
  
  BOOST_TEST(BOOST_TTI_HAS_TEMPLATE_GEN(NestedMemberTemplate)<AType::AnUnion>::value);
  
  // Passing non-class enclosing type will return false
  
  BOOST_TEST(!BOOST_TTI_HAS_TEMPLATE_GEN(AMemberTemplate)<AType *>::value);
  BOOST_TEST(!ATemplateWithParms<double>::value);
  
#endif // BOOST_PP_VARIADICS

  return boost::report_errors();

  }
