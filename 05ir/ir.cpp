#include <ql/quantlib.hpp>
#include <iostream>

#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN

#define BOOST_TEST_MODULE IR

#include <boost/test/unit_test.hpp>

using namespace QuantLib;

BOOST_AUTO_TEST_CASE(testInterestRateConversion)
{
  //annual/effective rate
  Rate annualRate = .05;

  //5% rate compouded annually
  InterestRate effectiveRate(annualRate, ActualActual(), Compounded, Annual);
  std::cout << "Rate with annual compounding is: " << effectiveRate.rate() << std::endl;

  //what is the equivalent semi-annual one year rate?
  InterestRate semiAnnualCompoundingOneYearRate = 
    effectiveRate.equivalentRate(Compounded, Semiannual, 1);
  std::cout << "Equivalent one year semi-annually compounded rate is: " << semiAnnualCompoundingOneYearRate.rate() << std::endl;

  //what is the Equivalent 1 year rate if compounded continously?
  InterestRate continousOneYearRate = effectiveRate.equivalentRate(Continuous, Annual, 1);
  std::cout << "Equivalent one year continuous compounded rate is: " << continousOneYearRate.rate() << std::endl;
}
