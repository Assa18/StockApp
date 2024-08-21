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
				m_State = UIStates::AddChange;
				//s_Type = StockChangeType::OUT;
				//s_ChangeData.Reset();
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
				//s_Type = StockChangeType::OUT;
				//s_ChangeData.Reset();
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
			ImGui::EndTabItem();
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::BeginTabItem("Termékek"))
		{
			if (ImGui::Button("Új termék hozzaadása"))
			{
				m_State = UIStates::AddProduct;
				//s_ProductData.Reset();
			}

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
		if (ImGui::Button("Ok"))
		{
			m_State = UIStates::NoState;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleColor();
}

void UIManager::EditChangeWindow()
{

}

void UIManager::AddProductWindow()
{

}

void UIManager::EditProductWindow()
{

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
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

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
				//s_Type = StockChangeType::IN;
				//s_ChangeData.Set(*(*it));
			}
			ImGui::SameLine();
			name = "Töröl##" + std::to_string(j);
			if (ImGui::SmallButton(name.c_str()))
			{
				m_DataM->DeleteStockChange((*it)->GetDate());
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