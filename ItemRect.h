#pragma once
#include "globals.h"
#include "enums.h"

class ItemRect {
private:
	int x;
	int y;
	int width;
	int height;
	int margin[4]; // top, right, bottom. left
	ItemPosition itemPos;

public:
	ItemRect(int x, int y, int width, int height);

	ItemRect(Rect rect);

	ItemRect();

	void Setup(ItemPosition itemPos, int margin=0);

	void Setup(ItemPosition itemPos, int marginTop = 0, int marginRight = 0, int marginBottom = 0, int marginLeft = 0);

	Rect GetRect();
};