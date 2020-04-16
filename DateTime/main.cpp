#include "DateTime.h"

int main(int argc, char *argv[])
{
	DateTime now = DateTime::Now();
	DateTime today = DateTime::Today();
	DateTime utc = DateTime::UtcNow();
	DateTime local = utc.ToLocalTime();
	DateTime universal = now.ToUniversalTime();
	std::string longDateString = now.ToLongDateString();
	std::string longTimeString = now.ToLongTimeString();
	std::string shortDateString = now.ToShortDateString();
	std::string shortTimeString = now.ToShortTimeString();
	std::string strDateTime = now.ToString("%Y-%m-%d %H:%M:%S %a");

	TimeSpan::MaxValue;
	TimeSpan::MinValue;
	TimeSpan timeSpan0(1, 2, 3, 4, 5);
	TimeSpan timeSpan1(6, 7, 8, 9, 10);
	TimeSpan timeSpan2(3600 * 1000);
	TimeSpan aaa = timeSpan0 - timeSpan1;
	timeSpan0.Subtract(timeSpan1);
	TimeSpan duration = aaa.Duration();
	TimeSpan negate = aaa.Negate();
	return 0;
}