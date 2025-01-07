#pragma once
#include <vector>
#include "Row.h"

class WindowStructure 
{
public:
	WindowStructure(int rows, int width, int height);

	int Width;
	int Height;

	std::vector<Row> Rows;
};