#pragma once

#include "Product.h"
#include <ctime>

enum class StockChangeType
{
	ANY = 0, IN, OUT
};

enum class TimeRanges
{
	ALL = 0, TODAY, YESTERDAY, THIS_WEEK, LAST_WEEK, THIS_MONTH, LAST_MONTH, THIS_YEAR, LAST_YEAR, COSTUM
};

class Date
{
public:
	int Year, Month, Day, Hour, Min, Sec;

	Date()
		:Year(2000), Month(1), Day(1), Hour(0), Min(0), Sec(0)
	{
	}
	
	Date(int year, int month, int day, int h, int min, int sec)
		:Year(year), Month(month), Day(day), Hour(h), Min(min), Sec(sec)
	{
	}

	void Set(int year, int month, int day, int h, int min, int sec);

	std::string ToString() const
	{
		return std::to_string(Year) + "." + std::to_string(Month) + "." + std::to_string(Day) + " " +
			std::to_string(Hour) + ":" + std::to_string(Min) + ":" + std::to_string(Sec);
	}

	bool Equals(const Date& date) const;

	static Date GetCurrrentDate();
	static Date GetDateFromString(const std::string& str);
	static int GetCurrentDayOfWeek();
	static int GetCurrentDayOfMonth();
	static int GetDayOfMonth(int month);

	inline Date& operator =(const Date& date)
	{
		Year = date.Year;
		Month = date.Month;
		Day = date.Day;
		Hour = date.Hour;
		Min = date.Min;
		Sec = date.Sec;

		return *this;
	}

	inline bool operator < (const Date& date) const
	{
		if (this->Year < date.Year) return true;
		if (this->Year == date.Year && this->Month < date.Month) return true;
		if (this->Year == date.Year && this->Month == date.Month && this->Day < date.Day) return true;
		if (this->Year == date.Year && this->Month == date.Month && this->Day == date.Day && this->Hour < date.Hour) return true;
		if (this->Year == date.Year && this->Month == date.Month && this->Day == date.Day && this->Hour == date.Hour && this->Min < date.Min) return true;
		if (this->Year == date.Year && this->Month == date.Month && this->Day == date.Day && this->Hour == date.Hour && this->Min == date.Min && this->Sec<date.Sec) return true;
		return false;
	}

	inline bool operator <= (const Date& date) const
	{
		return (*this < date || *this == date);
	}

	inline bool operator > (const Date& date) const
	{
		if (this->Year > date.Year) return true;
		if (this->Year == date.Year && this->Month > date.Month) return true;
		if (this->Year == date.Year && this->Month == date.Month && this->Day > date.Day) return true;
		if (this->Year == date.Year && this->Month == date.Month && this->Day == date.Day && this->Hour > date.Hour) return true;
		if (this->Year == date.Year && this->Month == date.Month && this->Day == date.Day && this->Hour == date.Hour && this->Min > date.Min) return true;
		if (this->Year == date.Year && this->Month == date.Month && this->Day == date.Day && this->Hour == date.Hour && this->Min == date.Min && this->Sec > date.Sec) return true;
		return false;
	}

	inline bool operator >= (const Date& date) const
	{
		return (*this > date || *this == date);
	}

	inline bool operator ==(const Date& date) const
	{
		return (this->Year == date.Year && this->Month == date.Month && this->Day == date.Day && this->Hour == date.Hour && this->Min == date.Min && this->Sec == date.Sec);
	}

	inline bool operator !=(const Date& date) const
	{
		return (this->Year != date.Year || this->Month != date.Month || this->Day != date.Day || this->Hour != date.Hour || this->Min != date.Min || this->Sec != date.Sec);
	}

	inline Date operator --(int)
	{
		Day--;
		if (Day == 0)
		{
			Month--;
			if (Month == 0)
			{
				Year--;
				Month = 12;
				Day = 31;
			}
			else Day = m_DaysOfMonth[Month - 1];
		}
		return *this;
	}

	inline Date operator ++(int)
	{
		Day++;
		if (Day > m_DaysOfMonth[Month - 1])
		{
			Month++;
			if (Month == 13)
			{
				Year++;
				Month = 1;
				Day = 1;
			}
			else Day = m_DaysOfMonth[Month - 1];
		}
		return *this;
	}

private:
	const int m_DaysOfMonth[12] = { 31,29,31,30,31,30,31,31,30,31,30,31 };
};

class StockChange
{
public:
	StockChange();
	StockChange(const StockChange& change);
	StockChange(Product* pr, const Date& date, int count = 1, StockChangeType type = StockChangeType::ANY);
	~StockChange();

	Product* GetProduct() const;
	Date GetDate() const;
	StockChangeType GetType() const;
	int GetCount() const;

	void SetProduct(Product* product);
	void SetDate(const Date& date);
	void SetType(StockChangeType type);
	void SetCount(int count);

	void Set(Product* prPtr, int count, StockChangeType type);
private:
	Product* m_Product;
	Date m_Date;
	int m_Count;

	StockChangeType m_Type;
};