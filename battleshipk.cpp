#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <stdexcept>

using namespace std;

enum class Cell // стан клітинки
{
	Empty,
	Ship,
	Hit,
	Miss
};

struct Board // ігрове поле
{
	vector<vector<Cell>> grid;
	int size;

	explicit Board(int sz = 10) : size(sz), grid(sz, vector<Cell>(sz, Cell::Empty)) {}

	auto &at(int r, int c) { return grid[r][c]; }
	const auto &at(int r, int c) const { return grid[r][c]; }

	bool inBounds(int r, int c) const // проверка полів
	{
		return r >= 0 && r < size && c >= 0 && c < size;
	}

	bool canPlace(int r, int c, int len, bool horiz) const
	{
		for (int i = 0; i < len; ++i)
		{
			auto [row, col] = horiz ? pair{r, c + i} : pair{r + i, c};
			if (!inBounds(row, col))
				return false;
			for (int dr = -1; dr <= 1; ++dr)
				for (int dc = -1; dc <= 1; ++dc)
				{
					int nr = row + dr, nc = col + dc;
					if (inBounds(nr, nc) && grid[nr][nc] == Cell::Ship)
						return false;
				}
		}
		return true;
	}

	void placeShip(int r, int c, int len, bool horiz)
	{
		for (int i = 0; i < len; ++i)
		{
			auto [row, col] = horiz ? pair{r, c + i} : pair{r + i, c};
			grid[row][col] = Cell::Ship;
		}
	}

	bool shoot(int r, int c)
	{
		auto &cell = grid[r][c];
		if (cell == Cell::Ship)
		{
			cell = Cell::Hit;
			return true;
		}
		if (cell == Cell::Empty)
		{
			cell = Cell::Miss;
			return false;
		}
		throw invalid_argument("Вже стріляв сюди!");
	}

	bool allSunk() const
	{
		for (auto &row : grid)
			for (auto cell : row)
				if (cell == Cell::Ship)
					return false;
		return true;
	}

	void print(bool hide = false) const
	{
		cout << "   A B C D E F G H I J\n";
		for (int i = 0; i < size; ++i)
		{
			cout << (i < 9 ? " " : "") << i + 1 << " ";
			for (int j = 0; j < size; ++j)
			{
				char ch = '.';
				switch (grid[i][j])
				{
				case Cell::Ship:
					ch = hide ? '.' : '#';
					break;
				case Cell::Hit:
					ch = 'X';
					break;
				case Cell::Miss:
					ch = '*';
					break;
				default:
					break;
				}
				cout << ch << ' ';
			}
			cout << '\n';
		}
	}
};

void placeRandom(Board &b, mt19937 &rng)
{
	const vector<int> fleet = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
	uniform_int_distribution<int> distPos(0, b.size - 1);
	uniform_int_distribution<int> distDir(0, 1);

	for (int len : fleet)
	{
		while (true)
		{
			int r = distPos(rng), c = distPos(rng);
			bool h = distDir(rng);
			if (b.canPlace(r, c, len, h))
			{
				b.placeShip(r, c, len, h);
				break;
			}
		}
	}
}

void placeManual(Board &b)
{
	const vector<int> fleet = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
	const vector<string> names = {
		"4-палубний",
		"3-палубний №1", "3-палубний №2",
		"2-палубний №1", "2-палубний №2", "2-палубний №3",
		"1-палубний №1", "1-палубний №2", "1-палубний №3", "1-палубний №4"};

	for (int k = 0; k < (int)fleet.size(); ++k)
	{
		int len = fleet[k];
		bool placed = false;

		while (!placed)
		{
			b.print();
			cout << "\nСтавимо: " << names[k] << " (розмір " << len << ")\n";

			cout << "Стовпець (A-J): ";
			char colCh;
			cin >> colCh;
			colCh = toupper(colCh);
			if (colCh < 'A' || colCh > 'J')
			{
				cout << "Невірно!\n";
				continue;
			}

			cout << "Рядок (1-10): ";
			int row;
			cin >> row;
			if (row < 1 || row > 10)
			{
				cout << "Невірно!\n";
				continue;
			}

			bool horiz = true;
			if (len > 1)
			{
				cout << "Напрямок г/в: ";
				string d;
				cin >> d;
				horiz = (d == "г" || d == "Г" || d == "g" || d == "G");
			}

			int r = row - 1, c = colCh - 'A';
			if (b.canPlace(r, c, len, horiz))
			{
				b.placeShip(r, c, len, horiz);
				placed = true;
			}
			else
			{
				cout << "Сюди не можна!\n";
			}
		}
	}
}
