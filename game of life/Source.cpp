#define OLC_PGE_APPLICATION
#include "pixelGameEngine.h"
#include <chrono>

class GameOfLife : public olc::PixelGameEngine
{
public:
	GameOfLife()
	{
		sAppName = "GameOfLife";
	}

	~GameOfLife()
	{
		delete[] cells;
		delete[] cellsCopy;
		delete[] cellClickedPreviousFrame;
	}

private:
	static constexpr int cellSize = 5;
	olc::vi2d mapSize;
	bool editing = true;
	bool* cells;
	bool* cellClickedPreviousFrame;
	olc::vi2d previousMousePosition;
	bool previousMouseState;

	//all the cells need to be updated at once.
	//to to this a copy of cells will be made 
	//and cells will be updated while reading
	//data from cellsCopy
	bool* cellsCopy;

private:
	int GetTotalAliveNeighbours(int x, int y)
	{
		auto offset = [&](int ox, int oy)
		{
			return (y + oy) * mapSize.x + (x + ox);
		};

		return cellsCopy[offset(-1, 0)] +  //left
			cellsCopy[offset(1, 0)] +      // right
			cellsCopy[offset(0, 1)] +      //bottom
			cellsCopy[offset(0, -1)] +     //top
			cellsCopy[offset(1, 1)] +      //bottom-right
			cellsCopy[offset(1, -1)] +     //top-right
			cellsCopy[offset(-1, -1)] +    //top-left
			cellsCopy[offset(-1, 1)];      //bottom-lef
	}

	void ResetCells()
	{
		for (int x = 0; x < mapSize.x; x++)
			for (int y = 0; y < mapSize.y; y++)
			{
				cells[y * mapSize.x + x] = false;
			}

		for (int i = 0; i < mapSize.x * mapSize.y; i++)
		{
			cellsCopy[i] = cells[i];
		}

		for (int i = 0; i < mapSize.x * mapSize.y; i++)
		{
			cellClickedPreviousFrame[i] = false;
		}
	}

public:
	bool OnUserCreate() override
	{
		mapSize.x = ScreenWidth() / cellSize;
		mapSize.y = ScreenHeight() / cellSize;

		cells = new bool[(size_t)mapSize.x * mapSize.y];
		cellsCopy = new bool[(size_t)mapSize.x * mapSize.y];
		cellClickedPreviousFrame = new bool[(size_t)mapSize.x * mapSize.y];

		ResetCells();
		previousMousePosition = GetMousePos();
		previousMouseState = false;

		return true;
	}
	bool OnUserUpdate(float elapsedTime) override
	{
		olc::vi2d mouse = GetMousePos();

		if (editing)
		{
			olc::vi2d cell = mouse / cellSize;

			if (GetMouse(0).bHeld)
			{
				if (cell.x > 0 && cell.x < mapSize.x - 1 &&
					cell.y > 0 && cell.y < mapSize.x - 1)
				{
					if (!cellClickedPreviousFrame[cell.y * mapSize.x + cell.x])
					{
						cellClickedPreviousFrame[cell.y * mapSize.x + cell.x] = true;
						cells[cell.y * mapSize.x + cell.x] = true;
						cellsCopy[cell.y * mapSize.x + cell.x] = true;
					}
					if (GetMouse(0).bReleased)
					{
						cellClickedPreviousFrame[cell.y * mapSize.x + cell.x] = false;
					}
				}
			}

			//fill in the blanks...
			if (previousMouseState)
			{
				olc::vf2d toMouse = mouse - previousMousePosition;
				olc::vf2d normDir = toMouse.norm();
				int mag = (int)toMouse.mag();
				if (mag != 0)
				{
					for (int i = 0; i < mag; i++)
					{
						olc::vi2d current = previousMousePosition + normDir * i;
						size_t index = (size_t)(current.y / cellSize) * mapSize.x + (current.x / cellSize);
						cellClickedPreviousFrame[index] = true;
						cells[index] = true;
						cellsCopy[index] = true;
					}
				}
			}
			previousMousePosition = mouse;
			previousMouseState = GetMouse(0).bHeld;
		}
		else
		{
			using namespace std::chrono_literals;
		    std::this_thread::sleep_for(100ms);

			for (int x = 1; x < mapSize.x - 1; x++)
				for (int y = 1; y < mapSize.y - 1; y++)
				{
					const int totalAliveNeighbours = [&]()
					{
						auto offset = [&](int ox, int oy)
						{
							return (y + oy) * mapSize.x + (x + ox);
						};

						return cellsCopy[offset(-1, 0)]  +  //left
							   cellsCopy[offset(1, 0)]   +  // right
							   cellsCopy[offset(0, 1)]   +  //bottom
							   cellsCopy[offset(0, -1)]  +  //top
							   cellsCopy[offset(1, 1)]   +  //bottom-right
							   cellsCopy[offset(1, -1)]  +  //top-right
							   cellsCopy[offset(-1, -1)] +  //top-left
							   cellsCopy[offset(-1, 1)];    //bottom-lef;
					}();

					if (cellsCopy[y * mapSize.x + x])
					{
						if (totalAliveNeighbours < 2 || totalAliveNeighbours > 3)
						{
							cells[y * mapSize.x + x] = false;
						}
					}
					else
					{
						if (totalAliveNeighbours == 3)
						{
							cells[y * mapSize.x + x] = true;
						}
					}
				}

		    for (int i = 0; i < mapSize.x * mapSize.y; i++)
			{
				cellsCopy[i] = cells[i];
			}
		}

		if (GetKey(olc::SPACE).bPressed)
		{
			editing = !editing;
		}

		if (GetKey(olc::C).bPressed && editing)
		{
			ResetCells();
		}

		Clear(olc::BLACK);
		for (int x = 0; x < mapSize.x; x++)
			for (int y = 0; y < mapSize.y; y++)
			{
				if (cells[y * mapSize.x + x])
				{
					FillRect(x * cellSize, y * cellSize, cellSize, cellSize, { 255, 253, 208 });
				}

				if(x < 1 || x > mapSize.x - 2 || y < 1 || y > mapSize.y - 2)
				{
					FillRect(x * cellSize, y * cellSize, cellSize, cellSize, { 100, 10, 10 });
				}
			}

		DrawString(10, 10, editing ? "Editing" : "Playing");
		

		return true;
	}
};
int main()
{
	GameOfLife gol;
	if (gol.Construct(600, 300, 2, 2))
		gol.Start();
	return 0;
}
