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

Real expectedValueCallPayoff(Real spot, Real strike, Rate r, Volatility sigma, Time t, Real x) {
  Real mean = log(spot) + (r - 0.5 * sigma * sigma) * t;
  Real stdDev = sigma * sqrt(t);
  boost::math::lognormal d(mean, stdDev);
  return PlainVanillaPayoff(Option::Type::Call, strike)(x) * boost::math::pdf(d, x);
}

/* E(max(0,S-K)) = Int(K, Inf) ((S-K)*f(S)dS)
    f(S) is the pdf of the natural log of S
 */

BOOST_AUTO_TEST_CASE(testPriceCallOption) {
  Real spot = 100.0;
  Rate r = 0.03;
  Time t = 0.5;
  Volatility vol = 0.20;
  Real strike = 110.0;

  Real a = strike, b = strike * 10.0;

  boost::function< Real(Real) > ptrToExpectedValueCallPayoff =
    boost::bind(&expectedValueCallPayoff, spot, strike, r, vol, t, _1);

  Real absAcc = .00001;
  Size maxEval = 1000;
  SimpsonIntegral numInt(absAcc, maxEval);

  Real callOptionValue = numInt(ptrToExpectedValueCallPayoff, a, b) * std::exp(-r * t);

  std::cout << boost::format("Call option value is %.4f") % callOptionValue << std::endl;
}

}
