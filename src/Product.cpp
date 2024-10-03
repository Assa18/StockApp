#include "Product.h"

Product::Product()
{
	m_ID = 0;
	m_Count = 0;
	m_BuyPrice = 0.0f;
	m_SellPrice = 0.0f;
}

Product::Product(uint32_t id, const std::string& barcode, const std::string& name, int count, float buyPrice, float sellPrice,
	const std::string& usage)
{
	m_ID = id;
	m_Barcode = barcode;
	m_Name = name;
	m_Count = count;
	m_BuyPrice = buyPrice;
	m_SellPrice = sellPrice;
	m_Usage = usage;
}

Product::~Product()
{
}

uint32_t Product::GetID() const
{
	return m_ID;
}

void Product::SetID(uint32_t id)
{
	m_ID = id;
}

std::string& Product::GetBarcode()
{
	return m_Barcode;
}

void Product::SetBarcode(const std::string& barcode)
{
	m_Barcode = barcode;
}

std::string& Product::GetUsage()
{
	return m_Usage;
}

void Product::SetUsage(const std::string& usage)
{
	m_Usage = usage;
}

std::string& Product::GetName()
{
	return m_Name;
}

void Product::SetName(const std::string& name)
{
	m_Name = name;
}

int& Product::GetCount()
{
	return m_Count;
}

void Product::SetCount(int count)
{
	m_Count = count;
}

float& Product::GetBuyPrice()
{
	return m_BuyPrice;
}

void Product::SetBuyPrice(float price)
{
	m_BuyPrice = price;
}

float& Product::GetSellPrice()
{
	return m_SellPrice;
}

void Product::SetSellPrice(float price)
{
	m_SellPrice = price;
}

void Product::Reset()
{
	m_Name = "";
	m_Barcode = "";
	m_Count = 0;
	m_BuyPrice = 0.0f;
	m_SellPrice = 0.0f;
	m_Usage = "";
}

static uint32_t s_LastID = 0;

void Product::SetLastID(uint32_t id)
{
	if (s_LastID < id) s_LastID = id;
}

uint32_t Product::GetNewID()
{
	s_LastID++;
	return s_LastID;
}

void Product::Set(const std::string& name, const std::string& barcode, int count, float buyPrice, float sellPrice, const std::string& usage)
{
	m_Name = name;
	m_Barcode = barcode;
	m_Count = count;
	m_BuyPrice = buyPrice;
	m_SellPrice = sellPrice;
	m_Usage = usage;
}