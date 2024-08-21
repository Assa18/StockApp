#pragma once

#include "DataManager.h"
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
	void ShowSearch(std::vector<StockChange*> &source, StockChangeType type, std::string& searchString);
private:
	friend class Serializer;
	DataManager* m_DataM;
	float m_FontSize = 20.0f;
	UIStates m_State = UIStates::NoState;

	const char* m_TextsChange[4] = { u8"Vonalkód", u8"Név", u8"Darabszám", u8"Dátum" };
	bool m_ShowTextsChange[4] = { true,true,true,true };

	const char* m_TextsAllChange[5] = { u8"Vonalkód", u8"Név", u8"Típus", u8"Darabszám", u8"Dátum"};
	bool m_ShowTextsAllChange[5] = { true,true,true,true,true };

	const char* m_TypeNames[3] = { u8"Minden", u8"Kimenö", u8"Bejövö" };

	std::string m_SearchStringIN;
	std::string m_SearchStringOUT;
	std::string m_SearchStringANY;
};