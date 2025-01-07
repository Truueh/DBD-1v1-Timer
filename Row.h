#pragma once
#include "ItemRect.h"
#include <vector>

class Row
{
public:
	Row(int itemsAmount, int width, int height);

	std::vector<ItemRect> Items;

	void EnlargeItem(int itemIndex, int size);

	std::vector<ItemRect> GetItems();
};