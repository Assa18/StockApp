#pragma once

#include "Product.h"
#include "StockChange.h"

#include <string>
#include <map>
#include <vector>
#include <array>

struct ProductStats
{
	int CountIN, CountOUT;
	double ValueIN, ValueOUT;
};

struct MonthPair
{
	int Year, Month;

	MonthPair()
		:Year(0), Month(0)
	{}
	MonthPair(int year, int month)
	:Year(year), Month(month)
	{}

	bool operator < (const MonthPair& other) const
	{
		if (this->Year < other.Year) return true;
		if (this->Year == other.Year && this->Month < other.Month) return true;
		return false;
	}
};

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

	void SearchProductBarcode(std::vector<Product*>& source, const std::string& code);
	void SearchProductName(std::vector<Product*>& source, const std::string& name);

	int GetNumOfProducts() const;

	void AddStockChange(StockChange stockChange);
	void DeleteStockChange(const Date& dateKey);

	int GetNumOfStockChanges() const;

	const std::map<uint32_t, Product>& GetProducts() const;
	std::map<uint32_t, Product>& GetProducts();
	std::map<Date, StockChange>& GetUpdates();
	const std::map<Date, StockChange>& GetUpdates() const;

	static bool MatchNames(const std::string& str1, const std::string& str2);

	std::vector<StockChange*>& GetChangesIN() { return m_ChangesIN; }
	std::vector<StockChange*>& GetChangesOUT() { return m_ChangesOUT; }
	std::vector<StockChange*>& GetChangesANY() { return m_ChangesANY; }
	std::vector<Product*>& GetProductPtrs() { return m_ProductPtrs; }

	void FillChangesIN();
	void FillChangesOUT();
	void FillChangesANY();
	void FillProductPtrs(bool onStock);

	void SearchByName(std::vector<StockChange*>& source, StockChangeType type, const std::string& name);
	void SearchByBarcode(std::vector<StockChange*>& source,	StockChangeType type, const std::string& barcode);
	
	void SearchByName(std::vector<StockChange*>& source, const Date& startDate, const Date& endDate, 
		StockChangeType type, const std::string& name);
	void SearchByBarcode(std::vector<StockChange*>& source, const Date& startDate, const Date& endDate, 
		StockChangeType type, const std::string& barcode);
	void SearchByDate(std::vector<StockChange*>& source, StockChangeType type, const std::string& date);

	//stats
	std::map<MonthPair, std::map<Product*, ProductStats>>& GetMonthStats() { return m_MonthStats; }
	std::map<int, std::map<Product*, ProductStats>>& GetYearStats() { return m_YearStats; }
	std::map<Product*, ProductStats>& GetCostumStats() { return m_CostumStats; }

	void CalculateStats();
	void CalculateStats(const Date& startDate, const Date& endDate);

	void SortProductsByName();
private:
	std::vector<StockChange*> m_ChangesIN;
	std::vector<StockChange*> m_ChangesOUT;
	std::vector<StockChange*> m_ChangesANY;

	std::vector<Product*> m_ProductPtrs;

	std::map<uint32_t, Product> m_Storage;
	std::map<Date, StockChange> m_Changes;

	//stats
	std::map<MonthPair, std::map<Product*, ProductStats>> m_MonthStats;
	std::map<int, std::map<Product*, ProductStats>> m_YearStats;
	std::map<Product*, ProductStats> m_CostumStats;

	bool Lower(const std::string& name1, const std::string& name2);
};