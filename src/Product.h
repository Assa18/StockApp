#pragma once

#include <string>

class Product
{
public:
	Product();
	Product(uint32_t id, const std::string& barcode, const std::string& name, int32_t count, float buyPrice, float sellPrice);
	~Product();

	uint32_t GetID() const;
	void SetID(uint32_t id);
	std::string GetBarcode() const;
	void SetBarcode(const std::string& barcode);
	std::string GetName() const;
	void SetName(const std::string& name);
	int GetCount() const;
	void SetCount(int count);
	float GetBuyPrice() const;
	void SetBuyPrice(float price);
	float GetSellPrice() const;
	void SetSellPrice(float price);

	void Set(const std::string& name, const std::string& barcode, int count, float buyPrice, float sellPrice);
	void Reset();

	static void SetLastID(uint32_t id);
	static uint32_t GetNewID();
private:

	uint32_t m_ID;
	std::string m_Barcode;
	std::string m_Name;
	int32_t m_Count;
	float m_BuyPrice;
	float m_SellPrice;
};

