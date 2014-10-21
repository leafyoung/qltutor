#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE IRR
#include <boost/test/unit_test.hpp>

#include <ql/quantlib.hpp>
#include <vector>
#include <iostream>

using namespace QuantLib;

namespace {
  using namespace QuantLib;
  class IRRSolver {
    public:
      explicit IRRSolver(const Leg& cashFlows, Real npv):
        _cashFlows(cashFlows), _npv(npv) {};
      Real operator() (const Rate& rate) const {
        InterestRate interestRate(rate, ActualActual(ActualActual::Bond), Compounded, Annual);
        return CashFlows::npv(_cashFlows, interestRate, false) - _npv;
      }
    private:
      const Real _npv;
      const Leg& _cashFlows;
  };

  BOOST_AUTO_TEST_CASE(testCalculateBondYieldToMaturity)
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
    Real npv = fixedRateBond.NPV();
    std::cout << "NPV of bond is: " << npv << std::endl;

    //solve for yield to maturity using bisection solver
    Bisection bisection;
    Real accuracy = 0.0000001, guess = .10;
    Real min = .0025, max = .15;

    //invoke bisection solver with IRRSolver functor
    Real irr = bisection.solve(IRRSolver(fixedRateBond.cashflows(), npv), accuracy, guess, min, max);
    std::cout << "Bond yield to maturity (IRR) is: " << irr << std::endl;

    /*
    //invoke bisection solver with C++ 11 lambda expression
    irr = bisection.solve(
        [&](const Rate& rate) {
          return CashFlows::npv(fixedRateBond.cashflows(), InterestRate(rate, ActualActual(ActualActual::Bond), Compounded, Annual), false) - npv;
        }
        , accuracy
        , guess
        , min
        , max );

    std::cout << "Bond yield to maturity (IRR) is" << irr << std::endl;
    */
  }
}
