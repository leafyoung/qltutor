#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE LOPT
#include <boost/test/unit_test.hpp>

#include <ql/quantlib.hpp>
#include <vector>
#include <iostream>
// #include <function>
#include <functional>

#include "er.hpp"

namespace {

using namespace QuantLib;

//objective function to be maximized
class PortfolioAllocationCostFunction : public CostFunction {
  public:
    //Must override this member function
    Real value(const Array& x) const {
      QL_REQUIRE(x.size() == 2, "Two bonds in portfolio!");
      return -1 * (4*x[0] + 3*x[1]); // mult by -1 to maximise
    }

    Disposable<Array> values(const Array& x) const {
      QL_REQUIRE(x.size() == 2, "TWo bonds in portfolio!");
      Array values(1);
      values[0] = value(x);
      return values;
    }
};

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

BOOST_AUTO_TEST_CASE(testLinearOptimization)
// int main( int argc, char* argv[] )
{
  PortfolioAllocationCostFunction portfolioAllocationCostFunction;

  //optimization constraints
  std::vector<std::function<bool (const Array&)> > constraints(3);

  //constraints implemented as C++11 lambda expressions
  constraints[0] = [](const Array& x) { return (x[0]+x[1] <= 100.0); };
  constraints[1] = [](const Array& x) { return ((2*x[0]+x[1])/100.0 <= 1.5); };
  constraints[2] = [](const Array& x) { return ((3*x[0]+4*x[1])/100.0 <= 3.6); };

  //instantiate constraints
  PositiveConstraint greaterThanZeroConstraint;
  PortfolioAllocationConstraints portfolioAllocationConstraints(constraints);
  //class that supports functional composition of constraints
  CompositeConstraint allConstraints(portfolioAllocationConstraints, greaterThanZeroConstraint);

  //end criteria that will terminate search
  Size maxIterations = 1000;
  Size minStatIterations = 10;
  Real rootEpisilon = 1e-8;
  Real functionEpsilon = 1e-9;
  Real gradientEpsilon = 1e-5;

  EndCriteria endCriteria(maxIterations, minStatIterations, rootEpisilon, functionEpsilon, gradientEpsilon);

  Problem bondAllocationProblem(portfolioAllocationCostFunction, allConstraints, Array(2,1));

  //use the simplex method
  Simplex solver(.1);

  EndCriteria::Type solution = solver.minimize(bondAllocationProblem, endCriteria);
  std::cout << boost::format("Simplex solution type: %s") % solution << std::endl;

  const Array& results = bondAllocationProblem.currentValue();
  std::cout << boost::format("Allocation %.2f percent to bond 1 and %.2f to bond 2.") % results[0] % results[1] << std::endl;
//  return 0;
}

}
