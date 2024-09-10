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

	const char* m_TextsChange[4] = { u8"Vonalk�d", u8"N�v", u8"Darabsz�m", u8"D�tum" };
	bool m_ShowTextsChange[4] = { true,true,true,true };

	const char* m_TextsAllChange[5] = { u8"Vonalk�d", u8"N�v", u8"T�pus", u8"Darabsz�m", u8"D�tum"};
	bool m_ShowTextsAllChange[5] = { true,true,true,true,true };

	const char* m_TextsProduct[5] = { u8"Vonalk�d", u8"N�v", u8"Darabsz�m", u8"V�tel �r", u8"Elad�si �r" };
	bool m_ShowTextsProduct[5] = { true,true,true,true,true };

	const char* m_TypeNames[3] = { u8"Minden", u8"Bej�v�", u8"Kimen�" };

	const char* m_TimeRangeNames[10] = {u8"Minden", u8"Ma", u8"Tegnap", u8"Ez a h�t", u8"M�lt h�t", u8"Ez a h�nap", 
		u8"M�lt h�nap", u8"Ez az �v", u8"M�lt �v", u8"Egy�ni"};

	const char* m_OverallTexts[8] = { u8"Vonalk�d", u8"N�v", u8"V�tel �r", u8"Bej�v� darabsz�m", u8"�ssz v�tel �r",
		u8"Elad�si �r", u8"Kimen� darabsz�m", u8"�ssz elad�si �r" };
	bool m_ShowOverallTexts[8] = { true,true,true,true,true,true,true,true };

	std::string m_MonthNames[12] = { "Janu�r", "Febru�r", "M�rcius", "�prilis", "M�jus", "J�nius", "J�lius", "Augusztus", "Szeptember",
	"Okt�ber", "November", "December" };

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