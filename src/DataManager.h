#pragma once

#include "Product.h"
#include "StockChange.h"

#include <string>
#include <map>
#include <vector>
#include <array>

class DataManager
{
public:
	DataManager();
	~DataManager();

	void AddProduct(Product product);
	void DeleteProduct(uint32_t id);

	Product* SearchProductByBarcode(const std::string& code);
	Product* SearchProductByName(const std::string& name);
	Product* SearchProductByID(uint32_t id);

	int GetNumOfProducts() const;

	void AddStockChange(StockChange stockChange);
	void DeleteStockChange(const Date& dateKey);

	int GetNumOfStockChanges() const;

	const std::map<uint32_t, Product>& GetProducts() const;
	std::map<uint32_t, Product>& GetProducts();
	std::map<Date, StockChange>& GetUpdates();
	const std::map<Date, StockChange>& GetUpdates() const;

	static bool MatchNames(const std::string& str1, const std::string& str2);

	std::vector<StockChange*>& GetChangesIN() { return s_ChangesIN; }
	std::vector<StockChange*>& GetChangesOUT() { return s_ChangesOUT; }
	std::vector<StockChange*>& GetChangesANY() { return s_ChangesANY; }
private:
	std::vector<StockChange*> s_ChangesIN;
	std::vector<StockChange*> s_ChangesOUT;
	std::vector<StockChange*> s_ChangesANY;

	std::map<uint32_t, Product> m_Storage;
	std::map<Date, StockChange> m_Changes;

public:
	struct ProductStats
	{
		ProductStats()
		{
			product = nullptr;
			INCount = 0;
			OUTCount = 0;
			TotalBuyPrice = 0;
			TotalSellPrice = 0;
		}

		ProductStats(Product* pr)
		{
			product = pr;
			INCount = 0;
			OUTCount = 0;
			TotalBuyPrice = 0;
			TotalSellPrice = 0;
		}

		Product* product;
		uint32_t INCount, OUTCount;
		double TotalBuyPrice, TotalSellPrice;
	};

	struct MonthStat
	{
		std::map<uint32_t, ProductStats> Stats;
		double TotalBuyings, TotalSellings;
	};

	struct YearStat
	{
		std::array<MonthStat, 12> Months;
		double TotalBuyings, TotalSellings;
	};
private:
	std::map<int, YearStat> m_StatsPerYears;
public:
	void ClearStats();
	void CalculateStats();
	std::map<uint32_t, ProductStats> CalculateStats(const Date& beginning, const Date& ending);

	std::map<int, YearStat>& GetYearStats();
	const std::map<int, YearStat>& GetYearStats() const;

	std::map<int, ProductStats> GetStatsYear(Product* pr);
	std::map<std::pair<int,int>, ProductStats> GetStatsMonth(Product* pr);
	ProductStats GetStatsCostum(Product* pr, const Date& start, const Date& end);
};