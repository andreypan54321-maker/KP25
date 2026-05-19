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

// вивід двох полів поряд
void printBothBoards(const Board &player, const Board &computer)
{
    cout << "\n  Ваше поле              Поле суперника\n";
    cout << "   A B C D E F G H I J     A B C D E F G H I J\n";
    for (int i = 0; i < player.size; ++i)
    {
        // ліве поле
        cout << (i < 9 ? " " : "") << i + 1 << " ";
        for (int j = 0; j < player.size; ++j)
        {
            char ch = '.';
            switch (player.grid[i][j])
            {
            case Cell::Ship: ch = '#'; break;
            case Cell::Hit:  ch = 'X'; break;
            case Cell::Miss: ch = '*'; break;
            default: break;
            }
            cout << ch << ' ';
        }

        cout << "    ";

        // праве поле (кораблі приховані)
        cout << (i < 9 ? " " : "") << i + 1 << " ";
        for (int j = 0; j < computer.size; ++j)
        {
            char ch = '.';
            switch (computer.grid[i][j])
            {
            case Cell::Ship: ch = '.'; break; // приховано
            case Cell::Hit:  ch = 'X'; break;
            case Cell::Miss: ch = '*'; break;
            default: break;
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

// AI — спочатку добиває після влучання, потім стріляє випадково
struct AI
{
    vector<pair<int,int>> targets; // клітинки для добивання
    vector<vector<bool>> shot;     // вже стріляли сюди
    mt19937 rng;

    AI(int size) : shot(size, vector<bool>(size, false)), rng(random_device{}()) {}

    pair<int,int> nextShot(int size)
    {
        // якщо є цілі для добивання — беремо першу
        while (!targets.empty())
        {
            auto [r, c] = targets.back();
            targets.pop_back();
            if (r >= 0 && r < size && c >= 0 && c < size && !shot[r][c])
                return {r, c};
        }

        // інакше — випадковий постріл
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
        // додаємо сусідні клітинки як цілі
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

// хід гравця
bool playerTurn(Board &computer)
{
    while (true)
    {
        cout << "Ваш хід. Введіть координати (наприклад A5): ";
        string input;
        cin >> input;

        if (input.size() < 2)
        {
            cout << "Невірний формат!\n";
            continue;
        }

        char colCh = toupper(input[0]);
        if (colCh < 'A' || colCh > 'J')
        {
            cout << "Невірний стовпець!\n";
            continue;
        }

        int row;
        try {
            row = stoi(input.substr(1));
        } catch (...) {
            cout << "Невірний рядок!\n";
            continue;
        }

        if (row < 1 || row > 10)
        {
            cout << "Рядок від 1 до 10!\n";
            continue;
        }

        int r = row - 1, c = colCh - 'A';

        try {
            bool hit = computer.shoot(r, c);
            cout << (hit ? "Влучив!\n" : "Промах!\n");
            return hit;
        } catch (invalid_argument &e) {
            cout << e.what() << "\n";
        }
    }
}

// хід комп'ютера
bool aiTurn(Board &player, AI &ai)
{
    auto [r, c] = ai.nextShot(player.size);
    ai.shot[r][c] = true;

    bool hit = player.shoot(r, c);
    char col = 'A' + c;
    cout << "Комп'ютер стріляє: " << col << r + 1;
    cout << (hit ? " — Влучив!\n" : " — Промах!\n");

    if (hit)
        ai.registerHit(r, c, player.size);

    return hit;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    mt19937 rng(random_device{}());

    cout << "=== МОРСЬКИЙ БІЙ ===\n\n";

    Board playerBoard, computerBoard;
    AI ai(10);

    // розстановка кораблів гравця
    cout << "Розставте свої кораблі.\n";
    cout << "1 - вручну  2 - автоматично: ";
    int choice;
    cin >> choice;

    if (choice == 1)
        placeManual(playerBoard);
    else
        placeRandom(playerBoard, rng);

    placeRandom(computerBoard, rng);

    cout << "\nГра починається!\n";

    // головний ігровий цикл
    while (true)
    {
        printBothBoards(playerBoard, computerBoard);

        // хід гравця
        bool playerHit = playerTurn(computerBoard);
        if (computerBoard.allSunk())
        {
            printBothBoards(playerBoard, computerBoard);
            cout << "Вітаємо! Ви перемогли!\n";
            break;
        }
        if (!playerHit)
        {
            // хід комп'ютера тільки якщо гравець промахнувся
            cout << "\nХід комп'ютера:\n";
            bool aiHit = aiTurn(playerBoard, ai);
            (void)aiHit;
            if (playerBoard.allSunk())
            {
                printBothBoards(playerBoard, computerBoard);
                cout << "Комп'ютер переміг. Спробуйте ще раз!\n";
                break;
            }
        }
    }

    return 0;
}
