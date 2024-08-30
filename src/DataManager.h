#pragma once

#include "Product.h"
#include "StockChange.h"

#include <string>
#include <map>
#include <vector>
#include <array>

struct ProductStats
{
	Date StartDate, EndDate;
	int CountIN, CountOUT;
	double ValueIN, ValueOUT;
};

struct YearStats
{
	uint32_t Year;
	double ValueIN, ValueOUT;
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
	std::map<Product*, ProductStats>& GetMonthStats() { return m_MonthStats; }
	std::map<int, std::map<Product*, ProductStats>>& GetYearStats() { return m_YearStats; }
	std::map<Product*, ProductStats>& GetCostumStats() { return m_CostumStats; }

	void CalculateStats();
	void CalculateStats(const Date& startDate, const Date& endDate);
private:
	std::vector<StockChange*> m_ChangesIN;
	std::vector<StockChange*> m_ChangesOUT;
	std::vector<StockChange*> m_ChangesANY;

	std::vector<Product*> m_ProductPtrs;

	std::map<uint32_t, Product> m_Storage;
	std::map<Date, StockChange> m_Changes;

	//stats
	std::map<Product*, ProductStats> m_MonthStats;
	std::map<int, std::map<Product*, ProductStats>> m_YearStats;
	std::map<Product*, ProductStats> m_CostumStats;
};