#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <stdexcept>
#include <windows.h>

using namespace std;

enum class Cell
{
    Empty,
    Ship,
    Hit,
    Miss
};

struct Board
{
    vector<vector<Cell>> grid;
    int size;

    explicit Board(int sz = 10) : size(sz), grid(sz, vector<Cell>(sz, Cell::Empty)) {}

    auto &at(int r, int c) { return grid[r][c]; }
    const auto &at(int r, int c) const { return grid[r][c]; }

    bool inBounds(int r, int c) const
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

void printBothBoards(const Board &player, const Board &computer)
{
    cout << "\n  Ваше поле              Поле суперника\n";
    cout << "   A B C D E F G H I J        A B C D E F G H I J\n";
    for (int i = 0; i < player.size; ++i)
    {
        cout << (i < 9 ? " " : "") << i + 1 << " ";
        for (int j = 0; j < player.size; ++j)
        {
            char ch = '.';
            switch (player.grid[i][j])
            {
            case Cell::Ship:
                ch = '#';
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

        cout << "    ";

        cout << (i < 9 ? " " : "") << i + 1 << " ";
        for (int j = 0; j < computer.size; ++j)
        {
            char ch = '.';
            switch (computer.grid[i][j])
            {
            case Cell::Ship:
                ch = '.';
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
    cout << '\n';
}

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
    for (int k = 0; k < (int)fleet.size(); ++k)
    {
        int len = fleet[k];
        bool placed = false;
        while (!placed)
        {
            b.print();
            cout << "\nСтавимо корабель (розмір " << len << ")\n";
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
            if (!(cin >> row) || row < 1 || row > 10)
            {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Невірно!\n";
                continue;
            }
            bool horiz = true;
            if (len > 1)
            {
                cout << "Напрямок г/в: ";
                string d;
                cin >> d;
                horiz = (d == "г" || d == "Г");
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

struct AI
{
    vector<pair<int, int>> targets;
    vector<vector<bool>> shot;
    mt19937 rng;
    AI(int size) : shot(size, vector<bool>(size, false)), rng(random_device{}()) {}
    pair<int, int> nextShot(int size)
    {
        while (!targets.empty())
        {
            auto [r, c] = targets.back();
            targets.pop_back();
            if (r >= 0 && r < size && c >= 0 && c < size && !shot[r][c])
                return {r, c};
        }
        uniform_int_distribution<int> dist(0, size - 1);
        while (true)
        {
            int r = dist(rng), c = dist(rng);
            if (!shot[r][c])
                return {r, c};
        }
    }
    void registerHit(int r, int c, int size)
    {
        const int dr[] = {-1, 1, 0, 0};
        const int dc[] = {0, 0, -1, 1};
        for (int i = 0; i < 4; ++i)
        {
            int nr = r + dr[i], nc = c + dc[i];
            if (nr >= 0 && nr < size && nc >= 0 && nc < size && !shot[nr][nc])
                targets.push_back({nr, nc});
        }
    }
};

bool playerTurn(Board &computer)
{
    while (true)
    {
        cout << "Ваш хід (наприклад A5): ";
        string input;
        cin >> input;
        if (input.size() < 2)
            continue;
        char colCh = toupper(input[0]);
        int row = stoi(input.substr(1));
        if (colCh < 'A' || colCh > 'J' || row < 1 || row > 10)
            continue;
        try
        {
            return computer.shoot(row - 1, colCh - 'A');
        }
        catch (...)
        {
            cout << "Вже стріляв!\n";
        }
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    mt19937 rng(random_device{}());
    cout << "=== МОРСЬКИЙ БІЙ ===\n\n";

    Board playerBoard, computerBoard;
    AI ai(10);

    cout << "1 - вручну  2 - автоматично: ";
    int choice;
    cin >> choice;
    if (choice == 1)
        placeManual(playerBoard);
    else
        placeRandom(playerBoard, rng);

    placeRandom(computerBoard, rng);

    bool gameOver = false;

    while (!gameOver)
    {
        printBothBoards(playerBoard, computerBoard);

        if (playerTurn(computerBoard))
        {
            cout << "Влучання! Ви стріляєте ще раз.\n";
            if (computerBoard.allSunk())
            {
                printBothBoards(playerBoard, computerBoard);
                cout << "\n*** ПЕРЕМОГА! Ви знищили флот супротивника! ***\n";
                gameOver = true;
            }
        }
        else
        {
            cout << "Промах! Хід переходить до комп'ютера.\n\n";

            bool botTurn = true;
            while (botTurn && !gameOver)
            {
                auto [br, bc] = ai.nextShot(playerBoard.size);
                ai.shot[br][bc] = true;

                cout << "Комп'ютер стріляє у " << char(bc + 'A') << br + 1 << "... ";

                if (playerBoard.shoot(br, bc))
                {
                    cout << "Влучання!\n";
                    ai.registerHit(br, bc, playerBoard.size);

                    if (playerBoard.allSunk())
                    {
                        printBothBoards(playerBoard, computerBoard);
                        cout << "\n*** ПОРАЗКА! Комп'ютер знищив ваш флот! ***\n";
                        gameOver = true;
                    }
                }
                else
                {
                    cout << "Промах!\n";
                    botTurn = false;
                }
            }
        }
    }

    cout << "\n\nНатисніть Enter, щоб вийти...";
    cin.ignore(10000, '\n');
    cin.get();
    return 0;
}
