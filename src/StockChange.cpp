#define _CRT_SECURE_NO_WARNINGS
#include "StockChange.h"

#include <string>
#include <sstream>

StockChange::StockChange()
{

}

StockChange::StockChange(const StockChange& change)
{
	m_Type = change.m_Type;
	m_Date = change.m_Date;
	m_Count = change.m_Count;
	m_Product = change.m_Product;
}

StockChange::StockChange(Product* pr, const Date& date, int count, StockChangeType type)
	:m_Date(date), m_Count(count), m_Type(type)
{
	m_Product = pr;
}

StockChange::~StockChange()
{

}

Product* StockChange::GetProduct() const
{
	return m_Product;
}

Date StockChange::GetDate() const
{
	return m_Date;
}

StockChangeType StockChange::GetType() const
{
	return m_Type;
}

int StockChange::GetCount() const
{
	return m_Count;
}

void StockChange::SetProduct(Product* product)
{
	m_Product = product;
}

void StockChange::SetDate(const Date& date)
{
	m_Date = date;
}

void StockChange::SetType(StockChangeType type)
{
	m_Type = type;
}

void StockChange::SetCount(int count)
{
	m_Count = count;
}

void StockChange::Set(Product* prPtr, int count, StockChangeType type)
{
	m_Product = prPtr;
	m_Count = count;
	m_Type = type;
}

Date Date::GetCurrrentDate()
{
	time_t t = time(NULL);
	tm* tPtr = localtime(&t);

	int year = (tPtr->tm_year) + 1900;
	int month = (tPtr->tm_mon) + 1;
	int day = (tPtr->tm_mday);
	int hour = (tPtr->tm_hour);
	int min = (tPtr->tm_min);
	int sec = (tPtr->tm_sec);

	return Date(year, month, day, hour, min, sec);
}

void Date::Set(int year, int month, int day, int h, int min, int sec)
{
	Year = year;
	Month = month;
	Day = day;
	Hour = h;
	Min = min;
	Sec = sec;
}

bool Date::Equals(const Date& date) const
{
	return (Year == date.Year && Month == date.Month && Day == date.Day);
}

Date Date::GetDateFromString(const std::string& str)
{
	Date dateTmp;
	std::stringstream ss(str);
	std::string date, time, input;
	std::getline(ss, date, ' ');
	std::getline(ss, time, ' ');

	std::stringstream ssDate(date);
	std::getline(ssDate, input, '.');
	dateTmp.Year = std::stoi(input);
	std::getline(ssDate, input, '.');
	dateTmp.Month = std::stoi(input);
	std::getline(ssDate, input, '.');
	dateTmp.Day = std::stoi(input);

	if (time != "")
	{
		std::stringstream ssTime(time);
		std::getline(ssTime, input, ':');
		dateTmp.Hour = std::stoi(input);
		std::getline(ssTime, input, ':');
		dateTmp.Min = std::stoi(input);
		std::getline(ssTime, input, ':');
		dateTmp.Sec = std::stoi(input);
	}

	return dateTmp;
}

int Date::GetCurrentDayOfWeek()
{
	time_t t = time(NULL);
	tm* tPtr = localtime(&t);

	return (tPtr->tm_wday == 0 ? 7 : tPtr->tm_wday);
}

int Date::GetCurrentDayOfMonth()
{
	time_t t = time(NULL);
	tm* tPtr = localtime(&t);

	return (tPtr->tm_mday);
}

int Date::GetDayOfMonth(int month)
{
	switch (month)
	{
	case 2:
		return 29;
		break;
	case 4: case 6: case 9: case 11:
		return 30;
		break;
	case 1: case 3: case 5: case 7: case 8: case 10: case 12:
		return 31;
		break;
	default:
		break;
	}
}