#include <ql/quantlib.hpp>
#include <iostream>

#define BOOST_TEST_DYN_LINK
// #define BOOST_AUTO_TEST_MAIN

#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE PV

#include <boost/test/unit_test.hpp>

using namespace QuantLib;

BOOST_AUTO_TEST_CASE(testCalculateLoanPayment)
{
  Leg cashFlows;
  Date date = Date::todaysDate();
  cashFlows.push_back(boost::shared_ptr<CashFlow>(new SimpleCashFlow(105.0, date+365)));
  Rate rate = .05;
  Real npv = CashFlows::npv(cashFlows, InterestRate(rate, ActualActual(), Compounded, Annual), true);
  std::cout << "Net Present Value (NPV) of cash flow is: " << npv << std::endl;

