#include "Grid.h"
#include <SDL/SDL.h>


Grid::Grid(int width, int height, int cellSize) : 
	m_width(width),
	m_height(height),
	m_cellSize(cellSize)
{
	m_numXCells = (int)ceil((float)m_width / m_cellSize);
	m_numYCells = (int)ceil((float)m_height / m_cellSize);

	m_cells.resize(m_numYCells * m_numXCells);
}

Grid::~Grid()
{
}

Cell* Grid::getCell(int x, int y)
{
	// bound x and y positions within the cell grid
	if (x < 0)
		x = 0;
	if (x >= m_numXCells)
		x = m_numXCells - 1;

	if (y < 0)
		y = 0;
	if (y >= m_numYCells)
		y = m_numYCells - 1;
	// ---

	return &m_cells[y * m_numXCells + x];
}

Cell* Grid::getCell(const glm::vec2 & pos)
{
	int cellX = (int)(pos.x / m_cellSize);
	int cellY = (int)(pos.y / m_cellSize);

	return getCell(cellX, cellY);
}

void Grid::update(Gutengine::InputManager &inputManager)
{
	if (inputManager.isKeyPressed(SDLK_SPACE)) {
		getCell(inputManager.getMouseCoords())->color = Gutengine::ColorRGBA8(0, 255, 0, 255);

	}
}
