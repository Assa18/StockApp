#pragma once

#include "DataManager.h"
#include <ImGui/imgui.h>

#include <vector>

namespace ImGui
{
	IMGUI_API bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback = nullptr, void* user_data = nullptr);
	IMGUI_API bool  InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	IMGUI_API bool  InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

}


enum class Colors {
	Black=0, White, Gray, Beige, Blue, Turkiz
};

class UIManager
{
public:
	UIManager();
	~UIManager();

	void Update();

	void SetDataManager(DataManager* dataM);
private:
	friend class Serializer;

	DataManager* m_DataM;
	float m_Fontsize = 20.0f;

	void ProductWindow(bool& show);
	void ChangeWindow(bool& show, bool editing, StockChangeType type);
	void ShowSettingsWindow();
	void ShowOverallWindow();

	void SearchStockChangesByBarcode(std::vector<StockChange*>& changes, const std::string& barcode, StockChangeType type, int timeRange = 0);
	void SearchStockChangesByName(std::vector<StockChange*>& changes, const std::string& name, StockChangeType type, int timeRange = 0);
	void SearchStockChangesByDate(std::vector<StockChange*>& changes, const Date& date, StockChangeType type);
	void NoSearch(std::vector<StockChange*>& changes);

	void SetupStyle();
};