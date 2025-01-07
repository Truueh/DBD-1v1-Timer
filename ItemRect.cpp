#include "ItemRect.h"

ItemRect::ItemRect(int x, int y, int width, int height)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	
	margin[0] = 0; margin[1] = 0; margin[2] = 0; margin[3] = 0;
	itemPos = ItemPosition::Left;
}

ItemRect::ItemRect(Rect rect)
{
	x = rect.X;
	y = rect.Y;
	width = rect.Width;
	height = rect.Height;
	margin[0] = 0; margin[1] = 0; margin[2] = 0; margin[3] = 0;
	itemPos = ItemPosition::Left;
}

ItemRect::ItemRect()
{
	x = 0; y = 0; width = 0; height = 0;
	margin[0] = 0; margin[1] = 0; margin[2] = 0; margin[3] = 0;
	itemPos = ItemPosition::Left;
}

void ItemRect::Setup(ItemPosition pos, int itemMarging)
{
	this->itemPos = pos;
	
	for (int i = 0; i < 4; i++)
		this->margin[i] = itemMarging;
}

void ItemRect::Setup(ItemPosition itemPos, int marginTop, int marginRight, int marginBottom, int marginLeft)
{
	this->itemPos = itemPos;

	margin[0] = marginTop; margin[1] = marginRight;
	margin[2] = marginBottom; margin[3] = marginLeft;
}

Rect ItemRect::GetRect()
{
	Rect rect;

	// Modify rect size according to margins
	rect.Width = width - margin[3] - margin[1];
	rect.Height = height - margin[0] - margin[2];

	// Modify rect position according to itemPos
	switch (itemPos)
	{
	case ItemPosition::Center:
		rect.X += width / 2;
		rect.X -= rect.Width / 2;
		rect.Y += height / 2;
		rect.Y -= rect.Height / 2;
		break;
	case ItemPosition::Left:
		rect.X = x;
		rect.Y = y;
		break;
	case ItemPosition::Right:
		rect.X = width - rect.Width;
		rect.Y = y;
		break;
	}

	return rect;
}
