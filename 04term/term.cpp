#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE TERM
// #include <boost/test/unit_test.hpp>

#include <ql/quantlib.hpp>
#include <vector>
#include <iostream>

using namespace QuantLib;

// BOOST_AUTO_TEST_CASE(testPriceBondWithFlatTermStructure)
int main()
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
  boost::shared_ptr<YieldTermStructure> flatForward(new FlatForward(issueDate, rate, ActualActual(ActualActual::Bond), Compounded, Annual));
  Handle<YieldTermStructure> flatTermStructure(flatForward);
  boost::shared_ptr<PricingEngine> bondEngine(new DiscountingBondEngine(flatTermStructure));
  fixedRateBond.setPricingEngine(bondEngine);
  Real npv = fixedRateBond.NPV();
  std::cout << "NPV of bond is: " << npv << std::endl;
  return 1;
}
