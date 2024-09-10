#pragma once

#include "Product.h"
#include "StockChange.h"

struct ChangeData
{
	std::string Name, Barcode;
	int Count;
	Date Time;
	StockChangeType Type;

	void Set(StockChange* change);
	void Reset(StockChangeType type = StockChangeType::ANY);
};

struct ProductData
{
	std::string Name, Barcode, Usage;
	float BuyPrice, SellPrice;
	int Count = 0;
	uint32_t ID;

	void Set(Product* product);
	void Reset();
};