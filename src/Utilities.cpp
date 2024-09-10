#include "Utilities.h"


void ChangeData::Set(StockChange* change)
{
	Name = change->GetProduct()->GetName();
	Barcode = change->GetProduct()->GetBarcode();
	Count = change->GetCount();
	Time = change->GetDate();
	Type = change->GetType();
}

void ChangeData::Reset(StockChangeType type)
{
	Name.clear();
	Barcode.clear();
	Count = 0;
	Time = Date::GetCurrrentDate();
	Type = type;
}

void ProductData::Set(Product* product)
{
	ID = product->GetID();
	Name = product->GetName();
	Barcode = product->GetBarcode();
	BuyPrice = product->GetBuyPrice();
	SellPrice = product->GetSellPrice();
	Count = product->GetCount();
}

void ProductData::Reset()
{
	ID = 0;
	Name.clear();
	Barcode.clear();
	BuyPrice = 0.0f;
	SellPrice = 0.0f;
	Count = 0;
}