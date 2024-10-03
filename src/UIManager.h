#pragma once

#include "DataManager.h"
#include "Utilities.h"

#include <ImGui/imgui.h>

#include <vector>

namespace ImGui
{
	IMGUI_API bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback = nullptr, void* user_data = nullptr);
}

enum class Colors {
	Black = 0, White, Gray, Beige, Blue, Turkiz
};

enum class UIStates {
	NoState = 0, Settings, Overall, AddChange, EditChange, AddProduct, EditProduct
};

class UIManager
{
public:
	UIManager();
	~UIManager();

	void SetDataManager(DataManager* dataM);
	void Update();
	void SetupStyle();
private:
	void SettingsWindow();
	void OverallWindow();

	void AddChangeWindow();
	void EditChangeWindow();

	void AddProductWindow();
	void EditProductWindow();

	void WhatToShow(int num, const char* texts[], bool booleans[]);
	void ShowTable(const std::vector<StockChange*>& source, int num, const char* texts[], bool showTexts[], StockChangeType type);
	void ShowTable(const std::vector<Product*>& source, int num, const char* texts[], bool showTexts[]);
	void ShowSearch(std::vector<StockChange*> &source, StockChangeType type, std::string& searchString);
	void ShowSearchAll();
	void ShowSearch(std::vector<Product*>& source, std::string& searchString);

	void SetTimeRange();

	void DisplayYearStats();
	void DisplayMonthStats();
	void DisplayCostumStats(float& buyings, float& sellings);
private:
	friend class Serializer;
	DataManager* m_DataM;
	float m_FontSize = 20.0f;
	UIStates m_State = UIStates::NoState;

	const char* m_TextsChange[4] = { u8"Barcode", u8"Name", u8"Count", u8"Date" };
	bool m_ShowTextsChange[4] = { true,true,true,true };

	const char* m_TextsAllChange[5] = { u8"Barcode", u8"Name", u8"Type", u8"Count", u8"Date"};
	bool m_ShowTextsAllChange[5] = { true,true,true,true,true };

	const char* m_TextsProduct[6] = { u8"Barcode", u8"Name", u8"Count", u8"Buy price", u8"Sell price", u8"Description"};
	bool m_ShowTextsProduct[6] = { true,true,true,true,true,true };

	const char* m_TypeNames[3] = { u8"All", u8"Buy", u8"Sell" };

	const char* m_TimeRangeNames[10] = {u8"All", u8"Today", u8"Yesterday", u8"This week", u8"Last week", u8"This month", 
		u8"Last month", u8"This year", u8"Last year", u8"Costum"};

	const char* m_OverallTexts[8] = { u8"Barcode", u8"Name", u8"Buy price", u8"In Count", u8"Total buyings",
		u8"Sell price", u8"Out count", u8"Total sellings" };
	bool m_ShowOverallTexts[8] = { true,true,true,true,true,true,true,true };

	std::string m_MonthNames[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September",
	"October", "November", "December" };

	std::string m_SearchStringIN;
	std::string m_SearchStringOUT;
	std::string m_SearchStringANY;
	std::string m_SearchStringPr;
	std::string m_SearchStringOverall;

	ChangeData m_ChangeData;
	ProductData m_ProductData;

	Product* m_Productptr;
	bool m_OnStock = false;

	int m_TimeRange = 0;
	int m_SearchType = 0;
	Date m_StartDate, m_EndDate;
};