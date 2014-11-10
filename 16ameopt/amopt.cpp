#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE PORT
#include <boost/test/unit_test.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/math/distributions.hpp>
#include <boost/function.hpp>
#include <boost/assign/std/vector.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <functional>

#include <ql/quantlib.hpp>
#include <boost/format.hpp>

namespace {

using namespace QuantLib;

// Volatility Surface
boost::shared_ptr<BlackVolTermStructure> bootstrapVolatilityCurve(
    const Date& evaluationDate
  , const std::vector<Real>& strikes
  , const std::vector<Volatility>& vols
  , const Date& expiration) {
  Calendar calendar = UnitedStates(UnitedStates::NYSE);

  std::vector<Date> expirations;
  expirations.push_back(expiration);

  Matrix volMatrix(strikes.size(), 1);

  //implied volatilities from Interactive Brokers
  for (int i = 0; i < vols.size(); ++i) {
    volMatrix[i][0] = vols[i];
  }

  return boost::shared_ptr<BlackVolTermStructure>(new BlackVarianceSurface(evaluationDate, calendar, expirations, strikes, volMatrix, Actual365Fixed()));
}

// Libor curve
boost::shared_ptr<YieldTermStructure> bootstrapLiborZeroCurve(const Date& evaluationDate) {
  using namespace boost::assign;

  //bootstrap from USD LIBOR rates
  IborIndex libor = USDLiborON();
  const Calendar& calendar = libor.fixingCalendar();
  const Date& settlement = calendar.advance(evaluationDate, 2, Days);
  const DayCounter& dayCounter = libor.dayCounter();
  Settings::instance().evaluationDate() = settlement;

  //rates obtained from http://www.global-rates.com/interest-rates/libor/libor.aspx
  Rate overnight = .10490/100.0;
  Rate oneWeek = .12925/100.0;
  Rate oneMonth = .16750/100.0;
  Rate twoMonths = .20700/100.0;
  Rate threeMonths = .23810/100.0;
  Rate sixMonths = .35140/100.0;
  Rate twelveMonths = .58410/100.0;

  std::vector<boost::shared_ptr<RateHelper> > liborRates;
  liborRates += boost::shared_ptr<RateHelper>(new DepositRateHelper(overnight, boost::shared_ptr<IborIndex>(new USDLiborON())));
  liborRates += boost::shared_ptr<RateHelper>(new DepositRateHelper(oneWeek,   boost::shared_ptr<IborIndex>(new USDLibor(Period(1, Weeks)))));
  liborRates += boost::shared_ptr<RateHelper>(new DepositRateHelper(oneMonth,  boost::shared_ptr<IborIndex>(new USDLibor(Period(1, Months)))));
  liborRates += boost::shared_ptr<RateHelper>(new DepositRateHelper(twoMonths, boost::shared_ptr<IborIndex>(new USDLibor(Period(2, Months)))));
  liborRates += boost::shared_ptr<RateHelper>(new DepositRateHelper(threeMonths, boost::shared_ptr<IborIndex>(new USDLibor(Period(3, Months)))));
  liborRates += boost::shared_ptr<RateHelper>(new DepositRateHelper(sixMonths, boost::shared_ptr<IborIndex>(new USDLibor(Period(6, Months)))));
  liborRates += boost::shared_ptr<RateHelper>(new DepositRateHelper(twelveMonths, boost::shared_ptr<IborIndex>(new USDLibor(Period(12, Months)))));

  //use cubic interpolation
  boost::shared_ptr<YieldTermStructure> yieldCurve =
    boost::shared_ptr<YieldTermStructure>(new PiecewiseYieldCurve<ZeroYield, Cubic>(settlement, liborRates, dayCounter));

  return yieldCurve;
}

// dividend curve
boost::shared_ptr<ZeroCurve> bootstrapDividendCurve
  ( const Date& evaluationDate
  , const Date& expiration
  , const Date& exDivDate
  , Real underlyingPrice
  , Real annualDividend )
{
  UnitedStates calendar(UnitedStates::NYSE);
  Settings::instance().evaluationDate() = evaluationDate;
  Real settlementDays = 2.0;

  Real dividendDiscountDays = (expiration - evaluationDate) + settlementDays;
  std::cout << boost::format("Dividend discounting days: %d") % dividendDiscountDays << std::endl;
  Rate dividendYield = (annualDividend/underlyingPrice) * dividendDiscountDays / 365;

  //ex div rates and yields
  std::vector<Date> exDivRates;
  std::vector<Rate> dividendYields;

  //last ex div date and yield
  exDivRates.push_back(calendar.advance(exDivDate, Period(-3, Months), ModifiedPreceding, true));
  dividendYields.push_back(dividendYield);

  //next ex div date (projected) and yield
  Date projectedNextExDivDate = calendar.advance(exDivDate, Period(3, Months), ModifiedPreceding, true);
  std::cout << boost::format("Next projected ex div date for INTC: %s") % projectedNextExDivDate << std::endl;
  exDivRates.push_back(projectedNextExDivDate);
  dividendYields.push_back(dividendYield);

  return boost::shared_ptr<ZeroCurve>(new ZeroCurve(exDivRates, dividendYields, ActualActual(), calendar));
}

//pricer

BOOST_AUTO_TEST_CASE(testAmericanOptionPricingWithDividends) {
  using namespace boost::assign;

  //set up calendar/dates
  Calendar calendar = UnitedStates(UnitedStates::NYSE);
  Date today(15, Nov, 2013);
  Real settlementDays = 2;
  Date settlement = calendar.advance(today, settlementDays, Days);
  Settings::instance().evaluationDate() = today;

  //define options to price
  Option::Type type(Option::Call);
  Real underlying = 24.52;

  //INTC Feb 21 strikes
  std::vector<Real> strikes;
  strikes += 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0;

  // volatility for each strike above
  std::vector<Volatility> vols;
  vols += .23356, .21369, .20657, .20128, .19917, .19978, .20117;

  Date expiration(21, Feb, 2014);

  Date exDivDate(5, Feb, 2014);
  Real annualDividend = .90;

  Handle<YieldTermStructure> yieldTermStructure(bootstrapLiborZeroCurve(today));

  Handle<YieldTermStructure> dividendTermStructure(bootstrapDividendCurve(today, expiration, exDivDate, underlying, annualDividend));

  Handle<BlackVolTermStructure> volatilityTermStructure(bootstrapVolatilityCurve(today, strikes, vols, expiration));

  Handle<Quote> underlyingH(boost::shared_ptr<Quote>(new SimpleQuote(underlying)));
  boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(new BlackScholesMertonProcess(underlyingH, dividendTermStructure, yieldTermStructure, volatilityTermStructure));

  boost::shared_ptr<PricingEngine> pricingEngine(new FDAmericanEngine<CrankNicolson>(bsmProcess, 801, 800));

  boost::shared_ptr<Exercise> americanExercise(new AmericanExercise(settlement, expiration));
  for (Real strike : strikes) {
    boost::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
    VanillaOption americanOption(payoff, americanExercise);
    americanOption.setPricingEngine(pricingEngine);
    Real tv = americanOption.NPV();
    std::cout << boost::format("Intel %s %.2f %s value: %.2f") % expiration % strike % type % tv << std::endl;
    std::cout << boost::format("Delta: %.4f") % americanOption.delta() << std::endl;
    std::cout << boost::format("Gamma: %.4f") % americanOption.gamma() << std::endl;
  }
}

}
