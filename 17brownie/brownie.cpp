#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE BROWNIE

#include <cstdlib>
#include <iostream>
#include <ql/quantlib.hpp>
// #include <boost/test/unit_test.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/math/distributions.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/binomial.hpp>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <functional>
#include <numeric>
#include <fstream>
#include <utility>
#include <boost/assign/std/vector.hpp>

using namespace QuantLib;

/*
Real studentTInverse() {
  return boost::math::quantile(d, p);
}
*/

using namespace boost::math::policies;
using namespace boost::math;

typedef binomial_distribution<
            double, 
            policy<discrete_quantile<integer_round_nearest> > > 
        binom_round_nearest;

class InverseCumulativeNormal
  : public std::unary_function<Real,Real> {
  public:
    // InverseCumulativeNormal( boost::math::students_t d) : d_(d) {}
    InverseCumulativeNormal() {}
    // function
    Real operator()(Real p) const {
      // return quantile(binom_round_nearest(50, 0.5), 0.95);
      //quantile(students_t(5), 0.05);
      quantile(binomial(50, 0.5), 0.05);
      return 1.0;
      //return 1.0;
    }
  private:
    //boost::math::students_t d_;
};

// BOOST_AUTO_TEST_CASE(testGeometricBrownieMotion) {

void testGeometricBrownieMotion() {
  Real startingPrice = 20.16;
  Real mu = .2312;
  Volatility sigma = 0.2116;
  Size timeSteps = 255 * 10;
  Time length = 1;

  const boost::shared_ptr<StochasticProcess>& gbm = 
    boost::shared_ptr<StochasticProcess>(new GeometricBrownianMotionProcess(startingPrice, mu, sigma));

  //Generating a series of normally distributed random number from a uniform distribution
  BigInteger seed = SeedGenerator::instance().get();
  typedef BoxMullerGaussianRng<MersenneTwisterUniformRng> MersenneBoxMuller;
  MersenneTwisterUniformRng mersenneRng(seed);
  MersenneBoxMuller boxMullerRng(mersenneRng);
  RandomSequenceGenerator<MersenneBoxMuller> gsg(timeSteps, boxMullerRng);

  //generate simulated path
  PathGenerator<RandomSequenceGenerator<MersenneBoxMuller> > gbmPathGenerator(gbm, length, timeSteps, gsg, false);
  const Path& samplePath = gbmPathGenerator.next().value;

  //calculate simulated sample returns using C++11 lambda expression
  boost::function<Real (Real, Real)> calcLogReturns = [](Real x, Real y) {return std::log(y/x);};
  std::vector<Real> logReturns;
  Path::iterator samplePathBegin = samplePath.begin();
  Path::iterator samplePathEnd = samplePath.end();
  Path::iterator endMinusOne = std::prev(samplePathEnd);
  Path::iterator beginPlusOne = std::next(samplePathBegin);

  std::transform(samplePathBegin, endMinusOne, beginPlusOne, std::back_inserter(logReturns), calcLogReturns);

  GeneralStatistics statistics;

  statistics.addSequence(logReturns.begin(), logReturns.end());
  std::cout << boost::format("Std. dev. of simulated returns (Normal): %.4f") % (statistics.standardDeviation() * std::sqrt(255 * 10)) << std::endl;

  statistics.reset();
  statistics.addSequence(samplePathBegin, samplePathEnd);
  std::cout << boost::format("Price statistics: mean=%.2f min=%.2f max=%.2f") % statistics.mean() % statistics.min() % statistics.max() << std::endl;

  std::ofstream gbmFile;
  gbmFile.open("C://TEMP//gbm.csv", std::ios::out);
  for (Size i = 0; i < timeSteps; ++i) {
    gbmFile << boost::format("%d %.4f") % i % samplePath.at(i) << std::endl;
  }

  gbmFile.close();
}

  /* gnuplot
  set key bottom center
  set key bottom box
  set xlabel "Time Step (Days)"
  set ylabel "Stock Price"
  plot "C://TEMP/gbm.csv" using 1:2 w lines t "INTC simulated stock price"
  */

/*
// BOOST_AUTO_TEST_CASE(testGeometricBrowianMotionStudentT) {
void testGeometricBrowianMotionStudentT() {
  Real startingPrice = 20.16;
  Real mu = .2312;
  Volatility sigma = .2116;
  Volatility scaledSigma = std::sqrt(sigma * sigma * 3/5);
  Size timeSteps = 255;
  Time length = 1;

  const boost::shared_ptr<StochasticProcess>& gbm =
    boost::shared_ptr<StochasticProcess>(new GeometricBrownianMotionProcess(startingPrice, mu, scaledSigma));

  BigInteger seed = SeedGenerator::instance().get();
  MersenneTwisterUniformRng mersenneRng(seed);
  RandomSequenceGenerator<MersenneTwisterUniformRng> rsg(timeSteps, mersenneRng);

  boost::math::students_t_distribution<> studentT(5);
  boost::function<Real (Real)> icd = [](const Real& p) {
    boost::math::students_t studentT(5);
    return quantile(studentT, p); };
    boost::bind(studentTInverse, studentT, _1);

  InverseCumulativeRsg<RandomSequenceGenerator<MersenneTwisterUniformRng>
    , boost::function<Real (Real)> > invCumRsg(rsg, icd);

  PathGenerator<InverseCumulativeRsg<RandomSequenceGenerator<MersenneTwisterUniformRng>
    , boost::function<Real (Real)> > > gbmPathGenerator(gbm, length, timeSteps, invCumRsg, false);
  const Path& samplePath = gbmPathGenerator.next().value;

  //calculate simulated sample returns using C++11 lambda expression
  boost::function<Real (Real, Real)> calcLogReturns = [](Real x, Real y) {return std::log(y/x);};
  std::vector<Real> logReturns;
  Path::iterator samplePathBegin = samplePath.begin();
  Path::iterator samplePathEnd = samplePath.end();
  Path::iterator endMinusOne = std::prev(samplePathEnd);
  Path::iterator beginPlusOne = std::next(samplePathBegin);

  std::transform(samplePathBegin, endMinusOne, beginPlusOne, std::back_inserter(logReturns), calcLogReturns);

  GeneralStatistics statistics;

  statistics.addSequence(logReturns.begin(), logReturns.end());
  std::cout << boost::format("Std. dev. of simulated returns (Normal): %.4f") % (statistics.standardDeviation() * std::sqrt(255 * 10)) << std::endl;

  statistics.reset();
  statistics.addSequence(samplePathBegin, samplePathEnd);
  std::cout << boost::format("Price statistics: mean=%.2f min=%.2f max=%.2f") % statistics.mean() % statistics.min() % statistics.max() << std::endl;

  std::ofstream gbmFile;
  gbmFile.open("C://TEMP//gbm-studentT.csv", std::ios::out);
  for (Size i = 0; i < timeSteps; ++i) {
    gbmFile << boost::format("%d %.4f") % i % samplePath.at(i) << std::endl;
  }

  gbmFile.close();

}
*/

/*
BOOST_AUTO_TEST_CASE(testGeometricBrowianMotionStudentTHalton) {
  Real startingPrice = 20.16;
  Real mu = .2312;
  Volatility sigma = .2116;
  Volatility scaledSigma = std::sqrt(sigma * sigma * 3/5);
  Size timeSteps = 255;
  Time length = 1;

  const boost::shared_ptr<StochasticProcess>& gbm =
    boost::shared_ptr<StochasticProcess>(new GeometricBrownianMotionProcess(startingPrice, mu, scaledSigma));

  BigInteger seed = SeedGenerator::instance().get();
  HaltonRsg rsg(timeSteps);

  boost::math::students_t_distribution<> studentT(5);
  boost::function<Real (Real)> icd = boost::bind(studentTInverse, studentT, _1);

  InverseCumulativeRsg<RandomSequenceGenerator<MersenneTwisterUniformRng>
    , boost::function<Real (Real)> > invCumRsg(rsg, icd);

  PathGenerator<InverseCumulativeRsg<RandomSequenceGenerator<MersenneTwisterUniformRng>
    , boost::function<Real (Real)> > > gbmPathGenerator(gbm, length, timeSteps, invCumRsg, false);
  const Path& samplePath = gbmPathGenerator.next().value;

  //calculate simulated sample returns using C++11 lambda expression
  boost::function<Real (Real, Real)> calcLogReturns = [](Real x, Real y) {return std::log(y/x);};
  std::vector<Real> logReturns;
  Path::iterator samplePathBegin = samplePath.begin();
  Path::iterator samplePathEnd = samplePath.end();
  Path::iterator endMinusOne = std::prev(samplePathEnd);
  Path::iterator beginPlusOne = std::next(samplePathBegin);

  std::transform(samplePathBegin, endMinusOne, beginPlusOne, std::back_inserter(logReturns), calcLogReturns);

  GeneralStatistics statistics;

  statistics.addSequence(logReturns.begin(), logReturns.end());
  std::cout << boost::format("Std. dev. of simulated returns (Normal): %.4f") % (statistics.standardDeviation() * std::sqrt(255 * 10)) << std::endl;

  statistics.reset();
  statistics.addSequence(samplePathBegin, samplePathEnd);
  std::cout << boost::format("Price statistics: mean=%.2f min=%.2f max=%.2f") % statistics.mean() % statistics.min() % statistics.max() << std::endl;

  std::ofstream gbmFile;
  gbmFile.open("C://TEMP//gbm-studentT.csv", std::ios::out);
  for (Size i = 0; i < timeSteps; ++i) {
    gbmFile << boost::format("%d %.4f") % i % samplePath.at(i) << std::endl;
  }

  gbmFile.close();
}
*/

int main()
{
  testGeometricBrownieMotion();
  return 1;
}


