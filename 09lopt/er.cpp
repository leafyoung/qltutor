#include "er.hpp"
#include <ql/QuantLib.hpp>

using namespace QuantLib;

Matrix WeightsMV(bool isConstrained, Size n, const Matrix& cov) {
  Matrix weights(n,1);

  if (isConstrained) {
    MeanVarianceFunction mvFunc(cov);

    Size maxIterations=10000; //end search after 1000 iterations if no solution
    Size minStatIterations=10; //don't spend more than 10 iterations at a single point
    Real rootEpsilon=1e-10; //end search if absolute difference of current and last root value is below epsilon
    Real functionEpsilon=1e-10; //end search if absolute difference of current and last function value is below epsilon
    Real gradientNormEpsilon=1e-6; //end search if absolute difference of norm of current and last gradient is below epsilon

    EndCriteria myEndCrit(maxIterations, minStatIterations, rootEpsilon,
      functionEpsilon, gradientNormEpsilon);

    //constraints
    LongOnlyConstraint longOnly;
    FullyInvestedConstraint fullyInvested;

    CompositeConstraint allConstraints(longOnly, fullyInvested);

    Problem myProb(mvFunc, allConstraints, Array(n-1, 1./n));

    Simplex solver(.05);

    EndCriteria::Type solution=solver.minimize(myProb, myEndCrit);

    switch (solution) {
      case EndCriteria::None:
      case EndCriteria::MaxIterations:
      case EndCriteria::Unknown:
        throw("#err: optimization didn't converge - no solution found");
      default: ;
    }

    Array x = myProb.currentValue();
    double sumWeights(0);
    for (Size i = 0; i < x.size(); ++i) {
      weights[i][0] = x[i];
      sumWeights += weights[i][0];
    }

    weights[n-1][0] = 1- sumWeights;
  } else {
    //closed form solution exists for unconstrained
    Matrix covInv = inverse(cov);
    Matrix l(n,1,1);
    weights = (covInv * l) / ( transpose(l) * covInv * l )[0][0];
  }

  return weights;
}

bool ValidateIdenticalCorrelation(const Matrix& correlation) {
  return false;
}

Matrix WeightsInvVol(Size n) {
  return Matrix(n,1);
}

Matrix WeightsERC(Size n, Matrix& cov, const Matrix& correlation) {
  Matrix weights(n,1);

  if (ValidateIdenticalCorrelation(correlation)) {
    weights = WeightsInvVol(n);
  } else {
    EqualRiskContributionFunction ercFunc(cov);

    Size maxIterations=10000; //end search after 1000 iterations if no solution
    Size minStatIterations=100; //don't spend more than 10 iterations at a single point
    Real rootEpsilon=1e-10; //end search if absolute difference of current and last root value is below epsilon
    Real functionEpsilon=1e-10; //end search if absolute difference of current and last function value is below epsilon
    Real gradientNormEpsilon=1e-6; //end search if absolute difference of norm of current and last gradient is below epsilon

    EndCriteria myEndCrit(maxIterations, minStatIterations, rootEpsilon,
      functionEpsilon, gradientNormEpsilon);

    //constraints
    LongOnlyConstraint longOnly;
    FullyInvestedConstraint fullyInvested;

    CompositeConstraint allConstraints(longOnly, fullyInvested);

    //would be better to start with variance weighted array but easier to debug and less inputs
    Problem myProb(ercFunc, allConstraints, Array(n, 2./n));

    LevenbergMarquardt solver;
    EndCriteria::Type solution=solver.minimize(myProb, myEndCrit);

    switch (solution) {
      case EndCriteria::None:
      case EndCriteria::MaxIterations:
      case EndCriteria::Unknown:
        throw("#err: optimization didn't converge - no solution found");
      default:;
    }

    Array x = myProb.currentValue();
    for (Size i = 0; i < x.size(); ++i) {
      weights[i][0] = x[i];
    }
  }

  return weights;
}
