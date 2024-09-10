#pragma once

#include <string>

class Product
{
public:
	Product();
	Product(uint32_t id, const std::string& barcode, const std::string& name, int32_t count, float buyPrice, float sellPrice,
		const std::string& usage);
	~Product();

	uint32_t GetID() const;
	void SetID(uint32_t id);
	std::string& GetBarcode();
	void SetBarcode(const std::string& barcode);
	std::string& GetUsage();
	void SetUsage(const std::string& usage);
	std::string& GetName();
	void SetName(const std::string& name);
	int& GetCount();
	void SetCount(int count);
	float& GetBuyPrice();
	void SetBuyPrice(float price);
	float& GetSellPrice();
	void SetSellPrice(float price);

	void Set(const std::string& name, const std::string& barcode, int count, float buyPrice, float sellPrice, const std::string& usage);
	void Reset();

	static void SetLastID(uint32_t id);
	static uint32_t GetNewID();
private:

	uint32_t m_ID;
	std::string m_Barcode;
	std::string m_Name;
	std::string m_Usage;
	int32_t m_Count;
	float m_BuyPrice;
	float m_SellPrice;
};

