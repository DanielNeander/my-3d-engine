#pragma once 

class MapGrid  
{
public:

	class GridCell
	{
	public:
		GridCell();
		GridCell(const int cost);
		GridCell(const GridCell& copy);
		GridCell &operator=(const GridCell& rhs);

		inline int getCost() const { return m_cost; }
		void setCost(const int cost) { m_cost = cost; }
	private:
		int m_cost;
	};

	MapGrid(const int gridsize);
	virtual ~MapGrid();

	int getGridSize() const { return m_gridsize; }
	int getCost(const int x, const int y) const;
	void setCost(const int x, const int y, const int cost);

	void setStart(const int x, const int y) { m_startx = x; m_starty = y; }
	void getStart(int &x, int &y) const { x = m_startx; y = m_starty; }

	void setEnd(const int x, const int y) { m_endx = x; m_endy = y; }
	void getEnd(int &x, int &y) const { x = m_endx; y = m_endy; }

private:
	GridCell **m_grid;
	int m_gridsize;
	int m_startx, m_starty, m_endx, m_endy;
};