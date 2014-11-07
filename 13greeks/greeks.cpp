#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE PORT
#include <boost/test/unit_test.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/math/distributions.hpp>
#include <boost/function.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <functional>
#include <numeric>

#include <ql/quantlib.hpp>
#include <boost/format.hpp>

namespace {

using namespace QuantLib;

BOOST_AUTO_TEST_CASE(testBlackScholes) {
  Real strike = 110.0;
  Real timeToMaturity = .5; // years
  Real spot = 100.0;
  Rate riskFree = .03;
  Rate dividendYield = 0.0;
  Volatility sigma = .20;

  // QL requires sigma * sqrt(T) instead of plain sigma
  Real vol = sigma * std::sqrt(timeToMaturity);

  DiscountFactor growth = std::exp(-dividendYield * timeToMaturity);

  DiscountFactor discount = std::exp(-riskFree * timeToMaturity);

  boost::shared_ptr<PlainVanillaPayoff> vanillaCallPayoff =
    boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Call, strike));

  BlackScholesCalculator bsCalculator(vanillaCallPayoff, spot, growth, vol, discount);

  std::cout << boost::format("Value of 110.0 call is %.4f") % bsCalculator.value() << std::endl;
  std::cout << boost::format("Delta of 110.0 call is %.4f") % bsCalculator.delta() << std::endl;
  std::cout << boost::format("Gamma of 110.0 call is %.4f") % bsCalculator.gamma() << std::endl;
  std::cout << boost::format("Vega of 110.0 call is %.4f")  % (bsCalculator.vega(timeToMaturity)/100) << std::endl;
  std::cout << boost::format("Theta of 110.0 call is %.4f") % (bsCalculator.thetaPerDay(timeToMaturity)) << std::endl;

  Real changeInSpot = 1.0;
  BlackScholesCalculator bsCalculatorSpotUpOneDollar(Option::Type::Call, strike, spot + changeInSpot, growth, vol, discount);
  std::cout << boost::format("Value of 110.0 call (spot up $%d) is %.4f")
    % changeInSpot % bsCalculatorSpotUpOneDollar.value() << std::endl;
  std::cout << boost::format("Value of 110.0 call (spot up $%d) estimated from delta is %.4f")
    % changeInSpot % (bsCalculator.value() + bsCalculator.delta() * changeInSpot) << std::endl;

  //use a Taylor series expansion to estimate the new price of a call given delta and gamma
  std::cout << boost::format("Value of 110.0 call (spot up $%d) estimated from delta and gamma is %.4f")
    % changeInSpot % (bsCalculator.value() + (bsCalculator.delta() * changeInSpot) + (.5 * bsCalculator.gamma() * changeInSpot)) << std::endl;

  //calculate new price of a call given a one point change in volatility
  Real changeInSigma = .01;
  BlackScholesCalculator bsCalculatorSigmaUpOnePoint(Option::Type::Call, strike, spot, growth, (sigma + changeInSigma) * std::sqrt(timeToMaturity) , discount);
  std::cout << boost::format("Value of 110.0 call (sigma up %.2f) is %.4f") % changeInSigma % bsCalculatorSigmaUpOnePoint.value() << std::endl;

  //estimate new price of call given one point change in volatility using vega
  std::cout << boost::format("Value of 110.0 call (sigma up %.2f) estimated from vega) is %.4f")
    % changeInSigma % (bsCalculator.value() + (bsCalculator.vega(timeToMaturity)/100)) << std::endl;
}

}
