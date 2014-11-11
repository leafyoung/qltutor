#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE POPT
#include <boost/test/unit_test.hpp>
#include <boost/detail/lightweight_test.hpp>

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

//aggregate all constraint expressions into a single constraint
class PortfolioAllocationConstraints : public Constraint {
  public:
    PortfolioAllocationConstraints(const std::vector<std::function<bool (const Array&) > >& expressions)
      : Constraint (boost::shared_ptr<Constraint::Impl>(new PortfolioAllocationConstraints::Impl(expressions))) {}

  private:
    // constraint implementation
    class Impl : public Constraint::Impl {
      public:
        Impl(const std::vector<std::function<bool (const Array&) > >& expressions) :
          expressions_(expressions) {}

        bool test(const Array& x) const {
          for (auto iter = expressions_.begin(); iter < expressions_.end(); ++iter) {
            if (!(*iter)(x)) {
              return false;
            }
          }
          //will only get here if all constraints satisfied
          return true;
        }

      private:
        const std::vector<std::function<bool (const Array&)> > expressions_;
    };
};

// Optimize for sharp ratio
class ThetaCostFunction : public CostFunction {
  public:
    ThetaCostFunction( const Matrix& covarianceMatrix
                     , const Matrix& returnMatrix )
      : covarianceMatrix_(covarianceMatrix)
      , returnMatrix_(returnMatrix) { }

    Real value(const Array& proportions) const {
      QL_REQUIRE(proportions.size() == 3, "Four assets in portfolio!");
      Array allProportions(4);
      allProportions[0] = proportions[0];
      allProportions[1] = proportions[1];
      allProportions[2] = proportions[2];
      allProportions[3] = 1 - proportions[0] - proportions[1] - proportions[2];
      return -1 * ((portfolioMean(allProportions) - c_)/portfolioStdDeviation(allProportions));
    }

    Disposable<Array> values(const Array& proportions) const {
      QL_REQUIRE(proportions.size() == 3, "Four assets in portfolio");
      Array values(1);
      values[0] = value(proportions);
      return values;
    }

    void setC(Real c) { c_ = c; }
    Real getC() const { return c_; }
    Real portfolioMean(const Array& proportions) const {
      Real portfolioMean = (proportions * returnMatrix_)[0];
      //std::cout << boost::format("Portfolio mean: %.4f") % portfolioMean << std::endl;
      return portfolioMean;
    }

    Real portfolioStdDeviation(const Array& proportions) const {
      Matrix matrixProportions(4, 1);
      for (size_t row = 0; row < 4; ++row) {
        matrixProportions[row][0] = proportions[row];
      }

      const Matrix& portfolioVarianceMatrix = transpose(matrixProportions) * covarianceMatrix_ * matrixProportions;
      Real portfolioVariance = portfolioVarianceMatrix[0][0];
      Real stdDeviation = std::sqrt(portfolioVariance);
      //std::cout << boost::format("Portfolio standard deviation: %.4f") % stdDeviation << std::endl;
      return stdDeviation;
    }

  private:
    const Matrix& covarianceMatrix_
                , returnMatrix_;
    Real c_;
};

BOOST_AUTO_TEST_CASE(testNoShortSales) {
  Matrix covarianceMatrix(4,4);

  //row 1
  covarianceMatrix[0][0] = .1; //AAPL-AAPL
  covarianceMatrix[0][1] = .03; //AAPL-IBM
  covarianceMatrix[0][2] = -.08; //AAPL-ORCL
  covarianceMatrix[0][3] = .05; //AAPL-GOOG
  //row 2
  covarianceMatrix[1][0] = .03; //IBM-AAPL
  covarianceMatrix[1][1] = .20; //IBM-IBM
  covarianceMatrix[1][2] = .02; //IBM-ORCL
  covarianceMatrix[1][3] = .03; //IBM-GOOG
  //row 3
  covarianceMatrix[2][0] = -.08; //ORCL-AAPL
  covarianceMatrix[2][1] = .02; //ORCL-IBM
  covarianceMatrix[2][2] = .30; //ORCL-ORCL
  covarianceMatrix[2][3] = .2; //ORCL-GOOG
  //row 4
  covarianceMatrix[3][0] = .05; //GOOG-AAPL
  covarianceMatrix[3][1] = .03; //GOOG-IBM
  covarianceMatrix[3][2] = .2; //GOOG-ORCL
  covarianceMatrix[3][3] = .9; //GOOG-GOOG

  std::cout << "Covariance matrix of returns: " << std::endl;
  std::cout << covarianceMatrix << std::endl;

  //portfolio return vector
  Matrix portfolioReturnVector(4,1);
  portfolioReturnVector[0][0] = .08; //AAPL
  portfolioReturnVector[1][0] = .09; //IBM
  portfolioReturnVector[2][0] = .10; //ORCL
  portfolioReturnVector[3][0] = .11; //GOOG

  std::cout << "Portfolio return vector" << std::endl;
  std::cout << portfolioReturnVector << std::endl;

  //constraints
  std::vector<std::function<bool (const Array&) > > noShortSalesConstraints(3);

  //constraints implemented in C++ 11 lambda expressions
  noShortSalesConstraints[0] = [] (const Array& x) { Real x4 = 1.0 - (x[0] + x[1] + x[2]); return (x[0] >= 0.05 && x[1] >= 0.05 && x[2] > 0.05 && x4 > 0.05);};
  noShortSalesConstraints[1] = [] (const Array& x) { Real x4 = 1.0 - (x[0] + x[1] + x[2]); return std::abs(1.0 - x[0] - x[1] - x[2] - x4) < 1e-9;};
  // noShortSalesConstraints[2] = [] (const Array& x) { Real x4 = 1.0 - (x[0] + x[1] + x[2]); return (std::abs(x[0]) <= 0.5 && std::abs(x[1]) <= 0.5 && std::abs(x[2]) <= 0.5 && std::abs(x4) <= 0.5);};
  noShortSalesConstraints[2] = [] (const Array& x) { Real x4 = 1.0 - (x[0] + x[1] + x[2]); return (x[0] <= .50 && x[1] <= .50 && x[2] <= .50 && x4 <= .50);};

  //instantiate constraints
  PortfolioAllocationConstraints noShortSalesPortfolioConstraints(noShortSalesConstraints);

  Size maxIterations = 100000;
  Size minStatIterations = 100;
  Real rootEpsilon = 1e-9;
  Real functionEpsilon = 1e-9;
  Real gradientNormEpsilon = 1e-9;
  EndCriteria endCriteria(maxIterations, minStatIterations, rootEpsilon, functionEpsilon, gradientNormEpsilon);

  std::map<Rate, std::pair<Volatility, Real> > mapOfStdDeviationToMeanNoShortSales;

  Rate startingC = -.035;
  Real increment = .005;

  for (int i = 0; i < 40; ++i) {
    Rate c = startingC + (i * increment);
    ThetaCostFunction thetaCostFunction(covarianceMatrix, portfolioReturnVector);
    thetaCostFunction.setC(c);
    Problem efficientFrontierNoShortSalesProblem(thetaCostFunction, noShortSalesPortfolioConstraints, Array(3, .2500));
    Simplex solver(.01);

    EndCriteria::Type noShortSalesSolution = solver.minimize(efficientFrontierNoShortSalesProblem, endCriteria);

    std::cout << boost::format("Solution type: %s") % noShortSalesSolution << std::endl;

    const Array& results = efficientFrontierNoShortSalesProblem.currentValue();
    Array proportions(4);
    proportions[0] = results[0];
    proportions[1] = results[1];
    proportions[2] = results[2];
    proportions[3] = 1.0 - results[0] - results[1] - results[2];

    std::cout << boost::format("Constant (c): %.4f") % thetaCostFunction.getC() << std::endl;
    std::cout << boost::format("Constant (c): %.4f") % thetaCostFunction.getC() << std::endl;
    std::cout << boost::format("AAPL weighting: %.4f") % proportions[0] << std::endl;
    std::cout << boost::format("IBM weighting: %.4f") % proportions[1] << std::endl;
    std::cout << boost::format("ORCL weighting: %.4f") % proportions[2] << std::endl;
    std::cout << boost::format("GOOG weighting: %.4f") % proportions[3] << std::endl;
    std::cout << boost::format("Theta: %.4f") % (-1 * efficientFrontierNoShortSalesProblem.functionValue()) << std::endl;
    Real portfolioMean = thetaCostFunction.portfolioMean(proportions);
    std::cout << boost::format("Portfolio mean: %.4f") % portfolioMean << std::endl;
    Volatility portfolioStdDeviation = thetaCostFunction.portfolioStdDeviation(proportions);
    std::cout << boost::format("Portfolio standard deviation: %.4f") % portfolioStdDeviation << std::endl;
    mapOfStdDeviationToMeanNoShortSales[c] = std::make_pair(portfolioStdDeviation, portfolioMean);
    std::cout << "------------------------------------" << std::endl;
  }

  std::ofstream noShortSalesFile;
  noShortSalesFile.open("C://TEMP//positionlimits.dat", std::ios::out);
  for ( std::map<Rate, std::pair<Volatility, Rate> >::const_iterator it = mapOfStdDeviationToMeanNoShortSales.begin()
      ; it != mapOfStdDeviationToMeanNoShortSales.end()
      ; ++it ) {
    noShortSalesFile << boost::format("%f %f %f") % it->first % it->second.first % it->second.second << std::endl;
  }
  noShortSalesFile.close();

  // plot using gnuplot
  // plot with gnuplot using commands below Run 'gnuplot' then type in:
  // set terminal png
  // set output "/tmp/positionlimits.png"
  // set key top left
  // set key box
  // set xlabel "Volatility"
  // set ylabel "Expected Return"
  // plot '/tmp/noshortsales.dat' using 2:3 w linespoints title "No Short Sales", "/tmp/positionlimits.dat" using 2:3 w linespoints title "Position Limits"
}


}
