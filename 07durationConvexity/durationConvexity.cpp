#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE DURATION
#include <boost/test/unit_test.hpp>

#include <ql/quantlib.hpp>
#include <vector>
#include <iostream>

using namespace QuantLib;

BOOST_AUTO_TEST_CASE(testCalculateBondDurationAndConvexity)
{
  Calendar calendar = UnitedStates(UnitedStates::GovernmentBond);
  const Natural settlementDays = 3;
  Date today = Date::todaysDate();
  Date issueDate = today;
  Date terminationDate = issueDate + Period(3, Years);
  Rate rate = .03;

  InterestRate couponRate(.05, ActualActual(ActualActual::Bond), Compounded, Annual);
  Real faceValue = 100.0;
  std::vector<InterestRate> coupons(3, couponRate);
  Schedule schedule(issueDate, terminationDate, Period(Annual), calendar, Unadjusted, Unadjusted, DateGeneration::Backward, false);
  FixedRateBond fixedRateBond(settlementDays, faceValue, schedule, coupons);
  boost::shared_ptr<YieldTermStructure> flatForwardRates(new FlatForward(issueDate, rate, ActualActual(ActualActual::Bond), Compounded, Annual));
  Handle<YieldTermStructure> flatTermStructure(flatForwardRates);
  boost::shared_ptr<PricingEngine> bondEngine(new DiscountingBondEngine(flatTermStructure));
  fixedRateBond.setPricingEngine(bondEngine);

  //calculate bond price
  // bondprice(r,c,t,f) = sum(c*f/(1+r)^i,i,1,t-1) + (c*f+f)/(1+r)^t;
  Real price = fixedRateBond.NPV();
  std::cout << "Price of bond is: " << price << std::endl;

  //calculate yield to maturity (YTM)/internal rate of return (IRR)
  // back-out r from bondprice
  Real ytm = fixedRateBond.yield(ActualActual(ActualActual::Bond), Compounded, Annual);
  std::cout << "yield to maturity: " << ytm << std::endl;

  //calcualte Macaulay duration
  //time-weighted average maturity of the bond
  //duration(r,c,t,f) = sum(i*c*f/(1+r)^i,i,1,t-1) + t*(c*f+f)/(1+r)^t/bondprice(r,c,t,f)
  InterestRate yield(ytm, ActualActual(ActualActual::Bond), Compounded, Annual);
  Time macDuration = BondFunctions::duration(fixedRateBond, yield, Duration::Macaulay, today);
  std::cout << "Macaulay duraiton: " << macDuration << std::endl;

  //calcualte modified duraiton
  //dmodr(r,c,t,f,p) = -1 * duration(r,c,t,f)/(1+r)
  //dmod = 1/P * dP/dr
  //dP/P = dmod * dr
  //dmod = dP/dr / P
  Time modDuration = -1 * BondFunctions::duration(fixedRateBond, yield, Duration::Modified, today);
  std::cout << "Modified duration: " << modDuration << std::endl;

  //calcualte convexity
  //convexity = 1/P * dP2/dr2
  //convexity = dP2/dr2 / P
  Real convexity = BondFunctions::convexity(fixedRateBond, yield, today);
  std::cout << "Convexity: " << convexity << std::endl;

  //Est. 1:   //dbondprice
  //Change_in_P = P * (Dmod * Change_in_r + .5 * Convexity * Change_in_r ^ 2)
  Real priceDuration = price + price * (modDuration * .01);
  std::cout << boost::format("Estimated bond price using only duration (rate up .01): %.2f") % priceDuration << std::endl;

  //estimate new bond price for an increase in interest rate of 1%
  //using duration and convexity
  Real priceConvexity = price + price * (modDuration * .01 + (.5 * convexity * std::pow(.01, 2)));
  std::cout << boost::format("Estimated bond price using duration and convexity (rate up .01): %.2f") % priceConvexity << std::endl;
}
