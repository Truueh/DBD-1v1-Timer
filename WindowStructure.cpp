#include "WindowStructure.h"

WindowStructure::WindowStructure(int rows, int width, int height)
{
	for (int i = 0; i < rows; i++)
		Rows[i] = Row(rows, width, height / rows);
}
