#pragma once

#include "DataManager.h"
#include "UIManager.h"

class Serializer
{
public:
	
	static void SerializeProducts(const DataManager* dM, const char* path);
	static void DeserializeProducts(DataManager* dM, const char* path);

	static void SerializeStockChanges(const DataManager* dM, const char* path);
	static void DeserializeStockChanges(DataManager* dM, const char* path);

	static void SerializeSettings(const DataManager* dM, const UIManager* uM, const char* path);
	static void DeserializeSettings(DataManager* dM, UIManager* uM, const char* path);
};