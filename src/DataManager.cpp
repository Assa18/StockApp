#include "DataManager.h"

DataManager::DataManager()
	:m_TVA(19.0f)
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
	//TODO: fix deleteing, find a way, or leave it

	/*Product* pr = &m_Storage[id];
	std::map<Date, StockChange>::iterator it;
	it = m_Changes.begin();
	while (it != m_Changes.end())
	{
		if ((*it).second.GetProduct() == pr)
			m_Changes.erase((*it).second.GetDate());
		else it++;
	}*/

	m_Storage.erase(id);
}

int DataManager::GetNumOfProducts() const
{
	return m_Storage.size();
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
		if ((*it).second.GetName() == name)
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

void DataManager::AddStockChange(StockChange stockChange)
{
	m_Changes[stockChange.GetDate()] = stockChange;
}

void DataManager::DeleteStockChange(const Date& dateKey)
{
	m_Changes.erase(dateKey);
}

int DataManager::GetNumOfStockChanges() const
{
	return m_Changes.size();
}

std::map<Date, StockChange>& DataManager::GetUpdates()
{
	return m_Changes;
}


const std::map<Date, StockChange>& DataManager::GetUpdates() const
{
	return m_Changes;
}

void DataManager::CalculateStats()
{
	ClearStats();
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if (m_StatsPerYears[(*it).second.GetDate().Year].Months[(*it).second.GetDate().Month - 1].Stats[(*it).second.GetProduct()->GetID()].product == nullptr)
			m_StatsPerYears[(*it).second.GetDate().Year].Months[(*it).second.GetDate().Month - 1].Stats[(*it).second.GetProduct()->GetID()].product = (*it).second.GetProduct();
		if ((*it).second.GetType() == StockChangeType::IN)
		{
			m_StatsPerYears[(*it).second.GetDate().Year].Months[(*it).second.GetDate().Month - 1].Stats[(*it).second.GetProduct()->GetID()].INCount += (*it).second.GetCount();
			m_StatsPerYears[(*it).second.GetDate().Year].Months[(*it).second.GetDate().Month - 1].Stats[(*it).second.GetProduct()->GetID()].TotalBuyPrice += (*it).second.GetCount() * (*it).second.GetProduct()->GetBuyPrice();
		}
		else
		{
			m_StatsPerYears[(*it).second.GetDate().Year].Months[(*it).second.GetDate().Month - 1].Stats[(*it).second.GetProduct()->GetID()].OUTCount += (*it).second.GetCount();
			m_StatsPerYears[(*it).second.GetDate().Year].Months[(*it).second.GetDate().Month - 1].Stats[(*it).second.GetProduct()->GetID()].TotalSellPrice += (*it).second.GetCount() * (*it).second.GetProduct()->GetSellPrice();
		}
	}

	std::map<int, YearStat>::iterator itYears;
	for (itYears = m_StatsPerYears.begin(); itYears != m_StatsPerYears.end(); itYears++)
	{
		for (int j = 0; j < 12; j++)
		{
			std::map<uint32_t, ProductStats>::iterator itProduct;
			for (itProduct = (*itYears).second.Months[j].Stats.begin(); itProduct != (*itYears).second.Months[j].Stats.end(); itProduct++)
			{
				(*itYears).second.Months[j].TotalBuyings += (*itProduct).second.TotalBuyPrice;
				(*itYears).second.Months[j].TotalSellings += (*itProduct).second.TotalSellPrice;
			}
			(*itYears).second.TotalBuyings += (*itYears).second.Months[j].TotalBuyings;
			(*itYears).second.TotalSellings += (*itYears).second.Months[j].TotalSellings;
		}
	}
}

std::map<uint32_t, DataManager::ProductStats> DataManager::CalculateStats(const Date& beginning, const Date& ending)
{
	std::map<uint32_t, DataManager::ProductStats> stats;
	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if ((*it).second.GetDate() >= beginning && (*it).second.GetDate() <= ending)
		{
			if (stats[(*it).second.GetProduct()->GetID()].product == nullptr)
				stats[(*it).second.GetProduct()->GetID()].product = (*it).second.GetProduct();
			if ((*it).second.GetType() == StockChangeType::IN) stats[(*it).second.GetProduct()->GetID()].INCount += (*it).second.GetCount();
			else stats[(*it).second.GetProduct()->GetID()].OUTCount += (*it).second.GetCount();
		}
	}

	std::map<uint32_t, DataManager::ProductStats>::iterator itStat;
	for (itStat = stats.begin(); itStat != stats.end(); itStat++)
	{
		(*itStat).second.TotalBuyPrice = (*itStat).second.INCount * m_Storage[(*itStat).first].GetBuyPrice();
		(*itStat).second.TotalSellPrice = (*itStat).second.OUTCount * m_Storage[(*itStat).first].GetSellPrice();
	}

	return stats;
}

void DataManager::ClearStats()
{
	std::map<int, YearStat>::iterator it;
	for (it = m_StatsPerYears.begin(); it != m_StatsPerYears.end(); it++)
	{
		(*it).second.TotalBuyings = 0;
		(*it).second.TotalSellings = 0;
		for (int j = 0; j < 12; j++)
		{
			(*it).second.Months[j].TotalBuyings = 0;
			(*it).second.Months[j].TotalSellings = 0;
			(*it).second.Months[j].Stats.clear();
		}
	}
}

std::map<int, DataManager::YearStat>& DataManager::GetYearStats()
{
	return m_StatsPerYears;
}

const std::map<int, DataManager::YearStat>& DataManager::GetYearStats() const
{
	return m_StatsPerYears;
}

std::map<int, DataManager::ProductStats> DataManager::GetStatsYear(Product* pr)
{
	std::map<int, ProductStats> mapStats;

	std::map<int, YearStat>::iterator it;
	for (it = m_StatsPerYears.begin(); it != m_StatsPerYears.end(); it++)
	{
		if (mapStats[(*it).first].product == nullptr) mapStats[(*it).first].product = pr;
		for (int i = 0; i < 12; i++)
		{
			mapStats[(*it).first].INCount += (*it).second.Months[i].Stats[pr->GetID()].INCount;
			mapStats[(*it).first].OUTCount += (*it).second.Months[i].Stats[pr->GetID()].OUTCount;
			mapStats[(*it).first].TotalBuyPrice += (*it).second.Months[i].Stats[pr->GetID()].TotalBuyPrice;
			mapStats[(*it).first].TotalSellPrice += (*it).second.Months[i].Stats[pr->GetID()].TotalSellPrice;
		}
	}

	return mapStats;
}

std::map<std::pair<int,int>, DataManager::ProductStats> DataManager::GetStatsMonth(Product* pr)
{
	std::map<std::pair<int, int>, ProductStats> mapStats;

	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		std::pair<int, int> tmpPair = std::make_pair((*it).first.Year, (*it).first.Month);
		if (mapStats[tmpPair].product == nullptr) mapStats[tmpPair].product = pr;
		if ((*it).second.GetType() == StockChangeType::IN)
		{
			mapStats[tmpPair].INCount += (*it).second.GetCount();
			mapStats[tmpPair].TotalBuyPrice += (*it).second.GetCount() * (*it).second.GetProduct()->GetBuyPrice();
		}
		else
		{
			mapStats[tmpPair].OUTCount += (*it).second.GetCount();
			mapStats[tmpPair].TotalSellPrice += (*it).second.GetCount() * (*it).second.GetProduct()->GetSellPrice();
		}
	}

	return mapStats;
}

DataManager::ProductStats DataManager::GetStatsCostum(Product* pr, const Date& start, const Date& end)
{
	ProductStats stats;

	std::map<Date, StockChange>::iterator it;
	for (it = m_Changes.begin(); it != m_Changes.end(); it++)
	{
		if ((*it).second.GetProduct() == pr && (*it).second.GetDate() >= start && (*it).second.GetDate() <= end)
		{
			if (stats.product == nullptr) stats.product = pr;
			if ((*it).second.GetType() == StockChangeType::IN)
			{
				stats.INCount += (*it).second.GetCount();
				stats.TotalBuyPrice += (*it).second.GetCount() * (*it).second.GetProduct()->GetBuyPrice();
			}
			else
			{
				stats.OUTCount += (*it).second.GetCount();
				stats.TotalSellPrice += (*it).second.GetCount() * (*it).second.GetProduct()->GetSellPrice();
			}
		}
	}

	return stats;
}