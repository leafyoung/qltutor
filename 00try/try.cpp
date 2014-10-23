#include <cstddef>
#include <iostream>
#include <ql/QuantLib.hpp>

using namespace QuantLib;

int main() {
  Calendar calendar = UnitedStates(UnitedStates::GovernmentBond);
  const Natural settlementDays = 1;
  Date today(2, April, 2013); // Date::todaysDate();
  Date bondIssueDate = calendar.adjust(today, ModifiedFollowing);
  Date bondMauturiyDate = bondIssueDate + Period(3, Years);
  Rate rate = .03;

  std::cout << today << std::endl;
  std::cout << "Hello World" << std::endl;
  return 0;
}
