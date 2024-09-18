#include "DataManager.h"

#include <set>
#include <algorithm>

DataManager::DataManager()
{
}

DataManager::~DataManager()
{
}

void DataManager::AddProduct(Product product)
{
	m_Storage[product.GetID()] = product;
}

void DataManager::DeleteProduct(uint32_t id)
{
	Product* pr = &m_Storage[id];
	std::map<Date, StockChange> temp;
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if ((*it).second.GetProduct() != pr)
			temp[(*it).first] = (*it).second;
	}
	m_Changes.clear();
	for (it = temp.begin(); it != temp.end(); it++)
	{
		if ((*it).second.GetProduct() != pr)
			m_Changes[(*it).first] = (*it).second;
	}

	m_Storage.erase(id);

	FillChangesIN();
	FillChangesOUT();
	FillChangesANY();
	FillProductPtrs(false);
}

int DataManager::GetNumOfProducts() const
{
	return (int)m_Storage.size();
}

const std::map<uint32_t, Product>& DataManager::GetProducts() const
{
	return m_Storage;
}

std::map<uint32_t, Product>& DataManager::GetProducts()
{
	return m_Storage;
}

Product* DataManager::SearchProductByBarcode(const std::string& code)
{
	if (code == "") return nullptr;
	std::map<uint32_t, Product>::iterator it;
	for (it = m_Storage.begin(); it != m_Storage.end(); it++)
	{
		if ((*it).second.GetBarcode() == code)
		{
			return &(*it).second;
		}
	}
	return nullptr;
}

Product* DataManager::SearchProductByName(const std::string& name)
{
	std::map<uint32_t, Product>::iterator it;
	for (it = m_Storage.begin(); it != m_Storage.end(); it++)
	{
		if (DataManager::MatchNames(name, (*it).second.GetName()))
		{
			return &(*it).second;
		}
	}
	return nullptr;
}

Product* DataManager::SearchProductByID(uint32_t id)
{
	std::map<uint32_t, Product>::iterator it;
	for (it = m_Storage.begin(); it != m_Storage.end(); it++)
	{
		if ((*it).second.GetID() == id)
		{
			return &(*it).second;
		}
	}
	return nullptr;
}

void DataManager::SearchProductBarcode(std::vector<Product*>& source, const std::string& code)
{
	source.clear();
	Product* pr = SearchProductByBarcode(code);
	if (pr) source.push_back(pr);
}

void DataManager::SearchProductName(std::vector<Product*>& source, const std::string& name)
{
	source.clear();
	Product* pr = SearchProductByName(name);
	if (pr) source.push_back(pr);
}

void DataManager::AddStockChange(StockChange stockChange)
{
	m_Changes[stockChange.GetDate()] = stockChange;
}

void DataManager::DeleteStockChange(const Date& dateKey)
{
	int count = m_Changes[dateKey].GetCount();
	if (m_Changes[dateKey].GetType() == StockChangeType::IN)
	{
		m_Changes[dateKey].GetProduct()->SetCount(std::max(m_Changes[dateKey].GetProduct()->GetCount() - count, 0));
	}
	else
	{
		m_Changes[dateKey].GetProduct()->SetCount(m_Changes[dateKey].GetProduct()->GetCount() + count);
	}

	m_Changes.erase(dateKey);
}

int DataManager::GetNumOfStockChanges() const
{
	return (int)m_Changes.size();
}

std::map<Date, StockChange>& DataManager::GetUpdates()
{
	return m_Changes;
}


const std::map<Date, StockChange>& DataManager::GetUpdates() const
{
	return m_Changes;
}

void DataManager::FillChangesIN()
{
	m_ChangesIN.clear();
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if ((*it).second.GetType() == StockChangeType::IN)
		{
			m_ChangesIN.push_back(&(*it).second);
		}
	}
}

void DataManager::FillChangesOUT()
{
	m_ChangesOUT.clear();
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if ((*it).second.GetType() == StockChangeType::OUT)
		{
			m_ChangesOUT.push_back(&(*it).second);
		}
	}
}

void DataManager::FillChangesANY()
{
	m_ChangesANY.clear();
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		m_ChangesANY.push_back(&(*it).second);
	}
}

void DataManager::FillProductPtrs(bool onStock)
{
	m_ProductPtrs.clear();
	std::map<uint32_t, Product>::iterator it;
	for (it = m_Storage.begin(); it != m_Storage.end(); it++)
	{
		if (onStock && (*it).second.GetCount() > 0) m_ProductPtrs.push_back(&(*it).second);
		else if (!onStock) m_ProductPtrs.push_back(&(*it).second);
	}

	SortProductsByName();
}

void DataManager::SearchByName(std::vector<StockChange*>& source, StockChangeType type, const std::string& name)
{
	source.clear();
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if ( ((name == "") || DataManager::MatchNames(name, (*it).second.GetProduct()->GetName())) &&
			(type == StockChangeType::ANY || (*it).second.GetType() == type))
			source.push_back(&(*it).second);
	}
}

void DataManager::SearchByBarcode(std::vector<StockChange*>& source, StockChangeType type, const std::string& barcode)
{
	source.clear();
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if (((barcode == "") || (*it).second.GetProduct()->GetBarcode() == barcode) &&
			(type == StockChangeType::ANY || (*it).second.GetType() == type))
			source.push_back(&(*it).second);
	}
}

void DataManager::SearchByName(std::vector<StockChange*>& source, const Date& startDate, const Date& endDate,
	StockChangeType type, const std::string& name)
{
	source.clear();
	std::map<Date, StockChange>::reverse_iterator it;
	for (it = m_Changes.rbegin(); it != m_Changes.rend(); it++)
	{
		if ((*it).second.GetDate() >= startDate && (*it).second.GetDate() <= endDate &&
			((name == "") || DataManager::MatchNames(name, (*it).second.GetProduct()->GetName())) &&
			(type == StockChangeType::ANY || (*it).second.GetType() == type))
			source.push_back(&(*it).second);
	}
}

void DataManager::SearchByBarcode(std::vector<StockChange*>& source, const Date& startDate, const Date& endDate,
	StockChangeType type, const std::string& barcode)
{

	source.clear();
	std::map<Date, StockChange>::reverse_iterator it;
	for (it = m_Changes.rbegin(); it != m_Changes.rend(); it++)
	{
		if ((*it).second.GetDate() >= startDate && (*it).second.GetDate() <= endDate &&
			((barcode == "") || (*it).second.GetProduct()->GetBarcode() == barcode) &&
			(type == StockChangeType::ANY || (*it).second.GetType() == type))
			source.push_back(&(*it).second);
	}
}

void DataManager::SearchByDate(std::vector<StockChange*>& source, StockChangeType type, const std::string& date)
{
	source.clear();
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if (((date == "") || (*it).second.GetDate().Equals(Date::GetDateFromString(date))) &&
			(type == StockChangeType::ANY || (*it).second.GetType() == type))
			source.push_back(&(*it).second);
	}
}

void DataManager::CalculateStats()
{
	m_YearStats.clear();

	// Year stats
	std::map<Date, StockChange>::reverse_iterator it;
	for (it = m_Changes.rbegin(); it != m_Changes.rend(); it++)
	{
		if ((*it).second.GetType() == StockChangeType::IN)
		{
			m_YearStats[(*it).first.Year][(*it).second.GetProduct()].CountIN += (*it).second.GetCount();
			m_YearStats[(*it).first.Year][(*it).second.GetProduct()].ValueIN += (*it).second.GetCount() * (*it).second.GetProduct()->GetBuyPrice();
		}
		else
		{
			m_YearStats[(*it).first.Year][(*it).second.GetProduct()].CountOUT += (*it).second.GetCount();
			m_YearStats[(*it).first.Year][(*it).second.GetProduct()].ValueOUT += (*it).second.GetCount() * (*it).second.GetProduct()->GetSellPrice();
		}
	}

	m_MonthStats.clear();
	for (it = m_Changes.rbegin(); it != m_Changes.rend(); it++)
	{
		if ((*it).second.GetType() == StockChangeType::IN)
		{
			m_MonthStats[MonthPair((*it).first.Year, (*it).first.Month)][(*it).second.GetProduct()].CountIN +=
				(*it).second.GetCount();
			m_MonthStats[MonthPair((*it).first.Year, (*it).first.Month)][(*it).second.GetProduct()].ValueIN +=
				(*it).second.GetCount() * (*it).second.GetProduct()->GetBuyPrice();
		}
		else
		{
			m_MonthStats[MonthPair((*it).first.Year, (*it).first.Month)][(*it).second.GetProduct()].CountOUT +=
				(*it).second.GetCount();
			m_MonthStats[MonthPair((*it).first.Year, (*it).first.Month)][(*it).second.GetProduct()].ValueOUT +=
				(*it).second.GetCount() * (*it).second.GetProduct()->GetSellPrice();
		}
	}
}

void DataManager::CalculateStats(const Date& startDate, const Date& endDate)
{
	m_CostumStats.clear();
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if ((*it).second.GetDate() >= startDate && (*it).second.GetDate() <= endDate)
		{
			if ((*it).second.GetType() == StockChangeType::IN)
			{
				m_CostumStats[(*it).second.GetProduct()].CountIN += (*it).second.GetCount();
				m_CostumStats[(*it).second.GetProduct()].ValueIN += (*it).second.GetCount() * (*it).second.GetProduct()->GetBuyPrice();
			}
			else
			{
				m_CostumStats[(*it).second.GetProduct()].CountOUT += (*it).second.GetCount();
				m_CostumStats[(*it).second.GetProduct()].ValueOUT += (*it).second.GetCount() * (*it).second.GetProduct()->GetSellPrice();
			}
		}
	}
}

bool DataManager::MatchNames(const std::string& str1, const std::string& str2)
{
	if (str1 == str2) return true;
	std::string copy1 = str1, copy2 = str2;

	for (int i = 0; i < copy1.size(); i++)
	{
		copy1[i] = std::tolower(copy1[i]);
	}

	for (int i = 0; i < copy2.size(); i++)
	{
		copy2[i] = std::tolower(copy2[i]);
	}

	if (copy2.find(copy1) != std::string::npos)
		return true;

	return false;
}

void DataManager::SortProductsByName()
{
	for (int j = 1; j < m_ProductPtrs.size(); j++)
	{
		auto tmp = m_ProductPtrs[j];
		int i = j - 1;
		while (i >= 0 && Lower(m_ProductPtrs[i]->GetName(), tmp->GetName()))
		{
			m_ProductPtrs[i + 1] = m_ProductPtrs[i];
			i--;
		}
		m_ProductPtrs[i + 1] = tmp;
	}
}

bool DataManager::Lower(const std::string& name1, const std::string& name2)
{
	std::string tmp1 = name1, tmp2 = name2;

	std::transform(tmp1.begin(), tmp1.end(), tmp1.begin(), [](unsigned char c) {return std::tolower(c); });
	std::transform(tmp2.begin(), tmp2.end(), tmp2.begin(), [](unsigned char c) {return std::tolower(c); });

	return tmp1 > tmp2;
}