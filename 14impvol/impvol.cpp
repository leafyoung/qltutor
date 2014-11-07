#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE PORT
#include <boost/test/unit_test.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/math/distributions.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <functional>
// #include <numeric>

#include <ql/quantlib.hpp>
#include <boost/format.hpp>

namespace {

using namespace QuantLib;

class StrikeInfo {
  public:
    typedef std::pair<SimpleQuote, SimpleQuote> BidAsk;

    StrikeInfo(Option::Type optionType,const BidAsk& bidAsk, Real strike)
      : _payoff(new PlainVanillaPayoff(optionType, strike))
      , _bidAsk(bidAsk)
      , _impliedVol(0.0) {}

    StrikeInfo(const StrikeInfo& that)
      : _payoff(new PlainVanillaPayoff( that.getPayoff().optionType()
                                      , that.getPayoff().strike()) )
      , _bidAsk(that.getBidAsk())
      , _impliedVol(that.getImpliedVol()) {}

    StrikeInfo& operator=(StrikeInfo that) {
      swap(*this, that);
    }

    friend void swap(StrikeInfo& first, StrikeInfo& second) {
      using std::swap;
      first._payoff.swap(second._payoff);
      std::swap(first._impliedVol, second._impliedVol);
      std::swap(first._bidAsk, second._bidAsk);
    }

    const StrikedTypePayoff& getPayoff() const { return *_payoff; }
    const BidAsk& getBidAsk()            const { return _bidAsk; }
    Real  getStrike()                    const { return _payoff->strike(); }

    void  setImpliedVol(Volatility impliedVol) { _impliedVol = impliedVol;}
    const Volatility& getImpliedVol()    const { return _impliedVol; }

  private:
    boost::scoped_ptr<StrikedTypePayoff> _payoff;
    Volatility _impliedVol;
    BidAsk _bidAsk;
};

BOOST_AUTO_TEST_CASE(testESFuturesImpliedVolatility) {
  using namespace boost::posix_time;
  using namespace boost::gregorian;

  ActualActual actualActual;
  Settings::instance().evaluationDate() = Date(26, Month::August, 2013);
  Date expiration(20, Month::September, 2013);

  Time timeToMaturity = actualActual.yearFraction(Settings::instance().evaluationDate(), expiration);
  ptime quoteTime(from_iso_string("20130826T143000"));
  time_duration timeOfDayDuration = quoteTime.time_of_day();
  timeToMaturity += (timeOfDayDuration.hours() + timeOfDayDuration.minutes()/60.0)/(24.0 * 365.0);
  std::cout << boost::format("Time to maturity: %.6f") % timeToMaturity << std::endl;
  Real forwardBid = 1656.00;
  Real forwardAsk = 1656.25;
  Rate riskFree = .00273;
  DiscountFactor discount = std::exp(-riskFree * timeToMaturity);

  //calculate implied Volatility for OTM put options
  std::vector<StrikeInfo> putOptions;
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(7.75, 8.00), 1600));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(8.50, 9.00), 1605));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(9.25, 9.75), 1610));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(10.25, 10.75), 1615));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(11.25, 11.75), 1620));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(12.50, 12.75), 1625));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(13.75, 14.00), 1630));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(15.00, 15.50), 1635));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(16.50, 17.00), 1640));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(18.00, 18.50), 1645));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(20.00, 20.25), 1650));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(21.75, 22.25), 1655));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(24.00, 24.25), 1660));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(26.25, 26.75), 1665));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(28.75, 29.25), 1670));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(31.25, 32.25), 1675));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(34.25, 35.25), 1680));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(37.25, 38.25), 1685));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(40.75, 41.75), 1690));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(44.25, 45.25), 1695));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(47.50, 49.75), 1700));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(51.50, 53.75), 1705));
  putOptions.push_back(StrikeInfo(Option::Type::Put, std::make_pair(55.75, 58.00), 1710));


  // Delta-hedged Call with selling on bid
  //              Put with buying on ask
  for (StrikeInfo& putOption: putOptions) {
    StrikeInfo::BidAsk bidAsk = putOption.getBidAsk();
    Real price = (bidAsk.first.value() + bidAsk.second.value())/2.0;
    const StrikedTypePayoff& payoff = putOption.getPayoff();

    Bisection bisection;
    Real accuracy = 0.000001, guess = .20;
    Real min = .05, max = .40;
    Volatility sigma = bisection.solve([&](const Volatility & sigma) {
        Real stdDev = sigma * std::sqrt(timeToMaturity);
        BlackCalculator blackCalculator(payoff.optionType(), payoff.strike(), forwardAsk, stdDev, discount);
        return blackCalculator.value() - price;
      }, accuracy, guess, min, max);

    putOption.setImpliedVol(sigma);
    if (payoff(forwardAsk) > 0) { continue; } // skip ITM options
    std::cout << boost::format("IV of %f put is %.4f") % putOption.getStrike() % sigma << std::endl;
  }

  std::vector<StrikeInfo> callOptions;
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(63.00, 65.25), 1600));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(59.25, 60.25), 1605));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(55.25, 56.25), 1610));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(51.00, 52.00), 1615));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(47.00, 48.00), 1620));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(43.25, 44.25), 1625));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(39.50, 40.50), 1630));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(35.75, 36.75), 1635));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(32.25, 33.25), 1640));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(29.25, 29.75), 1645));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(26.00, 26.25), 1650));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(23.00, 23.50), 1655));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(20.00, 20.50), 1660));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(17.25, 17.75), 1665));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(14.75, 15.25), 1670));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(12.50, 13.00), 1675));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(10.50, 11.00), 1680));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(8.75, 9.25), 1685));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(7.00, 7.50), 1690));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(5.75, 6.00), 1695));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(4.70, 4.80), 1700));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(3.70, 3.85), 1705));
  callOptions.push_back(StrikeInfo(Option::Type::Call, std::make_pair(2.90, 3.05), 1710));

  for (StrikeInfo& callOption : callOptions) {
    StrikeInfo::BidAsk bidAsk = callOption.getBidAsk();
    Real price = (bidAsk.first.value() + bidAsk.second.value()) / 2.0;
    const StrikedTypePayoff& payoff = callOption.getPayoff();
    Bisection bisection;
    Real accuracy = 0.000001, guess = .20;
    Real min = .05, max = .40;

    Volatility sigma = bisection.solve([&](const Volatility & sigma) {
        Real stdDev = sigma * std::sqrt(timeToMaturity);
        BlackCalculator blackCalculator(payoff.optionType(), payoff.strike(), forwardBid, stdDev, discount);
        return blackCalculator.value() - price;
        }, accuracy, guess, min, max);

    callOption.setImpliedVol(sigma);
    if (payoff(forwardBid) > 0) { continue; }
    std::cout << boost::format("IV of %f call is %.4f") % callOption.getStrike() % sigma << std::endl;
  }

  std::ofstream ivFile;
  ivFile.open("C://TEMP/iv.csv", std::ios::out);

  for (StrikeInfo& putOption : putOptions) {
    if (putOption.getImpliedVol() > 0.0) {
      ivFile << boost::format("%f,%f") % putOption.getStrike() % putOption.getImpliedVol() << std::endl;
    }
  }

  for (StrikeInfo& callOption : callOptions) {
    if (callOption.getImpliedVol() > 0.0) {
      ivFile << boost::format("%f,%f") % callOption.getStrike() % callOption.getImpliedVol() << std::endl;
    }
  }
  ivFile.close();

  //plot with gnuplot using commands below. Run 'gnuplot' then type in:
  /*
  set terminal png
  set output "/tmp/volsmile.png"
  set key top center
  set key box
  set xlabel "Strike"
  set ylabel "Volatility"
  plot '/tmp/iv.dat' using 1:2 w linespoints title "ES Volatility Smile"
  */
}

}
