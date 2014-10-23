#include <ql/quantlib.hpp>
#include <vector>

namespace {
  const QuantLib::Real rootEpisilon = 1e-8;
}

// http://www.thierry-roncalli.com/download/erc.pdf%E2%80%8E

// mean variance objective function
class MeanVarianceFunction: public QuantLib::CostFunction {
  private:
    QuantLib::Matrix covariance_;
    QuantLib::Size n;

  public:
    MeanVarianceFunction(const QuantLib::Matrix& covariance)
      : covariance_(covariance) {
      n = covariance.rows();
    }

    QuantLib::Real value(const QuantLib::Array& x) const {
      QL_REQUIRE(x.size()==n-1, "n - 1 weights required");
      QuantLib::Matrix weights(n,1);
      QuantLib::Real sumWeights(0.);

      for (QuantLib::Size i = 0; i < x.size(); ++i) {
        sumWeights += x[i];
        weights[i][0] = x[i];
      }

      weights[n-1][0] = 1 - sumWeights;

      QuantLib::Matrix var = transpose(weights) * covariance_ * weights;

      return var[0][0];
    }

    QuantLib::Disposable<QuantLib::Array> values(const QuantLib::Array& x) const {
      QuantLib::Array var(1,value(x));
      return var;
    }
};

class EqualRiskContributionFunction : public QuantLib::CostFunction {
  private:
    QuantLib::Matrix covariance_;
    QuantLib::Size n;

  public:
    EqualRiskContributionFunction(const QuantLib::Matrix& covariance)
      : covariance_(covariance)
    {
      n = covariance.rows();
    }

    QuantLib::Real value(const QuantLib::Array& x) const{

    QuantLib::Array dsx = values(x);
    QuantLib::Real res(0.);

    for (QuantLib::Size i = 0; i < n; ++i) {
      for(QuantLib::Size j = 0; j < i; ++j) {
        res += (dsx[i] - dsx[j]) * (dsx[i] - dsx[j]);
      }
    }

    return res;
  }

  QuantLib::Disposable<QuantLib::Array> values(const QuantLib::Array& x) const {
    QuantLib::Array dsx(n,0.);
    for (QuantLib::Size i = 0; i < n; ++i) {
      QuantLib::Array row(covariance_.row_begin(i), covariance_.row_end(i));
      dsx[i] = QuantLib::DotProduct(row, x) * x[i];
      double test = dsx[i];
    }
    return dsx;
  }
};

// long only constraint
class LongOnlyConstraint : public QuantLib::Constraint {
  private :
    class Impl : public QuantLib::Constraint::Impl {
      public :
        Impl() {}
        bool test(const QuantLib::Array& weights) const {
          QuantLib::Real sumWeights(0.);

          for (QuantLib::Size i=0; i<weights.size(); i++) {
            if (weights[i] < 0 - ::rootEpisilon) {
              return false;
            }
            sumWeights += weights[i];
          }

          // n'th asset weight equals 1 - sum(asset 1 …. assent n -1)
          if (1-sumWeights < 0 - ::rootEpisilon) {
            return false;
          }

          return true;
        }
      private:
    };

  public:
    LongOnlyConstraint() : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl> (new LongOnlyConstraint::Impl)) {}
};

// fully invested constraint
class FullyInvestedConstraint : public QuantLib::Constraint {
  private:
    class Impl : public QuantLib::Constraint::Impl {
      public:
        Impl() {}
        bool test(const QuantLib::Array& weights) const {
          QuantLib::Real sumWeights(0.);

          for (QuantLib::Size i=0; i<weights.size(); i++) {
            sumWeights += weights[i];
          }

          QuantLib::Real weightN = 1. - sumWeights;

          // check minimum leverage
          if (1. - (sumWeights + weightN) < -::rootEpisilon) {
            return false;
          }

          return true;
        }
      private:
    };
  public:
    FullyInvestedConstraint () : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl> (new FullyInvestedConstraint::Impl)) {}
};


