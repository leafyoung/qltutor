#include <ql/quantlib.hpp>
#include <iostream>

#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN

#define BOOST_TEST_MODULE FV

#include <boost/test/unit_test.hpp>

using namespace QuantLib;

BOOST_AUTO_TEST_CASE(testCalculateLoanPayment)
{
  typedef FixedRateCoupon Loan;
  Natural lengthOfLoan = 365;
  Real loanAmount = 100.0;
  Rate rate = 0.05;
  Date today = Date::todaysDate();
  Date paymentDate = today + lengthOfLoan;
  Loan loan(paymentDate, loanAmount, rate, ActualActual(), today, paymentDate);
  Real payment = loanAmount + loan.accruedAmount(paymentDate);
  std::cout << "Payment due in " << lengthOfLoan << " days on loan amount of $ "
            << loanAmount << " at annual rate " << rate * 100 << "% is: $" << payment << std::endl;

}
