#include <ql/quantlib.hpp>
#include <iostream>

#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN

#define BOOST_TEST_MODULE BOND

#include <boost/test/unit_test.hpp>

using namespace QuantLib;

BOOST_AUTO_TEST_CASE(testCalculateBondPrice)
{
  Leg cashFlows;
  Date date = Date::todaysDate();
  cashFlows.push_back(boost::shared_ptr<CashFlow>(new SimpleCashFlow(5.0, date+365)));
  cashFlows.push_back(boost::shared_ptr<CashFlow>(new SimpleCashFlow(5.0, date+2*365)));
  cashFlows.push_back(boost::shared_ptr<CashFlow>(new SimpleCashFlow(105.0, date+3*365)));
  Rate rate = .03;
  Real npv = CashFlows::npv(cashFlows, InterestRate(rate, ActualActual(ActualActual::Bond), Compounded, Annual), true);
  std::cout << "Price of 3 year bond with annual coupons is: " << npv << std::endl;
}
