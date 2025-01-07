#include "Row.h"

Row::Row(int itemsAmount, int width, int height) {
	for (int i = 0; i < itemsAmount; i++) {
		int itemWidth = width / itemsAmount;
		int x = itemWidth * i;
		int margin[4] = { 0, 0, 0, 0 };

		Items[i] = ItemRect(x, 0, itemWidth, height);
	}
}

void Row::EnlargeItem(int itemIndex, int size)
{
	// Make selected item larger
	Rect rect = Items[itemIndex].GetRect();
	rect.Width *= size;

	Items[itemIndex] = ItemRect(rect);

	// Remove leading items
	Items.erase(Items.begin() + itemIndex, Items.begin() + itemIndex + size);
}

std::vector<ItemRect> Row::GetItems()
{
	return Items;
}
