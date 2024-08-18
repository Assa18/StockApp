#include "Serializer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

void Serializer::SerializeProducts(const DataManager* dM, const char* path)
{
	std::ofstream outProducts(path);
	
	outProducts << "Number of products: " << dM->GetNumOfProducts() << '\n';
	for (auto pr : dM->GetProducts())
	{
		outProducts << "{\n";
		outProducts << '\t' << "ID:\t" << pr.second.GetID() << '\n';
		outProducts << '\t' << "Nev:\t" << pr.second.GetName() << '\n';
		outProducts << '\t' << "Vonalkod:\t" << pr.second.GetBarcode() << '\n';
		outProducts << '\t' << "Darabszam:\t" << pr.second.GetCount() << '\n';
		outProducts << '\t' << "Vetel_ar:\t" << pr.second.GetBuyPrice() << '\n';
		outProducts << '\t' << "Eladasi_ar:\t" << pr.second.GetSellPrice() << '\n';
		outProducts << "}\n";
	}

	outProducts.close();
}

void Serializer::DeserializeProducts(DataManager* dM, const char* path)
{
	std::ifstream inProducts(path);
	if (!inProducts)
	{
		std::cout << "Can't load input file: " << path << "\nThe database will be empty!\n";
		return;
	}
	
	std::string line, input;
	std::getline(inProducts, line);
	std::stringstream ss(line);
	std::getline(ss, input, ' ');
	std::getline(ss, input, ' ');
	std::getline(ss, input, ' ');
	std::getline(ss, input, ' ');
	int numOfProducts = std::stoi(input);

	for (int i = 0; i < numOfProducts; i++)
	{
		Product pr;
		// the '{'
		std::getline(inProducts, line);
		
		//actual data
		std::getline(inProducts, line);
		std::stringstream ss0(line);
		std::getline(ss0, input, '	');
		std::getline(ss0, input, '	');
		std::getline(ss0, input, '	');
		uint32_t id = std::stoi(input);
		pr.SetID(id);
		Product::SetLastID(id);

		std::getline(inProducts, line);
		std::stringstream ss1(line);
		std::getline(ss1, input, '	');
		std::getline(ss1, input, '	');
		std::getline(ss1, input, '	');
		pr.SetName(input);
		

		std::getline(inProducts, line);
		std::stringstream ss2(line);
		std::getline(ss2, input, '	');
		std::getline(ss2, input, '	');
		std::getline(ss2, input, '	');
		pr.SetBarcode(input);

		std::getline(inProducts, line);
		std::stringstream ss3(line);
		std::getline(ss3, input, '	');
		std::getline(ss3, input, '	');
		std::getline(ss3, input, '	');
		pr.SetCount(std::stoi(input));

		std::getline(inProducts, line);
		std::stringstream ss4(line);
		std::getline(ss4, input, '	');
		std::getline(ss4, input, '	');
		std::getline(ss4, input, '	');
		pr.SetBuyPrice(std::stof(input));

		std::getline(inProducts, line);
		std::stringstream ss6(line);
		std::getline(ss6, input, '	');
		std::getline(ss6, input, '	');
		std::getline(ss6, input, '	');
		pr.SetSellPrice(std::stof(input));

		//the '}'
		std::getline(inProducts, line);
		dM->AddProduct(pr);
	}

	inProducts.close();
}


void Serializer::SerializeStockChanges(const DataManager* dM, const char* path)
{
	std::ofstream outChanges(path);

	outChanges << "Number of changes: " << dM->GetNumOfStockChanges() << '\n';
	for (auto ch : dM->GetUpdates())
	{
		outChanges << "{\n";
		outChanges << '\t' << "Termek_ID:\t" << ch.second.GetProduct()->GetID() << '\n';
		outChanges << '\t' << "Datum:\t" << ch.second.GetDate().ToString() << '\n';
		outChanges << '\t' << "Darab:\t" << ch.second.GetCount() << '\n';
		outChanges << '\t' << "Tipus:\t" << (int)ch.second.GetType() << '\n';
		outChanges << "}\n";
	}

	outChanges.close();
}

void Serializer::DeserializeStockChanges(DataManager* dM, const char* path)
{
	std::ifstream inChanges(path);
	if (!inChanges)
	{
		std::cout << "Can't load input file: " << path << "\nThe database will be empty!\n";
		return;
	}

	std::string line, input;
	std::getline(inChanges, line);
	std::stringstream ss(line);
	std::getline(ss, input, ' ');
	std::getline(ss, input, ' ');
	std::getline(ss, input, ' ');
	std::getline(ss, input, ' ');
	int numOfChanges = std::stoi(input);

	for (int i = 0; i < numOfChanges; i++)
	{
		StockChange change;
		// '{'
		std::getline(inChanges, line);

		//actual data
		std::getline(inChanges, line);
		std::stringstream ss0(line);
		std::getline(ss0, input, '	');
		std::getline(ss0, input, '	');
		std::getline(ss0, input, '	');
		uint32_t id = std::stoi(input);
		change.SetProduct(dM->SearchProductByID(id));

		std::getline(inChanges, line);
		std::stringstream ss1(line);
		std::getline(ss1, input, '	');
		std::getline(ss1, input, '	');
		std::getline(ss1, input, '	');
		change.SetDate(Date::GetDateFromString(input));

		std::getline(inChanges, line);
		std::stringstream ss2(line);
		std::getline(ss2, input, '	');
		std::getline(ss2, input, '	');
		std::getline(ss2, input, '	');
		change.SetCount(std::stoi(input));
		
		std::getline(inChanges, line);
		std::stringstream ss3(line);
		std::getline(ss3, input, '	');
		std::getline(ss3, input, '	');
		std::getline(ss3, input, '	');
		change.SetType((StockChangeType)std::stoi(input));

		// '}'
		std::getline(inChanges, line);
		dM->AddStockChange(change);
	}

	inChanges.close();
}

void Serializer::SerializeSettings(const DataManager* dM, const UIManager* uM, const char* path)
{
	std::ofstream outSettings(path);

	outSettings << "Betumeret:\n" << uM->m_Fontsize << '\n';

	outSettings.close();
}

void Serializer::DeserializeSettings(DataManager* dM, UIManager* uM, const char* path)
{
	std::ifstream inSettings(path);
	
	if (!inSettings)
	{
		std::cout << "Nem sikerult a beallitasokat betolteni!\n";
		return;
	}

	std::string line;
	std::getline(inSettings, line);
	std::getline(inSettings, line);
	float fontSize = std::stof(line);
	uM->m_Fontsize = fontSize;
	inSettings.close();
}