#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FUTURE
#include <boost/test/unit_test.hpp>

#include <ql/quantlib.hpp>
#include <vector>
#include <iostream>

// Needed for Position
#include <ql/instruments/fixedratebondforward.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/pricingengines/bond/bondfunctions.hpp>

#include <boost/math/distributions.hpp>
#include <boost/function.hpp>
#include <boost/math/distributions.hpp>
#include <boost/format.hpp>

// K: forward price at issuance
// K = (S0 - I) * e^rT
// Future price
// F = S0 - I - K * e^-rT

using namespace QuantLib;

BOOST_AUTO_TEST_CASE(testCalculateForwardPriceOfFixedRateBond) {
  Calendar calendar = UnitedStates(UnitedStates::GovernmentBond);
  const Natural settlementDays = 1;
  Date today(2, April, 2013); // Date::todaysDate();
  Date bondIssueDate = calendar.adjust(today, ModifiedFollowing);
  Date bondMauturiyDate = bondIssueDate + Period(3, Years);
  Rate rate = .03;
  Settings::instance().evaluationDate() = bondIssueDate;

  std::cout << boost::format("Bond issue date is: %s") % bondIssueDate << std::endl;
  std::cout << boost::format("Bond maturity date is: %s") % bondMauturiyDate << std::endl;

  //coupon schedule
  std::vector<Rate> coupons(1, .05);

  //fixed rate bond
  Real faceValue = 100.0;
  boost::shared_ptr<FixedRateBond> fixedRateBondPtr
    (new FixedRateBond(settlementDays, calendar, faceValue, bondIssueDate, bondMauturiyDate, Period(Annual), coupons, ActualActual(ActualActual::Bond)));
  boost::shared_ptr<YieldTermStructure> flatForwardRates
    (new FlatForward(bondIssueDate, rate, ActualActual(ActualActual::Bond), Compounded, Annual));
  RelinkableHandle<YieldTermStructure> flatTermStructure(flatForwardRates);
  boost::shared_ptr<PricingEngine> bondEngine(new DiscountingBondEngine(flatTermStructure));
  fixedRateBondPtr->setPricingEngine(bondEngine);

  //calculate bond price
  Real bondPrice = fixedRateBondPtr->NPV();
  std::cout << "Bond price: " << bondPrice << std::endl;

  for(int i = 0; i < fixedRateBondPtr->cashflows().size(); i++) {
    boost::shared_ptr<CashFlow> cashFlow = fixedRateBondPtr->cashflows()[i];
    std::cout << boost::format("Cash flow: %s, %.2f") % cashFlow->date() % cashFlow->amount() << std::endl;
  }

  //forward maturity tenor
  Date forwardMaturityDate = bondIssueDate + Period(15, Months);
  Natural daysToMaturityOfForwardContract = (forwardMaturityDate - bondIssueDate) - settlementDays;
  std::cout << boost::format("Expiration of forward contract: %s") % forwardMaturityDate << std::endl;
  std::cout << boost::format("Days to maturity of forward contract: %i") % daysToMaturityOfForwardContract << std::endl;

  //calculate strike price/future value of the bond using periodic, annual rate (not continuous) because this is a bond
  Real income = (fixedRateBondPtr->nextCouponRate(bondIssueDate) * faceValue);
  const Real strike = bondPrice * (1 + rate * daysToMaturityOfForwardContract/365) - income;
  std::cout << boost::format("Strike price of forward contract is %.2f") % strike << std::endl;

  //forward contract on a fixed rate bond
  FixedRateBondForward fixedRateBondForward
    ( bondIssueDate, forwardMaturityDate, Position::Long, strike, settlementDays
    , ActualActual(ActualActual::Bond), calendar
    , ModifiedFollowing, fixedRateBondPtr, flatTermStructure, flatTermStructure);

  //calculate forward price of bond
  Real forwardPrice = fixedRateBondForward.NPV();
  std::cout << boost::format("Bond forward contract value: %.2f") % forwardPrice << std::endl;

  //evaluate forward contract by shocking interest rate +/- 1%
  boost::shared_ptr<YieldTermStructure> flatForwardRatesUpOnePercent
    (new FlatForward(bondIssueDate, rate + .01, ActualActual(ActualActual::Bond), Compounded, Annual));

  boost::shared_ptr<YieldTermStructure> flatForwardRatesDownOnePercent
    (new FlatForward(bondIssueDate, rate - .01, ActualActual(ActualActual::Bond), Compounded, Annual));

  flatTermStructure.linkTo(flatForwardRatesUpOnePercent);
  std::cout << boost::format("Bond forward contract value (rates + 1 percent): %.2f") % fixedRateBondForward.NPV() << std::endl;

  flatTermStructure.linkTo(flatForwardRatesDownOnePercent);
  std::cout << boost::format("Bond forward contract value (rates - 1 percent): %.2f") % fixedRateBondForward.NPV() << std::endl;
}
