#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE VOLSURF
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
// #include <numeric>

#include <ql/quantlib.hpp>
#include <boost/format.hpp>

namespace {

using namespace QuantLib;

// BlackVarianceSurface: strikes, maturity dates, matrix of volatilities.
// row: strikes, columns: option expiration dates

BOOST_AUTO_TEST_CASE(testVolatilitySurface) {
  using namespace boost::assign;

  Size nStrikes = 5, nTenors = 5;

  std::vector<Real> strikes;
  strikes += 1650.0, 1660.0, 1670.0, 1675.0, 1680.0;

  std::vector<Date> expirations;
  expirations +=  Date(20, Month::Dec, 2013)
                , Date(17, Month::Jan, 2014)
                , Date(21, Month::Mar, 2014)
                , Date(20, Month::Jun, 2014)
                , Date(19, Month::Sep, 2014);

  Matrix volMatrix(strikes.size(), expirations.size());

  std::cout << "Test Volatility Surface" << std::endl;

  //1650 - Dec, Jan, Mar, Jun, Sep
  volMatrix[0][0] = .15640;
  volMatrix[0][1] = .15433;
  volMatrix[0][2] = .16079;
  volMatrix[0][3] = .16394;
  volMatrix[0][4] = .17383;

  //1660 - Dec, Jan, Mar, Jun, Sep
  volMatrix[1][0] = .15343;
  volMatrix[1][1] = .15240;
  volMatrix[1][2] = .15804;
  volMatrix[1][3] = .16255;
  volMatrix[1][4] = .17303;

  //1670 - Dec, Jan, Mar, Jun, Sep
  volMatrix[2][0] = .15128;
  volMatrix[2][1] = .14888;
  volMatrix[2][2] = .15512;
  volMatrix[2][3] = .15944;
  volMatrix[2][4] = .17038;

  //1675 - Dec, Jan, Mar, Jun, Sep
  volMatrix[3][0] = .14798;
  volMatrix[3][1] = .14906;
  volMatrix[3][2] = .15522;
  volMatrix[3][3] = .16171;
  volMatrix[3][4] = .16156;

  //1680 - Dec, Jan, Mar, Jun, Sep
  volMatrix[4][0] = .14580;
  volMatrix[4][1] = .14576;
  volMatrix[4][2] = .15364;
  volMatrix[4][3] = .16037;
  volMatrix[4][4] = .16042;

  QL_REQUIRE(strikes.size() == volMatrix.rows(), "Strikes shall be the same as volMatrix.size()");
  QL_REQUIRE(expirations.size() == volMatrix.columns(), "Expirations shall be the same size as volMatrix[0].size()");

  Date evaluationDate(30, Month::Sep, 2013);
  Settings::instance().evaluationDate() = evaluationDate;
  Calendar calendar = UnitedStates(UnitedStates::NYSE);
  DayCounter dayCounter = ActualActual();
  BlackVarianceSurface volatilitySurface(Settings::instance().evaluationDate()
      , calendar, expirations, strikes, volMatrix, dayCounter);
  std::cout << "Test Volatility Surface" << std::endl;
  volatilitySurface.enableExtrapolation(false);
  std::cout << "Test Volatility Surface" << std::endl;

  std::cout << "Using standard bilinear interpolation..." << std::endl;
  Real dec1650Vol = volatilitySurface.blackVol(expirations[0], 1650.0, true);
  std::cout << boost::format("Dec13 1650.0 volatility: %f") % dec1650Vol << std::endl;

  Real dec1655Vol = volatilitySurface.blackVol(expirations[0], 1655.0, true);
  std::cout << boost::format("Dec13 1655.0 volatility: %f") % dec1650Vol << std::endl;

  Real dec1685Vol = volatilitySurface.blackVol(expirations[0], 1685.0, true);
  std::cout << boost::format("Dec13 1685.0 volatility: %f") % dec1650Vol << std::endl;

  Real jun1655Vol = volatilitySurface.blackVol(expirations[3], 1655.0, true);
  std::cout << boost::format("Jun14 1655.0 volatility (interpolated): %f") % jun1655Vol << std::endl;

  Real sep1680Vol = volatilitySurface.blackVol(expirations[4], 1680.0, true);
  std::cout << boost::format("Sep14 1680.0 volatility: %f") % sep1680Vol << std::endl;

  //change interpolator to bicubic splines
  volatilitySurface.setInterpolation<Bicubic>();

  std::cout << "Using bicubic spline interpolation..." << std::endl;
  dec1650Vol = volatilitySurface.blackVol(expirations[0], 1650.0, true);
  std::cout << boost::format("Dec13 1650.0 volatility: %f") % dec1650Vol << std::endl;

  dec1655Vol = volatilitySurface.blackVol(expirations[0], 1655.0, true);
  std::cout << boost::format("Dec13 1655.0 volatility (interpolated): %f") % dec1655Vol << std::endl;

  dec1685Vol = volatilitySurface.blackVol(expirations[0], 1685.0, true);
  std::cout << boost::format("Dec13 1685.0 volatility (interpolated): %f") % dec1685Vol << std::endl;

  jun1655Vol = volatilitySurface.blackVol(expirations[3], 1655.0, true);
  std::cout << boost::format("Jun14 1655.0 volatility (interpolated): %f") % jun1655Vol << std::endl;

  sep1680Vol = volatilitySurface.blackVol(expirations[4], 1680.0, true);
  std::cout << boost::format("Sep14 1680.0 volatility: %f") % sep1680Vol << std::endl;

  // write out data points for gnuplot
  std::ofstream volSurfaceFile;
  volSurfaceFile.open("C://TEMP//VolSurface.csv", std::ios::out);

  for (Date expiration : expirations) {
    for (Real strike = strikes[0] - 5.0; strike <= strikes[4] + 5.0; ++strike) {
      Real volatility = volatilitySurface.blackVol(expiration, strike, true);
      BigInteger dayCount = dayCounter.dayCount(Settings::instance().evaluationDate(), expiration);
      volSurfaceFile << boost::format("%f,%f,%f") % strike
        % dayCount
        % volatility << std::endl;
    }
  }

  volSurfaceFile.close();
}

/* gnuplot script to generate 3D surface plot

set key top center
set xlabel "Strike"
set ylabel "Time to maturity (days)"
set border 4095
set ticslevel 0
set dgrid3d 41,41
set pm3d
splot "volsurface.dat" u 1:2:3 with lines title "ES Volatility Surface"

*/


}
