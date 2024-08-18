#include "UIManager.h"
#include <iostream>

struct InputTextCallback_UserData
{
	std::string* Str;
	ImGuiInputTextCallback  ChainCallback;
	void* ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
	InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
		std::string* str = user_data->Str;
		IM_ASSERT(data->Buf == str->c_str());
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->c_str();
	}
	else if (user_data->ChainCallback)
	{
		// Forward to user callback, if any
		data->UserData = user_data->ChainCallbackUserData;
		return user_data->ChainCallback(data);
	}
	return 0;
}

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
	//IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;
	return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

struct DataProduct
{
	uint32_t id = 0;
	std::string barcode;
	std::string name;
	int count = 0;
	float buyPrice = 0.0f;
	float sellPrice = 0.0f;

	void Reset()
	{
		id = 0;
		name = "";
		barcode = "";
		count = 0;
		buyPrice = 0.0f;
		sellPrice = 0.0f;
	}

	void Set(const Product& product)
	{
		id = product.GetID();
		name = product.GetName();
		barcode = product.GetBarcode();
		count = product.GetCount();
		buyPrice = product.GetBuyPrice();
		sellPrice = product.GetSellPrice();
	}
};

struct DataChange
{
	std::string barcode;
	std::string name;
	int count = 0;
	Date date;

	void Reset()
	{
		name = "";
		barcode = "";
		count = 0;
		date = Date::GetCurrrentDate();
	}

	void Set(const StockChange& change)
	{
		name = change.GetProduct()->GetName();
		barcode = change.GetProduct()->GetBarcode();
		count = change.GetCount();
		date = change.GetDate();
	}
};

struct SearchData
{
	std::string searchString;
	std::vector<StockChange*> changes;
	int timeRange = 0;
	int type;
};

struct SearchDataProduct
{
	std::string searchString;
	bool searching = false;
	Product* product;
};

static DataProduct s_ProductData;
static DataChange s_ChangeData;
static SearchData s_SearchDataOUT;
static SearchData s_SearchDataIN;
static SearchData s_SearchDataALL;

static SearchDataProduct s_SearchProduct;

UIManager::UIManager()
	:m_DataM(nullptr)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", m_Fontsize);

	SetupStyle();
}

UIManager::~UIManager()
{
}

static bool s_ShowSettingsWindow = false;
static bool s_ShowOverallWindow = false;
static bool s_ShowProdWindow = false;
static bool s_ShowChangeWindow = false;

static bool s_EditingChanges = false;
static bool s_EditingProduct = false;
static StockChangeType s_Type = StockChangeType::NONE;
static Date s_StartDate, s_EndDate;

void UIManager::Update()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	static ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
	static bool pOpen = false;
	ImGui::Begin("ScannerApp", &pOpen, flags);
	ImGui::SetWindowFontScale(m_Fontsize / 18.0f);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menü"))
		{
			if (ImGui::MenuItem("Beállítások")) s_ShowSettingsWindow = true;
			ImGui::Separator();
			if (ImGui::MenuItem("Összegzés"))
			{
				s_ShowOverallWindow = true;
				m_DataM->ClearStats();
				m_DataM->CalculateStats();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGui::BeginTabBar("pages"))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::BeginTabItem("Kimenö termékek"))
		{
			if (ImGui::Button("Új kimenö mozgás"))
			{
				s_ShowChangeWindow = true;
				s_EditingChanges = false;
				s_Type = StockChangeType::OUT;
				s_ChangeData.Reset();
			}

			ImGui::SameLine();
			ImGui::BeginGroup();
			static bool showSelecting = false;
			if (ImGui::Button("Mit mutasson"))
				showSelecting = !showSelecting;
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			static bool showGroup[] = {true, true, true, true};
			static const char* groupTexts[] = { "Vonalkód", "Név", "Darabszám", "Dátum" };
			int nrColumns = 0;
			for (int i = 0; i < 4; i++)
			{
				if (showSelecting) ImGui::Checkbox(groupTexts[i], &showGroup[i]);
				nrColumns += showGroup[i];
			}
			ImGui::EndGroup();
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::BeginGroup();
			static int form = 0;
			if (ImGui::Button("Keresés"))
			{
				switch (form)
				{
					case 0:
						SearchStockChangesByBarcode(s_SearchDataOUT.changes, s_SearchDataOUT.searchString, StockChangeType::OUT);
						break;
					case 1:
						SearchStockChangesByName(s_SearchDataOUT.changes, s_SearchDataOUT.searchString, StockChangeType::OUT);
						break;
				}
			}
			ImGui::SameLine();
			ImGui::InputText("##", &s_SearchDataOUT.searchString);
			ImGui::SameLine();
			if (ImGui::Button("Keresés vége"))
			{
				NoSearch(s_SearchDataOUT.changes);
				s_SearchDataOUT.searchString.clear();
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Keresési forma:"); ImGui::SameLine();
			ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
			ImGui::RadioButton("Név szerint", &form, 1);
			ImGui::PopStyleColor();
			ImGui::EndGroup();

			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

			if (ImGui::BeginTable("Tableoutcomes", nrColumns+1, flags))
			{
				for (int i = 0; i < 4; i++)
				{
					if (showGroup[i]) ImGui::TableSetupColumn(groupTexts[i]);
				}
				ImGui::TableHeadersRow();

				int j = 0;
				if (s_SearchDataOUT.changes.size() == 0) ImGui::Text("A termék nem található!");
				static std::vector<StockChange*>::iterator it;
				for (it = s_SearchDataOUT.changes.begin(); it != s_SearchDataOUT.changes.end(); it++)
				{
					if ((*it)->GetType() == StockChangeType::OUT)
					{
						ImGui::TableNextRow();
						if (showGroup[0])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", (*it)->GetProduct()->GetBarcode().c_str());
						}

						if (showGroup[1])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", (*it)->GetProduct()->GetName().c_str());
						}

						if (showGroup[2])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%d", (*it)->GetCount());
						}

						if (showGroup[3])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", (*it)->GetDate().ToString().c_str());
						}

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6313f, 0.8078f, 0.7647f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
						ImGui::TableNextColumn();
						std::string name = "Szerkeszt##" + std::to_string(j);
						if (ImGui::SmallButton(name.c_str()))
						{
							s_ShowChangeWindow = true;
							s_EditingChanges = true;
							s_Type = StockChangeType::OUT;
							s_ChangeData.Set(*(*it));
						}
						ImGui::SameLine();
						name = "Töröl##" + std::to_string(j);
						if (ImGui::SmallButton(name.c_str()))
						{
							m_DataM->DeleteStockChange((*it)->GetDate());
							NoSearch(s_SearchDataALL.changes);
							NoSearch(s_SearchDataIN.changes);
							NoSearch(s_SearchDataOUT.changes);
							ImGui::PopStyleColor();
							ImGui::PopStyleColor();
							break;
						}
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						j++;
					}
				}
				ImGui::EndTable();
			}
			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::BeginTabItem("Bejövö termékek"))
		{
			if (ImGui::Button("Új bemenö mozgás"))
			{
				s_ShowChangeWindow = true;
				s_EditingChanges = false;
				s_Type = StockChangeType::IN;
				s_ChangeData.Reset();
			}

			ImGui::SameLine();
			ImGui::BeginGroup();
			static bool showSelecting = false;
			if (ImGui::Button("Mit mutasson"))
				showSelecting = !showSelecting;
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			static bool showGroup[] = { true, true, true, true };
			static const char* groupTexts[] = { "Vonalkód", "Név", "Darabszám", "Dátum" };
			int nrColumns = 0;
			for (int i = 0; i < 4; i++)
			{
				if (showSelecting) ImGui::Checkbox(groupTexts[i], &showGroup[i]);
				nrColumns += showGroup[i];
			}
			ImGui::EndGroup();
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::BeginGroup();
			static int form = 0;
			if (ImGui::Button("Keresés"))
			{
				switch (form)
				{
				case 0:
					SearchStockChangesByBarcode(s_SearchDataIN.changes, s_SearchDataIN.searchString, StockChangeType::IN);
					break;
				case 1:
					SearchStockChangesByName(s_SearchDataIN.changes, s_SearchDataIN.searchString, StockChangeType::IN);
					break;
				}
			}
			ImGui::SameLine();
			ImGui::InputText("##", &s_SearchDataIN.searchString);
			ImGui::SameLine();
			if (ImGui::Button("Keresés vége"))
			{
				NoSearch(s_SearchDataIN.changes);
				s_SearchDataIN.searchString.clear();
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Keresési forma:"); ImGui::SameLine();
			ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
			ImGui::RadioButton("Név szerint", &form, 1);
			ImGui::PopStyleColor();
			ImGui::EndGroup();

			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

			if (ImGui::BeginTable("Tableincomes", nrColumns + 1, flags))
			{
				for (int i = 0; i < 4; i++)
				{
					if (showGroup[i]) ImGui::TableSetupColumn(groupTexts[i]);
				}
				ImGui::TableHeadersRow();

				int j = 0;
				if (s_SearchDataIN.changes.size() == 0) ImGui::Text("A termék nem talalható!");
				static std::vector<StockChange*>::iterator it;
				for (it = s_SearchDataIN.changes.begin(); it != s_SearchDataIN.changes.end(); it++)
				{
					if ((*it)->GetType() == StockChangeType::IN)
					{
						ImGui::TableNextRow();
						if (showGroup[0])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", (*it)->GetProduct()->GetBarcode().c_str());
						}

						if (showGroup[1])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", (*it)->GetProduct()->GetName().c_str());
						}

						if (showGroup[2])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%d", (*it)->GetCount());
						}

						if (showGroup[3])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", (*it)->GetDate().ToString().c_str());
						}

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6313f, 0.8078f, 0.7647f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
						ImGui::TableNextColumn();
						std::string name = "Szerkeszt##" + std::to_string(j);
						if (ImGui::SmallButton(name.c_str()))
						{
							s_ShowChangeWindow = true;
							s_EditingChanges = true;
							s_Type = StockChangeType::IN;
							s_ChangeData.Set(*(*it));
						}
						ImGui::SameLine();
						name = "Töröl##" + std::to_string(j);
						if (ImGui::SmallButton(name.c_str()))
						{
							m_DataM->DeleteStockChange((*it)->GetDate());
							NoSearch(s_SearchDataALL.changes);
							NoSearch(s_SearchDataIN.changes);
							NoSearch(s_SearchDataOUT.changes);
							ImGui::PopStyleColor();
							ImGui::PopStyleColor();
							break;
						}
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						j++;
					}
				}

				ImGui::EndTable();
			}
			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::BeginTabItem("Mozgások összegzése"))
		{
			ImGui::BeginGroup();
			static bool showSelecting = false;
			if (ImGui::Button("Mit mutasson"))
				showSelecting = !showSelecting;
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			static bool showGroup[] = { true, true, true, true, true };
			static const char* groupTexts[] = { "Vonalkód", "Név", "Típus", "Darabszám", "Dátum" };
			static const char* typeNames[] = { "Minden", "Bejövö", "Kimenö" };
			int nrColumns = 0;
			for (int i = 0; i < 5; i++)
			{
				if (showSelecting) ImGui::Checkbox(groupTexts[i], &showGroup[i]);
				nrColumns += showGroup[i];
			}
			ImGui::EndGroup();
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::BeginGroup();
			static int form = 0;
			if (ImGui::Button("Keresés"))
			{
				switch (form)
				{
				case 0:
					SearchStockChangesByBarcode(s_SearchDataALL.changes, s_SearchDataALL.searchString,
						(StockChangeType)s_SearchDataALL.type, s_SearchDataALL.timeRange);
					break;
				case 1:
					SearchStockChangesByName(s_SearchDataALL.changes, s_SearchDataALL.searchString,
						(StockChangeType)s_SearchDataALL.type, s_SearchDataALL.timeRange);
					break;
				case 2:
					SearchStockChangesByDate(s_SearchDataALL.changes,
						Date::GetDateFromString(s_SearchDataALL.searchString), (StockChangeType)s_SearchDataALL.type);
					break;
				}
			}
			ImGui::SameLine();
			ImGui::InputText("##", &s_SearchDataALL.searchString);
			ImGui::SameLine();
			if (ImGui::Button("Keresés vege"))
			{
				NoSearch(s_SearchDataALL.changes);
				s_SearchDataALL.searchString.clear();
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Keresési forma:"); ImGui::SameLine();
			ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
			ImGui::RadioButton("Név szerint", &form, 1); ImGui::SameLine();
			ImGui::RadioButton("Dátum szerint", &form, 2);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			static const char* timeRangeNames[] = { "Minden", "Ma", "Tegnap", "Ez a hét", "Múlt hét", "Ez a hónap", "Múlt hónap", "Ez az év", "Múlt év", "Egyéni"};
			static bool showDatePicker = false;
			ImGui::Text("Idö intevallum:"); ImGui::SameLine(); ImGui::PopStyleColor();
			if (form == 2) showDatePicker = false;
			if (form != 2 && ImGui::Combo("##10", &s_SearchDataALL.timeRange, timeRangeNames, IM_ARRAYSIZE(timeRangeNames)))
			{
				if (s_SearchDataALL.timeRange == (int)TimeRanges::COSTUM) showDatePicker = true;
				else showDatePicker = false;
			}

			if (showDatePicker)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::Text("Kezdö dátum:"); ImGui::SameLine();
				ImGui::PopStyleColor();
				ImGui::InputInt3("##111", &s_StartDate.Year);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::Text("Záró dátum:"); ImGui::SameLine();
				ImGui::PopStyleColor();
				ImGui::InputInt3("##112", &s_EndDate.Year);
			}

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Típus:"); ImGui::SameLine();
			ImGui::PopStyleColor();
			ImGui::Combo("##11", &s_SearchDataALL.type, typeNames, IM_ARRAYSIZE(typeNames));

			ImGui::EndGroup();

			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

			if (ImGui::BeginTable("Tablesearch", nrColumns + 1, flags))
			{
				for (int i = 0; i < 5; i++)
				{
					if (showGroup[i]) ImGui::TableSetupColumn(groupTexts[i]);
				}
				ImGui::TableHeadersRow();

				int j = 0;
				if (s_SearchDataALL.changes.size() == 0) ImGui::Text("A termék nem található!");
				static std::vector<StockChange*>::iterator it;
				for (it = s_SearchDataALL.changes.begin(); it != s_SearchDataALL.changes.end(); it++)
				{
					ImGui::TableNextRow();
					if (showGroup[0])
					{
						ImGui::TableNextColumn();
						ImGui::Text("%s", (*it)->GetProduct()->GetBarcode().c_str());
					}

					if (showGroup[1])
					{
						ImGui::TableNextColumn();
						ImGui::Text("%s", (*it)->GetProduct()->GetName().c_str());
					}

					if (showGroup[2])
					{
						ImGui::TableNextColumn();
						ImGui::Text("%s", typeNames[(int)(*it)->GetType()]);
					}

					if (showGroup[3])
					{
						ImGui::TableNextColumn();
						ImGui::Text("%d", (*it)->GetCount());
					}

					if (showGroup[4])
					{
						ImGui::TableNextColumn();
						ImGui::Text("%s", (*it)->GetDate().ToString().c_str());
					}

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6313f, 0.8078f, 0.7647f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
					ImGui::TableNextColumn();
					std::string name = "Szerkeszt##" + std::to_string(j);
					if (ImGui::SmallButton(name.c_str()))
					{
						s_ShowChangeWindow = true;
						s_EditingChanges = true;
						s_Type = (*it)->GetType();
						s_ChangeData.Set(*(*it));
					}
					ImGui::SameLine();
					name = "Töröl##" + std::to_string(j);
					if (ImGui::SmallButton(name.c_str()))
					{
						m_DataM->DeleteStockChange((*it)->GetDate());
						NoSearch(s_SearchDataALL.changes);
						NoSearch(s_SearchDataIN.changes);
						NoSearch(s_SearchDataOUT.changes);
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						break;
					}
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					j++;
				}

				ImGui::EndTable();
			}
			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::BeginTabItem("Termékek"))
		{
			if (ImGui::Button("Új termék hozzaadása"))
			{
				s_ShowProdWindow = true;
				s_EditingProduct = false;
				s_ProductData.Reset();
			}

			ImGui::SameLine();
			ImGui::BeginGroup();
			static bool showSelecting = false;
			if (ImGui::Button("Mit mutasson"))
				showSelecting = !showSelecting;

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			static bool showProps[6] = { true,true,true,true,true,true };
			static const char* propsTexts[6] = { "Vonalkód", "Név", "Darabszám", "Vétel ár", "Eladási ár"};
			int nrColumns=0;
			
			for (int i = 0; i < 5; i++)
			{
				if (showSelecting) ImGui::Checkbox(propsTexts[i], &showProps[i]);
				nrColumns += showProps[i];
			}
			ImGui::EndGroup();
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::BeginGroup();
			static int form = 0;
			if (ImGui::Button("Keresés"))
			{
				if (form == 0) s_SearchProduct.product = m_DataM->SearchProductByBarcode(s_SearchProduct.searchString);
				else s_SearchProduct.product = m_DataM->SearchProductByName(s_SearchProduct.searchString);
				s_SearchProduct.searching = true;
			}
			ImGui::SameLine();
			ImGui::InputText("##", &s_SearchProduct.searchString);
			ImGui::SameLine();
			if (ImGui::Button("Keresés vége"))
			{
				s_SearchProduct.searching = false;
				s_SearchProduct.searchString.clear();
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Keresési forma:"); ImGui::SameLine();
			ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
			ImGui::RadioButton("Név szerint", &form, 1);
			ImGui::SameLine();
			static bool onStock = false;
			ImGui::Checkbox("Készleten", &onStock);
			ImGui::PopStyleColor();
			ImGui::EndGroup();

			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;

			if (ImGui::BeginTable("TableProducts", nrColumns+1,flags))
			{
				for (int i = 0; i < 5; i++)
				{
					if (showProps[i]) ImGui::TableSetupColumn(propsTexts[i]);
				}
				ImGui::TableHeadersRow();

				if (!s_SearchProduct.searching)
				{
					int j = 0;
					for (auto item : m_DataM->GetProducts())
					{
						if (!onStock || (onStock && item.second.GetCount() > 0))
						{
							ImGui::TableNextRow();

							if (showProps[0])
							{
								ImGui::TableNextColumn();
								ImGui::Text("%s", item.second.GetBarcode().c_str());
							}

							if (showProps[1])
							{
								ImGui::TableNextColumn();
								ImGui::Text("%s", item.second.GetName().c_str());
							}

							if (showProps[2])
							{
								ImGui::TableNextColumn();
								ImGui::Text("%d", item.second.GetCount());
							}

							if (showProps[3])
							{
								ImGui::TableNextColumn();
								ImGui::Text("%.2f", item.second.GetBuyPrice());
							}

							if (showProps[4])
							{
								ImGui::TableNextColumn();
								ImGui::Text("%.2f", item.second.GetSellPrice());
							}

							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6313f, 0.8078f, 0.7647f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
							ImGui::TableNextColumn();
							std::string name = "Szerkeszt##" + std::to_string(j);
							if (ImGui::SmallButton(name.c_str()))
							{
								s_ShowProdWindow = true;
								s_ProductData.Set(item.second);
								s_EditingProduct = true;
							}
							/*
							ImGui::SameLine();
							name = "Torol##" + std::to_string(j);
							if (ImGui::SmallButton(name.c_str()))
							{
								m_DataM->DeleteProduct(item.second.GetID());
							}*/
							ImGui::PopStyleColor();
							ImGui::PopStyleColor();
							j++;
						}
					}
				}
				else
				{
					if (s_SearchProduct.product == nullptr) ImGui::Text("A termék nem található!");
					else
					{
						ImGui::TableNextRow();

						if (showProps[0])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", s_SearchProduct.product->GetBarcode().c_str());
						}

						if (showProps[1])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", s_SearchProduct.product->GetName().c_str());
						}

						if (showProps[2])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%d", s_SearchProduct.product->GetCount());
						}

						if (showProps[3])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%.2f", s_SearchProduct.product->GetBuyPrice());
						}

						if (showProps[4])
						{
							ImGui::TableNextColumn();
							ImGui::Text("%.2f", s_SearchProduct.product->GetSellPrice());
						}

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6313f, 0.8078f, 0.7647f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
						ImGui::TableNextColumn();
						if (ImGui::SmallButton("Szerkeszt"))
						{
							s_ShowProdWindow = true;
							s_EditingProduct = true;
							s_ProductData.Set(*s_SearchProduct.product);
						}
						/*
						ImGui::SameLine();
						if (ImGui::SmallButton("Torol"))
						{
							m_DataM->DeleteProduct(s_SearchProduct.product->GetID());
							s_SearchProduct.searching = false;
						}*/
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
					}
				}
				ImGui::EndTable();
			}

			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();
		
		if (s_ShowProdWindow) ProductWindow(s_ShowProdWindow);
		if (s_ShowChangeWindow) ChangeWindow(s_ShowChangeWindow, s_EditingChanges, s_Type);

		ImGui::EndTabBar();
	}

	if (s_ShowSettingsWindow) ShowSettingsWindow();
	if (s_ShowOverallWindow) ShowOverallWindow();

	ImGui::End();

	//static bool show = true;
	//ImGui::ShowDemoWindow(&show);
}

void UIManager::SetDataManager(DataManager* dataM)
{
	m_DataM = dataM;
	NoSearch(s_SearchDataALL.changes);
	NoSearch(s_SearchDataOUT.changes);
	NoSearch(s_SearchDataIN.changes);
}

void UIManager::ProductWindow(bool& show)
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::OpenPopup("Product window");
	if (ImGui::BeginPopupModal("Product window", &show, ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Adja meg/ módosítsa az adatokat:");
		ImGui::Text("Név:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputText("##1", &s_ProductData.name);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Vonalkód:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputText("##2", &s_ProductData.barcode);


		if (s_EditingProduct)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Darab:"); ImGui::SameLine();
			ImGui::PopStyleColor();
			ImGui::InputInt("##3", &s_ProductData.count);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Vétel ár:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputFloat("##4", &s_ProductData.buyPrice);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Eladási ár:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputFloat("##5", &s_ProductData.sellPrice, 0.0f, 0.0f, "%.2f");

		static bool errorEmpty = false;
		if (ImGui::Button("Ok"))
		{
			if (s_ProductData.name == "") errorEmpty = true;
			else
			{
				uint32_t id = 0;
				if (m_DataM->GetProducts().count(s_ProductData.id) > 0)
				{
					Product* prSet = &m_DataM->GetProducts()[s_ProductData.id];
					prSet->Set(s_ProductData.name, s_ProductData.barcode, s_ProductData.count,
						s_ProductData.buyPrice, s_ProductData.sellPrice);
				}
				else
				{
					id = Product::GetNewID();
					Product pr(id, s_ProductData.barcode, s_ProductData.name, s_ProductData.count,
						s_ProductData.buyPrice, s_ProductData.sellPrice);
					m_DataM->AddProduct(pr);
				}
				ImGui::CloseCurrentPopup();
				show = false;
				s_SearchProduct.searching = false;
				errorEmpty = false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Mégse"))
		{
			ImGui::CloseCurrentPopup();
			show = false;
			errorEmpty = false;
		}

		if (errorEmpty)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Nem adhatsz a termékkészlethez üres terméket!");
			ImGui::PopStyleColor();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleColor();
}

static Product* prWindow;
static int changeCount = 0;
void UIManager::ChangeWindow(bool& show, bool editing, StockChangeType type)
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::OpenPopup("Change window");
	if (ImGui::BeginPopupModal("Change window", &show, ImGuiWindowFlags_NoTitleBar))
	{
		static bool error = false;
		static bool errorCount = false;
		static int form = 0;

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Keresési forma:"); ImGui::SameLine();
		ImGui::RadioButton("Vonalkód##1", &form, 0); ImGui::SameLine();
		ImGui::RadioButton("Név##1", &form, 1);
		ImGui::PopStyleColor();
		if (!editing)
		{
			if (form == 0)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::Text("Vonalkód: "); ImGui::SameLine();
				ImGui::PopStyleColor();
				if (ImGui::InputText("##3", &s_ChangeData.barcode))
				{
					prWindow = m_DataM->SearchProductByBarcode(s_ChangeData.barcode);
					if (prWindow)
					{
						s_ChangeData.name = prWindow->GetName();
						error = false;
					}
				}
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				if (!prWindow) ImGui::Text("Nincs ilyen vonalkódú termék!");
				ImGui::Text("Név: %s", s_ChangeData.name.c_str());
				ImGui::PopStyleColor();
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::Text("Név:"); ImGui::SameLine();
				ImGui::PopStyleColor();
				if (ImGui::InputText("##4", &s_ChangeData.name))
				{
					prWindow = m_DataM->SearchProductByName(s_ChangeData.name);
					if (prWindow)
					{
						error = false;
						s_ChangeData.barcode = prWindow->GetBarcode();
					}
				}
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				if (!prWindow) ImGui::Text("Nincs ilyen nevü termék!");
				ImGui::Text("Vonalkód: %s", s_ChangeData.barcode.c_str());
				ImGui::PopStyleColor();
			}
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Vonalkód: %s", s_ChangeData.barcode.c_str());
			ImGui::Text("Név: %s", s_ChangeData.name.c_str());
			ImGui::PopStyleColor();
			if (!prWindow)
			{
				prWindow = m_DataM->GetUpdates()[s_ChangeData.date].GetProduct();
				changeCount = s_ChangeData.count;
			}
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Darabszám:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputInt("##1", &s_ChangeData.count);
		if (s_ChangeData.count < 0) s_ChangeData.count = 0;

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Dátum: %s", s_ChangeData.date.ToString().c_str());
		static const char* typeNames[] = { "Nincs  típus", "Bejövö", "Kimenö" };
		ImGui::Text("Mozgás típusa: %s", typeNames[(int)type]);
		ImGui::PopStyleColor();

		if (ImGui::Button("Ok"))
		{
			if (prWindow)
			{
				if ((type == StockChangeType::OUT && prWindow->GetCount() < s_ChangeData.count) || s_ChangeData.count < 0) errorCount = true;
				else
				{
					if (m_DataM->GetUpdates().count(s_ChangeData.date) > 0)
					{
						m_DataM->GetUpdates()[s_ChangeData.date].SetCount(s_ChangeData.count);
					}
					else
					{
						StockChange sc(prWindow, s_ChangeData.date, s_ChangeData.count, type);
						m_DataM->AddStockChange(sc);
					}

					if (type == StockChangeType::OUT)
					{
						prWindow->SetCount(prWindow->GetCount() + changeCount);
						prWindow->SetCount(prWindow->GetCount() - s_ChangeData.count);
					}
					else if (type == StockChangeType::IN)
					{
						prWindow->SetCount(prWindow->GetCount() - changeCount);
						prWindow->SetCount(prWindow->GetCount() + s_ChangeData.count);
					}
					error = false;
					errorCount = false;
					prWindow = nullptr;
				}
			}
			else error = true;
			if (!error && !errorCount)
			{
				ImGui::CloseCurrentPopup();
				show = false;
			}
			NoSearch(s_SearchDataALL.changes);
			NoSearch(s_SearchDataIN.changes);
			NoSearch(s_SearchDataOUT.changes);
		}
		ImGui::SameLine();
		if (ImGui::Button("Mégse"))
		{
			ImGui::CloseCurrentPopup();
			show = false;
			error = false;
			errorCount = false;
			prWindow = nullptr;
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		if (error) ImGui::Text("Nincs ilyen termék!");
		if (errorCount) ImGui::Text("Nem lehet többet eladni a termékböl, mint amennyi készleten van!");
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}
	ImGui::PopStyleColor();
}

static float lastFontsize;
void UIManager::ShowSettingsWindow()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::OpenPopup("SettingsWindow");
	if (ImGui::BeginPopupModal("SettingsWindow", &s_ShowSettingsWindow, ImGuiWindowFlags_NoTitleBar))
	{
		lastFontsize = m_Fontsize;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Betüméret: ");
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
		ImGui::SliderFloat("##", &m_Fontsize, 10.0f, 30.0f, "%.1f");
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::SetWindowFontScale(m_Fontsize / 18.0f);

		if (ImGui::Button("OK"))
		{
			ImGui::CloseCurrentPopup();
			s_ShowSettingsWindow = false;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleColor();
}

static std::string inputString;
static Product* prSearch = nullptr;
static std::map<uint32_t, DataManager::ProductStats> mapStat;
static std::map<int, DataManager::ProductStats> mapProductStats;
static std::map<std::pair<int, int>, DataManager::ProductStats> mapMonthStat;
static DataManager::ProductStats Stats;
void UIManager::ShowOverallWindow()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::OpenPopup("Összegzések");
	if (ImGui::BeginPopupModal("Összegzések", &s_ShowOverallWindow))
	{
		ImGui::BeginGroup();
		static bool showSelecting = false;
		if (ImGui::Button("Mit mutasson"))
			showSelecting = !showSelecting;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

		static bool showProps[] = { true,true,true,true,true,true,true,true };
		static const char* propsTexts[8] = { "Vonalkód", "Név", "Vétel ár", "Bejövö darabszám", "Össz vétel ár",
		"Eladási ár", "Kimenö darabszám", "Össz eladási ár" };
		int nrColumns = 0;

		for (int i = 0; i < 8; i++)
		{
			if (showSelecting) ImGui::Checkbox(propsTexts[i], &showProps[i]);
			nrColumns += showProps[i];
		}
		ImGui::PopStyleColor();
		ImGui::EndGroup();
		ImGui::SameLine();

		ImGui::BeginGroup();
		static int form = 0;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Termék:");
		ImGui::PopStyleColor();
		ImGui::SameLine();

		static std::string MonthNames[] = { "Január", "Február", "Március", "Április", "Május", "Június",
		"Július", "Augusztus", "Szeptember", "Október", "November", "December" };
		static bool searching = false;
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;

		ImGui::InputText("##1", &inputString);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Keresési forma:"); ImGui::SameLine();
		ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
		ImGui::RadioButton("Név szerint", &form, 1);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (ImGui::Button("Keresés"))
		{
			searching = true;
			if (form == 0) prSearch = m_DataM->SearchProductByBarcode(inputString);
			else prSearch = m_DataM->SearchProductByName(inputString);
			mapProductStats.clear();
			mapMonthStat.clear();
			Stats.product = nullptr;
		}
		ImGui::SameLine();
		if (ImGui::Button("Keresés vége"))
		{
			searching = false;
			mapProductStats.clear();
			inputString.clear();
			Stats.product = nullptr;
			mapMonthStat.clear();
		}
		ImGui::EndGroup();

		if (ImGui::BeginTabBar("overall"))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			if (ImGui::BeginTabItem("Évi lebontás"))
			{
				std::map<int, DataManager::YearStat>::reverse_iterator it;
				for (it = m_DataM->GetYearStats().rbegin(); it != m_DataM->GetYearStats().rend(); it++)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::SeparatorText(std::to_string((*it).first).c_str());
					ImGui::Text("Ebben az évben %.2f összegben volt áru vásárolva", (*it).second.TotalBuyings);
					ImGui::Text("Ebben az évben %.2f összegben volt áru eladva", (*it).second.TotalSellings);
					ImGui::PopStyleColor();
					if (ImGui::BeginTable("tableyear", nrColumns, flags))
					{
						for (int i = 0; i < 8; i++)
						{
							if (showProps[i]) ImGui::TableSetupColumn(propsTexts[i]);
						}
						ImGui::TableHeadersRow();

						if (searching)
						{
							if (!prSearch) ImGui::Text("A termék nem található!");
							else
							{
								if (mapProductStats.size() == 0) mapProductStats = m_DataM->GetStatsYear(prSearch);
								ImGui::TableNextRow();
								if (showProps[0])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", mapProductStats[(*it).first].product->GetBarcode().c_str());
								}

								if (showProps[1])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", mapProductStats[(*it).first].product->GetName().c_str());
								}

								if (showProps[2])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", mapProductStats[(*it).first].product->GetBuyPrice());
								}

								if (showProps[3])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%d", mapProductStats[(*it).first].INCount);
								}

								if (showProps[4])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", mapProductStats[(*it).first].TotalBuyPrice);
								}

								if (showProps[5])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", mapProductStats[(*it).first].product->GetSellPrice());
								}

								if (showProps[6])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%d", mapProductStats[(*it).first].OUTCount);
								}

								if (showProps[7])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", mapProductStats[(*it).first].TotalSellPrice);
								}
							}
						}
						else
						{
							static std::map<uint32_t, Product>::iterator itPr;
							for (itPr = m_DataM->GetProducts().begin(); itPr != m_DataM->GetProducts().end();itPr++)
							{
								int inCount = 0, outCount = 0;
								float buyings = 0.0f, sellings = 0.0f;
								for (int i = 0; i < 12; i++)
								{
									if ((*it).second.Months[i].Stats.count((*itPr).first)>0)
									{
										inCount += (*it).second.Months[i].Stats[(*itPr).first].INCount;
											outCount += (*it).second.Months[i].Stats[(*itPr).first].OUTCount;
											buyings += (*it).second.Months[i].Stats[(*itPr).first].TotalBuyPrice;
											sellings += (*it).second.Months[i].Stats[(*itPr).first].TotalSellPrice;
									}
								}

								ImGui::TableNextRow();
								if (showProps[0])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", (*itPr).second.GetBarcode().c_str());
								}

								if (showProps[1])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", (*itPr).second.GetName().c_str());
								}

								if (showProps[2])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", (*itPr).second.GetBuyPrice());
								}

								if (showProps[3])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%d", inCount);
								}

								if (showProps[4])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", buyings);
								}

								if (showProps[5])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", (*itPr).second.GetSellPrice());
								}

								if (showProps[6])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%d", outCount);
								}

								if (showProps[7])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", sellings);
								}
							}
						}

						ImGui::EndTable();
					}
				}

				ImGui::EndTabItem();
			}
			ImGui::PopStyleColor();

			static float monthSells = 0.0f, monthBuys = 0.0f;

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			if (ImGui::BeginTabItem("Havi lebontás"))
			{
				std::map<int, DataManager::YearStat>::reverse_iterator itYear;
				for (itYear = m_DataM->GetYearStats().rbegin(); itYear != m_DataM->GetYearStats().rend(); itYear++)
				{
					for (int i = 11; i >= 0; i--)
					{
						if ((*itYear).second.Months[i].TotalBuyings == 0 && (*itYear).second.Months[i].TotalSellings == 0) continue;
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
						ImGui::SeparatorText((std::to_string((*itYear).first) + ' ' + MonthNames[i]).c_str());
						ImGui::PopStyleColor();
						
						monthBuys = 0.0f;
						monthSells = 0.0f;
						if (ImGui::BeginTable("tablemonths", nrColumns, flags))
						{
							for (int j = 0; j < 8; j++)
							{
								if (showProps[j]) ImGui::TableSetupColumn(propsTexts[j]);
							}
							ImGui::TableHeadersRow();

							if (searching)
							{
								if (!prSearch) ImGui::Text("A termék nem található!");
								else
								{
									if (mapMonthStat.size() == 0) mapMonthStat = m_DataM->GetStatsMonth(prSearch);

									std::pair<int, int> pairStat = std::make_pair((*itYear).first, i+1);
									
									ImGui::TableNextRow();
									if (showProps[0])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%s", mapMonthStat[pairStat].product->GetBarcode().c_str());
									}

									if (showProps[1])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%s", mapMonthStat[pairStat].product->GetName().c_str());
									}

									if (showProps[2])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%.2f", mapMonthStat[pairStat].product->GetBuyPrice());
									}

									if (showProps[3])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%d", mapMonthStat[pairStat].INCount);
									}

									if (showProps[4])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%.2f", mapMonthStat[pairStat].TotalBuyPrice);
									}

									if (showProps[5])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%.2f", mapMonthStat[pairStat].product->GetSellPrice());
									}

									if (showProps[6])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%d", mapMonthStat[pairStat].OUTCount);
									}

									if (showProps[7])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%.2f", mapMonthStat[pairStat].TotalSellPrice);
									}
								}
							}
							else
							{
								std::map<uint32_t, DataManager::ProductStats>::iterator itStat;
								for (itStat = (*itYear).second.Months[i].Stats.begin(); itStat != (*itYear).second.Months[i].Stats.end(); itStat++)
								{
									ImGui::TableNextRow();
									if (showProps[0])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%s", (*itStat).second.product->GetBarcode().c_str());
									}

									if (showProps[1])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%s", (*itStat).second.product->GetName().c_str());
									}

									if (showProps[2])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%.2f", (*itStat).second.product->GetBuyPrice());
									}

									if (showProps[3])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%d", (*itStat).second.INCount);
									}

									if (showProps[4])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%.2f", (*itStat).second.TotalBuyPrice);
									}

									if (showProps[5])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%.2f", (*itStat).second.product->GetSellPrice());
									}

									if (showProps[6])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%d", (*itStat).second.OUTCount);
									}

									if (showProps[7])
									{
										ImGui::TableNextColumn();
										ImGui::Text("%.2f", (*itStat).second.TotalSellPrice);
									}
									monthBuys += (*itStat).second.TotalBuyPrice;
									monthSells += (*itStat).second.TotalSellPrice;
								}
							}

							ImGui::EndTable();
						}
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
						ImGui::Text("Ebben a hónapban %.2f értékben volt áru vásárolva.", monthBuys);
						ImGui::Text("Ebben a hónapban %.2f értékben volt áru eladva.", monthSells);
						ImGui::PopStyleColor();
					}
				}

				ImGui::EndTabItem();
			}
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			if (ImGui::BeginTabItem("Egyéni lebontás"))
			{
				static bool showTable = false;
				ImGui::BeginGroup();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::Text("Kezdö dátum:"); ImGui::SameLine();
				ImGui::PopStyleColor();
				ImGui::InputInt3("##111", &s_StartDate.Year);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::Text("Záró dátum:"); ImGui::SameLine();
				ImGui::PopStyleColor();
				ImGui::InputInt3("##112", &s_EndDate.Year);
				ImGui::EndGroup();
				ImGui::SameLine();
				if (ImGui::Button("Dátum beállítása"))
				{
					mapStat = m_DataM->CalculateStats(s_StartDate, s_EndDate);
					showTable = true;
				}

				static float overallBuys = 0.0f, overallSells = 0.0f;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::Text("Ebben az idöszakban %.2f értékben volt áru vásárolva.", overallBuys);
				ImGui::Text("Ebben az idöszakban %.2f értékben volt áru eladva.", overallSells);
				ImGui::PopStyleColor();

				overallBuys = 0.0f, overallSells = 0.0f;

				if (showTable)
				{
					if (ImGui::BeginTable("tableCostum", nrColumns, flags))
					{
						for (int i = 0; i < 8; i++)
						{
							if (showProps[i]) ImGui::TableSetupColumn(propsTexts[i]);
						}
						ImGui::TableHeadersRow();

						if (searching)
						{
							if (!prSearch) ImGui::Text("A termék nem található!");
							else
							{
								if (Stats.product == nullptr) Stats = m_DataM->GetStatsCostum(prSearch, s_StartDate, s_EndDate);

								ImGui::TableNextRow();
								if (showProps[0])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", Stats.product->GetBarcode().c_str());
								}

								if (showProps[1])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", Stats.product->GetName().c_str());
								}

								if (showProps[2])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", Stats.product->GetBuyPrice());
								}

								if (showProps[3])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%d", Stats.INCount);
								}

								if (showProps[4])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", Stats.TotalBuyPrice);
								}

								if (showProps[5])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", Stats.product->GetSellPrice());
								}

								if (showProps[6])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%d", Stats.OUTCount);
								}

								if (showProps[7])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", Stats.TotalSellPrice);
								}
							}
						}
						else
						{
							static std::map<uint32_t, DataManager::ProductStats>::iterator itPr;
							for (itPr = mapStat.begin(); itPr != mapStat.end(); itPr++)
							{
								ImGui::TableNextRow();
								if (showProps[0])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", (*itPr).second.product->GetBarcode().c_str());
								}

								if (showProps[1])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%s", (*itPr).second.product->GetName().c_str());
								}

								if (showProps[2])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", (*itPr).second.product->GetBuyPrice());
								}

								if (showProps[3])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%d", (*itPr).second.INCount);
								}

								if (showProps[4])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", (*itPr).second.TotalBuyPrice);
								}

								if (showProps[5])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", (*itPr).second.product->GetSellPrice());
								}

								if (showProps[6])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%d", (*itPr).second.OUTCount);
								}

								if (showProps[7])
								{
									ImGui::TableNextColumn();
									ImGui::Text("%.2f", (*itPr).second.TotalSellPrice);
								}
								overallBuys += (*itPr).second.TotalBuyPrice;
								overallSells += (*itPr).second.TotalSellPrice;
							}
						}

						ImGui::EndTable();
					}
				}

				ImGui::EndTabItem();
			}
			ImGui::PopStyleColor();

			ImGui::EndTabBar();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleColor();
}

static void SetDateRange(TimeRanges range)
{
	switch (range)
	{
	case TimeRanges::ALL:
		s_StartDate.Set(1990, 1, 1, 0, 0, 0);
		s_EndDate = Date::GetCurrrentDate();
		break;
	case TimeRanges::TODAY:
		s_StartDate.Set(Date::GetCurrrentDate().Year, Date::GetCurrrentDate().Month, Date::GetCurrrentDate().Day, 0, 0, 0);
		s_EndDate = Date::GetCurrrentDate();
		break;
	case TimeRanges::YESTERDAY:
		s_StartDate = Date::GetCurrrentDate();
		s_StartDate.Set(Date::GetCurrrentDate().Year, Date::GetCurrrentDate().Month, Date::GetCurrrentDate().Day-1, 0, 0, 0);
		s_EndDate.Set(Date::GetCurrrentDate().Year, Date::GetCurrrentDate().Month, Date::GetCurrrentDate().Day, 0, 0, 0);
		break;
	case TimeRanges::THIS_WEEK:
		s_StartDate = Date::GetCurrrentDate();
		for (int i = 0; i < Date::GetCurrentDayOfWeek(); i++) s_StartDate--;
		s_EndDate = Date::GetCurrrentDate();
		break;
	case TimeRanges::LAST_WEEK:
		s_StartDate = Date::GetCurrrentDate();
		s_EndDate = Date::GetCurrrentDate();
		for (int i = 0; i < Date::GetCurrentDayOfWeek(); i++)
		{
			s_StartDate--;
			s_EndDate--;
		}
		for (int i = 0; i < 7; i++) s_StartDate--;
		break;
	case TimeRanges::THIS_MONTH:
		s_StartDate = Date::GetCurrrentDate();
		for (int i = 0; i < Date::GetCurrentDayOfMonth(); i++) s_StartDate--;
		s_EndDate = Date::GetCurrrentDate();
		break;
	case TimeRanges::LAST_MONTH:
		s_StartDate = Date::GetCurrrentDate();
		s_EndDate = Date::GetCurrrentDate();
		for (int i = 0; i < Date::GetDayOfMonth(s_StartDate.Month); i++)
		{
			s_StartDate--;
			s_EndDate--;
		}
		for (int i = 0; i < Date::GetDayOfMonth(Date::GetCurrrentDate().Month - 1); i++) s_StartDate--;
		break;
	case TimeRanges::THIS_YEAR:
		s_StartDate.Set(Date::GetCurrrentDate().Year, 1, 1, 0, 0, 0);
		s_EndDate = Date::GetCurrrentDate();
		break;
	case TimeRanges::LAST_YEAR:
		s_StartDate.Set(Date::GetCurrrentDate().Year - 1, 12, 31, 0, 0, 0);
		s_EndDate.Set(Date::GetCurrrentDate().Year - 1, 1, 1, 0, 0, 0);
		break;
	default:
		break;
	}
}

void UIManager::SearchStockChangesByBarcode(std::vector<StockChange*>& changes, const std::string& barcode, 
	StockChangeType type, int timeRange)
{
	SetDateRange((TimeRanges)timeRange);
	changes.clear();
	std::map<Date, StockChange>::reverse_iterator it;
	for (it = m_DataM->GetUpdates().rbegin(); it != m_DataM->GetUpdates().rend(); it++)
	{
		if ((*it).second.GetDate() >= s_StartDate && (*it).second.GetDate()<=s_EndDate && 
			((barcode == "") || (*it).second.GetProduct()->GetBarcode() == barcode) &&
			(type == StockChangeType::NONE || (*it).second.GetType() == type))
			changes.push_back(&(*it).second);
	}
}

void UIManager::SearchStockChangesByName(std::vector<StockChange*>& changes, const std::string& name, 
	StockChangeType type, int timeRange)
{
	SetDateRange((TimeRanges)timeRange);
	changes.clear();
	std::map<Date, StockChange>::reverse_iterator it;
	for (it = m_DataM->GetUpdates().rbegin(); it != m_DataM->GetUpdates().rend(); it++)
	{
		if ((*it).second.GetDate() >=s_StartDate && (*it).second.GetDate()<=s_EndDate && 
			((name=="") || (*it).second.GetProduct()->GetName() == name) &&
			(type == StockChangeType::NONE || (*it).second.GetType() == type))
			changes.push_back(&(*it).second);
	}
}

void UIManager::SearchStockChangesByDate(std::vector<StockChange*>& changes, const Date& date, 
	StockChangeType type)
{
	changes.clear();
	std::map<Date, StockChange>::reverse_iterator it;
	for (it = m_DataM->GetUpdates().rbegin(); it != m_DataM->GetUpdates().rend(); it++)
	{
		if ((type == StockChangeType::NONE && (*it).second.GetDate().Equals(date)) || ((*it).second.GetDate().Equals(date) && (*it).second.GetType() == type))
			changes.push_back(&(*it).second);
	}
}

void UIManager::NoSearch(std::vector<StockChange*>& changes)
{
	changes.clear();
	std::map<Date, StockChange>::reverse_iterator it;
	for (it = m_DataM->GetUpdates().rbegin(); it != m_DataM->GetUpdates().rend(); it++)
	{
		changes.push_back(&(*it).second);
	}
}

void UIManager::SetupStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	style->ChildRounding = 4.0f;
	style->FrameBorderSize = 1.0f;
	style->FrameRounding = 2.0f;
	style->GrabMinSize = 7.0f;
	style->PopupRounding = 2.0f;
	style->ScrollbarRounding = 12.0f;
	style->ScrollbarSize = 13.0f;
	style->TabBorderSize = 1.0f;
	style->TabRounding = 0.0f;
	style->WindowRounding = 4.0f;

	ImVec4 paletteColors[6];
	paletteColors[(int)Colors::Black] = { 0.0f, 0.0f, 0.0f, 1.0f };
	paletteColors[(int)Colors::White] = { 1.0f, 1.0f, 1.0f, 1.0f };
	paletteColors[(int)Colors::Gray] = { 0.3254f, 0.3372f, 0.3411f, 1.0f };
	paletteColors[(int)Colors::Beige] = { 0.9803f, 0.9568f, 0.8274f, 1.0f };
	paletteColors[(int)Colors::Blue] = {0.6039f, 0.7686f, 0.9725f, 1.0f};
	paletteColors[(int)Colors::Turkiz] = { 0.6313f, 0.8078f, 0.7647f, 1.0f };
	//paletteColors[(int)Colors::Green] = { 0.6156f, 0.9843f, 0.7058f, 1.0f };

	colors[ImGuiCol_WindowBg] = paletteColors[(int)Colors::Gray];
	colors[ImGuiCol_PopupBg] = paletteColors[(int)Colors::Gray];
	colors[ImGuiCol_Text] = paletteColors[(int)Colors::White];
	colors[ImGuiCol_Button] = paletteColors[(int)Colors::Beige];
	colors[ImGuiCol_ButtonHovered] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_ButtonActive] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_Header] = paletteColors[(int)Colors::Gray];
	colors[ImGuiCol_ResizeGrip] = paletteColors[(int)Colors::Blue];
	colors[ImGuiCol_Tab] = paletteColors[(int)Colors::Beige];
	colors[ImGuiCol_TabActive] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_TabHovered] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_TableBorderLight] = paletteColors[(int)Colors::Black];
	colors[ImGuiCol_TableBorderStrong] = paletteColors[(int)Colors::Black];
	colors[ImGuiCol_TableHeaderBg] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_TableRowBg] = paletteColors[(int)Colors::Blue];
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.8509f, 0.9137f, 0.9882f, 1.0f);
	colors[ImGuiCol_FrameBg] = paletteColors[(int)Colors::Beige];
	colors[ImGuiCol_FrameBgActive] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_FrameBgHovered] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_MenuBarBg] = paletteColors[(int)Colors::Gray];
	colors[ImGuiCol_Separator] = paletteColors[(int)Colors::White];
	colors[ImGuiCol_CheckMark] = paletteColors[(int)Colors::Black];
	colors[ImGuiCol_HeaderActive] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_HeaderHovered] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_SliderGrab] = paletteColors[(int)Colors::Turkiz];
	colors[ImGuiCol_SliderGrabActive] = paletteColors[(int)Colors::Turkiz];
}