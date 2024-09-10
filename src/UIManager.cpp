#include "UIManager.h"

#include <map>
#include <string>

// ImGui stuff, implenting the ImGui::InputText with std::string
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

// UIManager implementation
UIManager::UIManager()
	:m_DataM(nullptr)
{
}

UIManager::~UIManager()
{

}

void UIManager::SetDataManager(DataManager* dataM)
{
	m_DataM = dataM;
	m_DataM->FillChangesANY();
	m_DataM->FillChangesIN();
	m_DataM->FillChangesOUT();
	m_DataM->FillProductPtrs(m_OnStock);
}

void UIManager::Update()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	static ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
	static bool pOpen = false;
	ImGui::Begin("ScannerApp", &pOpen, flags);
	ImGui::SetWindowFontScale(m_FontSize / 18.0f);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menü"))
		{
			if (ImGui::MenuItem("Beállítások")) m_State = UIStates::Settings;
			ImGui::Separator();
			if (ImGui::MenuItem("Összegzés"))
			{
				m_State = UIStates::Overall;
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
				m_State = UIStates::AddChange;
				m_ChangeData.Reset(StockChangeType::OUT);
			}
			ImGui::SameLine();
			WhatToShow(4, m_TextsChange, m_ShowTextsChange);
			ImGui::SameLine();
			ShowSearch(m_DataM->GetChangesOUT(), StockChangeType::OUT, m_SearchStringOUT);

			ShowTable(m_DataM->GetChangesOUT(), 4, m_TextsChange, m_ShowTextsChange, StockChangeType::OUT);

			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::BeginTabItem("Bejövö termékek"))
		{
			if (ImGui::Button("Új bejövö mozgás"))
			{
				m_State = UIStates::AddChange;
				m_ChangeData.Reset(StockChangeType::IN);
			}
			ImGui::SameLine();
			WhatToShow(4, m_TextsChange, m_ShowTextsChange);
			ImGui::SameLine();
			ShowSearch(m_DataM->GetChangesIN(), StockChangeType::IN, m_SearchStringIN);

			ShowTable(m_DataM->GetChangesIN(), 4, m_TextsChange, m_ShowTextsChange, StockChangeType::IN);

			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::BeginTabItem("Mozgások összegzése"))
		{
			WhatToShow(5, m_TextsAllChange, m_ShowTextsAllChange);
			ImGui::SameLine();
			ShowSearchAll();

			ShowTable(m_DataM->GetChangesANY(), 5, m_TextsAllChange, m_ShowTextsAllChange, StockChangeType::ANY);

			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::BeginTabItem("Termékek"))
		{
			if (ImGui::Button("Új termék hozzaadása"))
			{
				m_State = UIStates::AddProduct;
				m_ProductData.Reset();
			}
			ImGui::SameLine();
			WhatToShow(5, m_TextsProduct, m_ShowTextsProduct);
			ImGui::SameLine();
			ShowSearch(m_DataM->GetProductPtrs(), m_SearchStringPr);
			
			ShowTable(m_DataM->GetProductPtrs(), 5, m_TextsProduct, m_ShowTextsProduct);

			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();

		ImGui::EndTabBar();
	}

	switch (m_State)
	{
	case UIStates::NoState:
		break;
	case UIStates::Settings:
		SettingsWindow();
		break;
	case UIStates::Overall:
		OverallWindow();
		break;
	case UIStates::AddChange:
		AddChangeWindow();
		break;
	case UIStates::EditChange:
		EditChangeWindow();
		break;
	case UIStates::AddProduct:
		AddProductWindow();
		break;
	case UIStates::EditProduct:
		EditProductWindow();
		break;
	default:
		break;
	}

	ImGui::End();
}

void UIManager::AddChangeWindow()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	
	ImGui::OpenPopup("Add change window");
	if (ImGui::BeginPopupModal("Add change window"))
	{
		static bool errorProduct = false;
		static bool errorCount = false;
		static int form = 0;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Keresési forma:"); ImGui::SameLine();
		ImGui::RadioButton("Vonalkód##1", &form, 0); ImGui::SameLine();
		ImGui::RadioButton("Név##1", &form, 1);
		ImGui::PopStyleColor();

		if (form == 0)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Vonalkód: "); ImGui::SameLine();
			ImGui::PopStyleColor();
			if (ImGui::InputText("##3", &m_ChangeData.Barcode))
			{
				m_Productptr = m_DataM->SearchProductByBarcode(m_ChangeData.Barcode);
				if (m_Productptr)
				{
					m_ChangeData.Name = m_Productptr->GetName();
					errorProduct = false;
				}
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			if (!m_Productptr) ImGui::Text("Nincs ilyen vonalkódú termék!");
			ImGui::Text("Név: %s", m_ChangeData.Name.c_str());
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Név:"); ImGui::SameLine();
			ImGui::PopStyleColor();
			if (ImGui::InputText("##4", &m_ChangeData.Name))
			{
				m_Productptr = m_DataM->SearchProductByName(m_ChangeData.Name);
				if (m_Productptr)
				{
					m_ChangeData.Barcode = m_Productptr->GetBarcode();
					m_ChangeData.Name = m_Productptr->GetName();
					errorProduct = false;
				}
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			if (!m_Productptr) ImGui::Text("Nincs ilyen nevü termék!");
			ImGui::Text("Vonalkód: %s", m_ChangeData.Barcode.c_str());
			ImGui::PopStyleColor();
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Darabszám:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputInt("##1", &m_ChangeData.Count);
		if (m_ChangeData.Count < 0) m_ChangeData.Count = 0;

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Dátum: %s", m_ChangeData.Time.ToString().c_str());
		ImGui::Text("Mozgás típusa: %s", m_TypeNames[(int)m_ChangeData.Type]);
		ImGui::PopStyleColor();

		if (ImGui::Button("Ok"))
		{
			if (m_Productptr)
			{
				if ((m_ChangeData.Type == StockChangeType::OUT && m_Productptr->GetCount() < m_ChangeData.Count)) errorCount = true;
				else
				{
					StockChange sc(m_Productptr, m_ChangeData.Time, m_ChangeData.Count, m_ChangeData.Type);
					m_DataM->AddStockChange(sc);

					if (m_ChangeData.Type == StockChangeType::OUT)
					{
						m_Productptr->SetCount(m_Productptr->GetCount() - m_ChangeData.Count);
					}
					else
					{
						m_Productptr->SetCount(m_Productptr->GetCount() + m_ChangeData.Count);
					}
					errorProduct = false;
					errorCount = false;
					m_Productptr = nullptr;
				}
			}
			else errorProduct = true;
			if (!errorProduct && !errorCount)
			{
				m_State = UIStates::NoState;
				ImGui::CloseCurrentPopup();
			}
			m_DataM->FillChangesANY();
			if (m_ChangeData.Type == StockChangeType::OUT) m_DataM->FillChangesOUT();
			else m_DataM->FillChangesIN();
		}
		ImGui::SameLine();
		if (ImGui::Button("Mégse"))
		{
			m_State = UIStates::NoState;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		if (errorProduct) ImGui::Text("Nincs ilyen termék!");
		if (errorCount) ImGui::Text("Nem lehet többet eladni a termékböl, mint amennyi készleten van!");
		ImGui::PopStyleColor();
		
		ImGui::EndPopup();
	}

	ImGui::PopStyleColor();
}

void UIManager::EditChangeWindow()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	ImGui::OpenPopup("Edit change window");
	if (ImGui::BeginPopupModal("Edit change window"))
	{
		static bool errorCount = false;
		static int prevCount = m_ChangeData.Count;
		
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Név: %s", m_ChangeData.Name.c_str());
		ImGui::Text("Vonalkód: %s", m_ChangeData.Barcode.c_str());
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Darabszám:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputInt("##1", &m_ChangeData.Count);
		if (m_ChangeData.Count < 0) m_ChangeData.Count = 0;

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Dátum: %s", m_ChangeData.Time.ToString().c_str());
		ImGui::Text("Mozgás típusa: %s", m_TypeNames[(int)m_ChangeData.Type]);
		ImGui::PopStyleColor();

		if (ImGui::Button("Ok"))
		{
			if ((m_ChangeData.Type == StockChangeType::OUT && m_Productptr->GetCount() < m_ChangeData.Count)) errorCount = true;
			else
			{
				m_DataM->GetUpdates()[m_ChangeData.Time].m_Count = m_ChangeData.Count;

				if (m_ChangeData.Type == StockChangeType::OUT)
				{
					m_Productptr->SetCount(m_Productptr->GetCount() + prevCount);
					m_Productptr->SetCount(m_Productptr->GetCount() - m_ChangeData.Count);
				}
				else
				{
					m_Productptr->SetCount(m_Productptr->GetCount() - prevCount);
					m_Productptr->SetCount(m_Productptr->GetCount() + m_ChangeData.Count);
				}
				errorCount = false;
				m_Productptr = nullptr;
			}
			
			if (!errorCount)
			{
				m_State = UIStates::NoState;
				ImGui::CloseCurrentPopup();
			}
			m_DataM->FillChangesANY();
			if (m_ChangeData.Type == StockChangeType::OUT) m_DataM->FillChangesOUT();
			else m_DataM->FillChangesIN();
		}
		ImGui::SameLine();
		if (ImGui::Button("Mégse"))
		{
			m_State = UIStates::NoState;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		if (errorCount) ImGui::Text("Nem lehet többet eladni a termékböl, mint amennyi készleten van!");
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}

	ImGui::PopStyleColor();
}

void UIManager::AddProductWindow()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	ImGui::OpenPopup("Add product window");
	if (ImGui::BeginPopupModal("Add product window"))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Adja meg/ módosítsa az adatokat:");
		ImGui::Text("Név:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputText("##1", &m_ProductData.Name);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Vonalkód:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputText("##2", &m_ProductData.Barcode);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Vétel ár:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputFloat("##4", &m_ProductData.BuyPrice);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Eladási ár:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputFloat("##5", &m_ProductData.SellPrice, 0.0f, 0.0f, "%.2f");

		static bool errorEmpty = false;
		if (ImGui::Button("Ok"))
		{
			if (m_ProductData.Name == "") errorEmpty = true;
			else
			{
				uint32_t id = Product::GetNewID();
				Product pr(id, m_ProductData.Barcode, m_ProductData.Name, m_ProductData.Count,
					m_ProductData.BuyPrice, m_ProductData.SellPrice);
				m_DataM->AddProduct(pr);
				m_DataM->FillProductPtrs(m_OnStock);
				ImGui::CloseCurrentPopup();
				m_State = UIStates::NoState;
				errorEmpty = false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Mégse"))
		{
			ImGui::CloseCurrentPopup();
			m_State = UIStates::NoState;
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

void UIManager::EditProductWindow()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	ImGui::OpenPopup("Edit product window");
	if (ImGui::BeginPopupModal("Edit product window"))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Adja meg/ módosítsa az adatokat:");
		ImGui::Text("Név:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputText("##1", &m_ProductData.Name);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Vonalkód:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputText("##2", &m_ProductData.Barcode);

		if (m_State == UIStates::EditProduct)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("Darab:"); ImGui::SameLine();
			ImGui::PopStyleColor();
			ImGui::InputInt("##3", &m_ProductData.Count);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Vétel ár:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputFloat("##4", &m_ProductData.BuyPrice);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Eladási ár:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputFloat("##5", &m_ProductData.SellPrice, 0.0f, 0.0f, "%.2f");

		static bool errorEmpty = false;
		if (ImGui::Button("Ok"))
		{
			if (m_ProductData.Name == "") errorEmpty = true;
			else
			{
				if (m_DataM->GetProducts().count(m_ProductData.ID) > 0)
				{
					m_DataM->GetProducts()[m_ProductData.ID].Set(m_ProductData.Name, m_ProductData.Barcode, m_ProductData.Count,
						m_ProductData.BuyPrice, m_ProductData.SellPrice);
				}

				m_DataM->FillProductPtrs(m_OnStock);
				ImGui::CloseCurrentPopup();
				m_State = UIStates::NoState;
				errorEmpty = false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Mégse"))
		{
			ImGui::CloseCurrentPopup();
			m_State = UIStates::NoState;
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

void UIManager::SettingsWindow()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::OpenPopup("SettingsWindow");
	static bool open = (m_State == UIStates::Settings);
	if (ImGui::BeginPopupModal("SettingsWindow", &open, ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Betüméret: ");
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
		ImGui::SliderFloat("##", &m_FontSize, 10.0f, 30.0f, "%.1f");
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::SetWindowFontScale(m_FontSize / 18.0f);

		if (ImGui::Button("OK"))
		{
			ImGui::CloseCurrentPopup();
			m_State = UIStates::NoState;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleColor();
}

void UIManager::OverallWindow()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::OpenPopup("Összegzések");
	if (ImGui::BeginPopupModal("Összegzések"))
	{
		if (ImGui::Button("Bezár"))
		{
			m_State = UIStates::NoState;
		}

		ImGui::SameLine();
		WhatToShow(8, m_OverallTexts, m_ShowOverallTexts);
		ImGui::SameLine();
		ImGui::BeginGroup();

		static int form = 0;
		if (ImGui::Button("Keresés"))
		{
			m_OnStock = false;
			switch (form)
			{
			case 0:
				m_DataM->SearchProductBarcode(m_DataM->GetProductPtrs(), m_SearchStringOverall);
				break;
			case 1:
				m_DataM->SearchProductName(m_DataM->GetProductPtrs(), m_SearchStringOverall);
				break;
			default:
				break;
			}
		}
		ImGui::SameLine();
		ImGui::InputText("##", &m_SearchStringOverall);

		ImGui::SameLine();
		if (ImGui::Button("Keresés vége"))
		{
			m_DataM->FillProductPtrs(m_OnStock);
			m_SearchStringOverall.clear();
		}
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Keresési forma:"); ImGui::SameLine();
		ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
		ImGui::RadioButton("Név szerint", &form, 1);
		ImGui::PopStyleColor();

		ImGui::EndGroup();

		if (ImGui::BeginTabBar("overall"))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			if (ImGui::BeginTabItem("Évi lebontás"))
			{
				DisplayYearStats();
				ImGui::EndTabItem();
			}
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			if (ImGui::BeginTabItem("Havi lebontás"))
			{
				DisplayMonthStats();
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
				ImGui::InputInt3("##111", &m_StartDate.Year);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::Text("Záró dátum:"); ImGui::SameLine();
				ImGui::PopStyleColor();
				ImGui::InputInt3("##112", &m_EndDate.Year);
				ImGui::EndGroup();
				ImGui::SameLine();
				if (ImGui::Button("Dátum beállítása"))
				{
					m_DataM->CalculateStats(m_StartDate, m_EndDate);
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
					DisplayCostumStats(overallBuys, overallSells);
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

void UIManager::DisplayYearStats()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	
	int nrColumns = 0;
	for (int i = 0; i < 8; i++)
	{
		nrColumns += m_ShowOverallTexts[i];
	}
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

	std::map<int, std::map<Product*, ProductStats>>::reverse_iterator it;
	for (it = m_DataM->GetYearStats().rbegin(); it != m_DataM->GetYearStats().rend(); it++)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::SeparatorText(std::to_string((*it).first).c_str());
		ImGui::PopStyleColor();

		static float overallBuysYear = 0.0f, overallSellsYear = 0.0f;
		overallBuysYear = 0.0f, overallSellsYear = 0.0f;

		if (ImGui::BeginTable("table", nrColumns, flags))
		{
			for (int i = 0; i < 8; i++)
			{
				if (m_ShowOverallTexts[i]) ImGui::TableSetupColumn(m_OverallTexts[i]);
			}
			ImGui::TableHeadersRow();

			std::map<Product*, ProductStats>::iterator itPr;
			for (itPr = (*it).second.begin(); itPr != (*it).second.end(); itPr++)
			{
				ImGui::TableNextRow();
				if (m_ShowOverallTexts[0])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", (*itPr).first->GetBarcode().c_str());
				}

				if (m_ShowOverallTexts[1])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", (*itPr).first->GetName().c_str());
				}

				if (m_ShowOverallTexts[2])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", (*itPr).first->GetBuyPrice());
				}

				if (m_ShowOverallTexts[3])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%d", (*itPr).second.CountIN);
				}

				if (m_ShowOverallTexts[4])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", (*itPr).second.ValueIN);
				}

				if (m_ShowOverallTexts[5])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", (*itPr).first->GetSellPrice());
				}

				if (m_ShowOverallTexts[6])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%d", (*itPr).second.CountOUT);
				}

				if (m_ShowOverallTexts[7])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", (*itPr).second.ValueOUT);
				}
				overallBuysYear += (*itPr).second.ValueIN;
				overallSellsYear += (*itPr).second.ValueOUT;
			}
			ImGui::EndTable();
		}
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Ebben az idöszakban %.2f értékben volt áru vásárolva.", overallBuysYear);
		ImGui::Text("Ebben az idöszakban %.2f értékben volt áru eladva.", overallSellsYear);
		ImGui::PopStyleColor();

	}

	ImGui::PopStyleColor();
}

void UIManager::DisplayMonthStats()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	int nrColumns = 0;
	for (int i = 0; i < 8; i++)
	{
		nrColumns += m_ShowOverallTexts[i];
	}
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

	std::map<MonthPair, std::map<Product*, ProductStats>>::reverse_iterator it;
	for (it = m_DataM->GetMonthStats().rbegin(); it != m_DataM->GetMonthStats().rend(); it++)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::SeparatorText((std::to_string((*it).first.Year) + " " +m_MonthNames[(*it).first.Month-1]).c_str());
		ImGui::PopStyleColor();

		static float overallBuysMonth = 0.0f, overallSellsMonth = 0.0f;
		overallBuysMonth = 0.0f, overallSellsMonth = 0.0f;

		if (ImGui::BeginTable("table", nrColumns, flags))
		{
			for (int i = 0; i < 8; i++)
			{
				if (m_ShowOverallTexts[i]) ImGui::TableSetupColumn(m_OverallTexts[i]);
			}
			ImGui::TableHeadersRow();

			std::map<Product*, ProductStats>::iterator itPr;
			for (itPr = (*it).second.begin(); itPr != (*it).second.end(); itPr++)
			{
				ImGui::TableNextRow();
				if (m_ShowOverallTexts[0])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", (*itPr).first->GetBarcode().c_str());
				}

				if (m_ShowOverallTexts[1])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", (*itPr).first->GetName().c_str());
				}

				if (m_ShowOverallTexts[2])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", (*itPr).first->GetBuyPrice());
				}

				if (m_ShowOverallTexts[3])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%d", (*itPr).second.CountIN);
				}

				if (m_ShowOverallTexts[4])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", (*itPr).second.ValueIN);
				}

				if (m_ShowOverallTexts[5])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", (*itPr).first->GetSellPrice());
				}

				if (m_ShowOverallTexts[6])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%d", (*itPr).second.CountOUT);
				}

				if (m_ShowOverallTexts[7])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", (*itPr).second.ValueOUT);
				}
				overallBuysMonth += (*itPr).second.ValueIN;
				overallSellsMonth += (*itPr).second.ValueOUT;
			}
			ImGui::EndTable();
		}
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Ebben az idöszakban %.2f értékben volt áru vásárolva.", overallBuysMonth);
		ImGui::Text("Ebben az idöszakban %.2f értékben volt áru eladva.", overallSellsMonth);
		ImGui::PopStyleColor();

	}

	ImGui::PopStyleColor();
}

void UIManager::DisplayCostumStats(float& buyings, float& sellings)
{
	int nrColumns = 0;
	for (int i = 0; i < 8; i++)
	{
		nrColumns += m_ShowOverallTexts[i];
	}
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

	if (ImGui::BeginTable("table", nrColumns, flags))
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_ShowOverallTexts[i]) ImGui::TableSetupColumn(m_OverallTexts[i]);
		}
		ImGui::TableHeadersRow();

		std::vector<Product*>::iterator it;
		for (it = m_DataM->GetProductPtrs().begin(); it != m_DataM->GetProductPtrs().end(); it++)
		{
			ProductStats& stats = m_DataM->GetCostumStats()[(*it)];
			ImGui::TableNextRow();
			if (m_ShowOverallTexts[0])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%s", (*it)->GetBarcode().c_str());
			}

			if (m_ShowOverallTexts[1])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%s", (*it)->GetName().c_str());
			}

			if (m_ShowOverallTexts[2])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", (*it)->GetBuyPrice());
			}

			if (m_ShowOverallTexts[3])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%d", stats.CountIN);
			}

			if (m_ShowOverallTexts[4])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", stats.ValueIN);
			}

			if (m_ShowOverallTexts[5])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", (*it)->GetSellPrice());
			}

			if (m_ShowOverallTexts[6])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%d", stats.CountOUT);
			}

			if (m_ShowOverallTexts[7])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", stats.ValueOUT);
			}
			buyings += stats.ValueIN;
			sellings += stats.ValueOUT;
		}
		ImGui::EndTable();
	}
}

void UIManager::WhatToShow(int num, const char* texts[], bool booleans[])
{
	ImGui::BeginGroup();
	static bool showSelecting = false;
	if (ImGui::Button("Mit mutasson"))
		showSelecting = !showSelecting;

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	for (int i = 0; i < num; i++)
	{
		if (showSelecting) ImGui::Checkbox(texts[i], &booleans[i]);
	}
	ImGui::PopStyleColor();
	ImGui::EndGroup();
}

void UIManager::ShowTable(const std::vector<StockChange*>& source, int num, const char* texts[], bool showTexts[], StockChangeType type)
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	int nrColumns = 0;
	for (int i = 0; i < num; i++)
	{
		nrColumns += showTexts[i];
	}
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

	if (ImGui::BeginTable("table", nrColumns + 1, flags))
	{
		for (int i = 0; i < num; i++)
		{
			if (showTexts[i]) ImGui::TableSetupColumn(texts[i]);
		}
		ImGui::TableHeadersRow();

		int j = 0;
		if (source.size() == 0) ImGui::Text("A termék nem található!");
		std::vector<StockChange*>::const_reverse_iterator it;
		for (it = source.rbegin(); it != source.rend(); it++)
		{
			ImGui::TableNextRow();
			if (showTexts[0])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%s", (*it)->GetProduct()->GetBarcode().c_str());
			}

			if (showTexts[1])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%s", (*it)->GetProduct()->GetName().c_str());
			}

			if (num == 5)
			{
				if (showTexts[2])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", m_TypeNames[(int)(*it)->GetType()]);
				}

				if (showTexts[3])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%d", (*it)->GetCount());
				}

				if (showTexts[4])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", (*it)->GetDate().ToString().c_str());
				}

			}
			else
			{
				if (showTexts[2])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%d", (*it)->GetCount());
				}

				if (showTexts[3])
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", (*it)->GetDate().ToString().c_str());
				}
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6313f, 0.8078f, 0.7647f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
			ImGui::TableNextColumn();
			std::string name = "Szerkeszt##" + std::to_string(j);
			if (ImGui::SmallButton(name.c_str()))
			{
				m_State = UIStates::EditChange;
				m_ChangeData.Set((*it));
				m_Productptr = (*it)->m_Product;
			}
			ImGui::SameLine();
			name = "Töröl##" + std::to_string(j);
			if (ImGui::SmallButton(name.c_str()))
			{
				m_DataM->DeleteStockChange((*it)->GetDate());
				m_DataM->FillChangesANY();
				if (type == StockChangeType::IN) m_DataM->FillChangesIN();
				else m_DataM->FillChangesOUT();
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

	ImGui::PopStyleColor();
}

void UIManager::ShowTable(const std::vector<Product*>& source, int num, const char* texts[], bool showTexts[])
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	int nrColumns = 0;
	for (int i = 0; i < num; i++)
	{
		nrColumns += showTexts[i];
	}
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

	if (ImGui::BeginTable("table", nrColumns + 1, flags))
	{
		for (int i = 0; i < num; i++)
		{
			if (showTexts[i]) ImGui::TableSetupColumn(texts[i]);
		}
		ImGui::TableHeadersRow();

		int j = 0;
		if (source.size() == 0) ImGui::Text("A termék nem található!");
		std::vector<Product*>::const_iterator it;
		for (it = source.begin(); it != source.end(); it++)
		{
			ImGui::TableNextRow();
			if (showTexts[0])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%s", (*it)->GetBarcode().c_str());
			}

			if (showTexts[1])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%s", (*it)->GetName().c_str());
			}

			if (showTexts[2])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%d", (*it)->GetCount());
			}

			if (showTexts[3])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%f", (*it)->GetBuyPrice());
			}

			if (showTexts[4])
			{
				ImGui::TableNextColumn();
				ImGui::Text("%f", (*it)->GetSellPrice());
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6313f, 0.8078f, 0.7647f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9803f, 0.9568f, 0.8274f, 1.0f));
			ImGui::TableNextColumn();
			std::string name = "Szerkeszt##" + std::to_string(j);
			if (ImGui::SmallButton(name.c_str()))
			{
				m_State = UIStates::EditProduct;
				m_ProductData.Set((*it));
			}
			ImGui::SameLine();
			name = "Töröl##" + std::to_string(j);
			if (ImGui::SmallButton(name.c_str()))
			{
				m_DataM->DeleteProduct((*it)->GetID());
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

	ImGui::PopStyleColor();
}

void UIManager::ShowSearch(std::vector<StockChange*> &source, StockChangeType type, std::string& searchString)
{
	ImGui::BeginGroup();

	static int form = 0;
	if (ImGui::Button("Keresés"))
	{
		switch (form)
		{
		case 0:
			m_DataM->SearchByBarcode(source, type, searchString);
			break;
		case 1:
			m_DataM->SearchByName(source, type, searchString);
			break;
		default:
			break;
		}
	}
	ImGui::SameLine();
	ImGui::InputText("##", &searchString);

	ImGui::SameLine();
	if (ImGui::Button("Keresés vége"))
	{
		switch (type)
		{
		case StockChangeType::ANY:
			m_DataM->FillChangesANY();
			break;
		case StockChangeType::IN:
			m_DataM->FillChangesIN();
			break;
		case StockChangeType::OUT:
			m_DataM->FillChangesOUT();
			break;
		default:
			break;
		}
		searchString.clear();
	}
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Text("Keresési forma:"); ImGui::SameLine();
	ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
	ImGui::RadioButton("Név szerint", &form, 1);
	ImGui::PopStyleColor();

	ImGui::EndGroup();
}

void UIManager::ShowSearchAll()
{
	ImGui::BeginGroup();

	static int form = 0;

	if (ImGui::Button("Keresés"))
	{
		switch (form)
		{
		case 0:
			SetTimeRange();
			m_DataM->SearchByBarcode(m_DataM->GetChangesANY(), m_StartDate, m_EndDate, (StockChangeType)m_SearchType, m_SearchStringANY);
			break;
		case 1:
			SetTimeRange();
			m_DataM->SearchByName(m_DataM->GetChangesANY(), m_StartDate, m_EndDate, (StockChangeType)m_SearchType, m_SearchStringANY);
			break;
		case 2:
			m_DataM->SearchByDate(m_DataM->GetChangesANY(), (StockChangeType)m_SearchType, m_SearchStringANY);
			break;
		}
	}
	ImGui::SameLine();
	ImGui::InputText("##", &m_SearchStringANY);
	ImGui::SameLine();
	if (ImGui::Button("Keresés vege"))
	{
		m_SearchStringANY.clear();
		m_DataM->FillChangesANY();
	}
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Text("Keresési forma:"); ImGui::SameLine();
	ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
	ImGui::RadioButton("Név szerint", &form, 1); ImGui::SameLine();
	ImGui::RadioButton("Dátum szerint", &form, 2);
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	static bool showDatePicker = false;
	ImGui::Text("Idö intevallum:"); ImGui::SameLine(); ImGui::PopStyleColor();
	if (form == 2) showDatePicker = false;
	if (form != 2 && ImGui::Combo("##10", &m_TimeRange, m_TimeRangeNames, 10))
	{
		if (m_TimeRange == (int)TimeRanges::COSTUM) showDatePicker = true;
		else showDatePicker = false;
	}

	if (showDatePicker)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Kezdö dátum:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputInt3("##111", &m_StartDate.Year);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("Záró dátum:"); ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::InputInt3("##112", &m_EndDate.Year);
	}

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Text("Típus:"); ImGui::SameLine();
	ImGui::PopStyleColor();
	ImGui::Combo("##11", &m_SearchType, m_TypeNames, 3);

	ImGui::EndGroup();
}

void UIManager::ShowSearch(std::vector<Product*>& source, std::string& searchString)
{
	ImGui::BeginGroup();

	static int form = 0;
	if (ImGui::Button("Keresés"))
	{
		switch (form)
		{
		case 0:
			m_DataM->SearchProductBarcode(source, searchString);
			break;
		case 1:
			m_DataM->SearchProductName(source, searchString);
			break;
		default:
			break;
		}
	}
	ImGui::SameLine();
	ImGui::InputText("##", &searchString);

	ImGui::SameLine();
	if (ImGui::Button("Keresés vége"))
	{
			m_DataM->FillProductPtrs(m_OnStock);
		searchString.clear();
	}
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Text("Keresési forma:"); ImGui::SameLine();
	ImGui::RadioButton("Vonalkód szerint", &form, 0); ImGui::SameLine();
	ImGui::RadioButton("Név szerint", &form, 1);
	ImGui::SameLine();
	if (ImGui::Checkbox("Készleten", &m_OnStock))
	{
		m_DataM->FillProductPtrs(m_OnStock);
	}
	ImGui::PopStyleColor();

	ImGui::EndGroup();
}

void UIManager::SetTimeRange()
{
	switch (m_TimeRange)
	{
	case (int)TimeRanges::ALL:
		m_StartDate.Set(1990, 1, 1, 0, 0, 0);
		m_EndDate = Date::GetCurrrentDate();
		break;
	case (int)TimeRanges::TODAY:
		m_StartDate.Set(Date::GetCurrrentDate().Year, Date::GetCurrrentDate().Month, Date::GetCurrrentDate().Day, 0, 0, 0);
		m_EndDate = Date::GetCurrrentDate();
		break;
	case (int)TimeRanges::YESTERDAY:
		m_StartDate = Date::GetCurrrentDate();
		m_StartDate.Set(Date::GetCurrrentDate().Year, Date::GetCurrrentDate().Month, Date::GetCurrrentDate().Day - 1, 0, 0, 0);
		m_EndDate.Set(Date::GetCurrrentDate().Year, Date::GetCurrrentDate().Month, Date::GetCurrrentDate().Day, 0, 0, 0);
		break;
	case (int)TimeRanges::THIS_WEEK:
		m_StartDate = Date::GetCurrrentDate();
		for (int i = 0; i < Date::GetCurrentDayOfWeek(); i++) m_StartDate--;
		m_EndDate = Date::GetCurrrentDate();
		break;
	case (int)TimeRanges::LAST_WEEK:
		m_StartDate = Date::GetCurrrentDate();
		m_EndDate = Date::GetCurrrentDate();
		for (int i = 0; i < Date::GetCurrentDayOfWeek(); i++)
		{
			m_StartDate--;
			m_EndDate--;
		}
		for (int i = 0; i < 7; i++) m_StartDate--;
		break;
	case (int)TimeRanges::THIS_MONTH:
		m_StartDate = Date::GetCurrrentDate();
		for (int i = 0; i < Date::GetCurrentDayOfMonth(); i++) m_StartDate--;
		m_EndDate = Date::GetCurrrentDate();
		break;
	case (int)TimeRanges::LAST_MONTH:
		m_StartDate = Date::GetCurrrentDate();
		m_EndDate = Date::GetCurrrentDate();
		for (int i = 0; i < Date::GetDayOfMonth(m_StartDate.Month); i++)
		{
			m_StartDate--;
			m_EndDate--;
		}
		for (int i = 0; i < Date::GetDayOfMonth(Date::GetCurrrentDate().Month - 1); i++) m_StartDate--;
		break;
	case (int)TimeRanges::THIS_YEAR:
		m_StartDate.Set(Date::GetCurrrentDate().Year, 1, 1, 0, 0, 0);
		m_EndDate = Date::GetCurrrentDate();
		break;
	case (int)TimeRanges::LAST_YEAR:
		m_StartDate.Set(Date::GetCurrrentDate().Year - 1, 12, 31, 0, 0, 0);
		m_EndDate.Set(Date::GetCurrrentDate().Year - 1, 1, 1, 0, 0, 0);
		break;
	default:
		break;
	}
}

void UIManager::SetupStyle()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", m_FontSize);

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
	paletteColors[(int)Colors::Blue] = { 0.6039f, 0.7686f, 0.9725f, 1.0f };
	paletteColors[(int)Colors::Turkiz] = { 0.6313f, 0.8078f, 0.7647f, 1.0f };

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