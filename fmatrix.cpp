#include <iostream>
#include <vector>
#include <ncurses.h>
#include <getopt.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <algorithm>

#define DEFAULT_SPEED 50

struct Droplet
{
    int x, y, length, speed, counter;
};

void print_help()
{
    std::cout << "Usage: fmatrix [options]\n"
              << "Options:\n"
              << "  -s, --speed <ms>   Delay between frames in ms (default: 50)\n"
              << "  -h, --help         Display this help and exit\n"
              << "  -v, --version      Display version information and exit\n";
}

int main(int argc, char **argv)
{
    int speed = DEFAULT_SPEED;

    static struct option long_options[] = {
        {"speed", required_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "s:hv", long_options, nullptr)) != -1)
    {
        switch (opt)
        {
        case 's':
            speed = std::stoi(optarg);
            break;
        case 'h':
            print_help();
            return 0;
        case 'v':
            std::cout << "fmatrix version 1.1.0 (WhiteHead Edition)\n";
            return 0;
        default:
            return 1;
        }
    }

    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    start_color();
    use_default_colors();

    // Цветовые пары: 1 - Зеленый на прозрачном, 2 - Белый на прозрачном
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_WHITE, -1);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    std::vector<Droplet> droplets;
    auto init_droplets = [&]()
    {
        droplets.clear();
        for (int i = 0; i < cols; i += 2)
        {
            droplets.push_back({i, rand() % rows, rand() % (rows / 2) + 5, rand() % 3 + 1, 0});
        }
    };

    init_droplets();
    srand(time(NULL));

    bool running = true;
    while (running)
    {
        int new_rows, new_cols;
        getmaxyx(stdscr, new_rows, new_cols);

        if (new_rows != rows || new_cols != cols)
        {
            rows = new_rows;
            cols = new_cols;
            init_droplets();
            clear();
        }

        for (auto &d : droplets)
        {
            if (++d.counter >= d.speed)
            {
                d.counter = 0;

                // 1. Отрисовка "тела" капли (зеленым) чуть выше головы
                attron(COLOR_PAIR(1));
                int body_pos = (d.y - 1 < 0) ? rows - 1 : d.y - 1;
                mvaddch(body_pos, d.x, 33 + rand() % 94);
                attroff(COLOR_PAIR(1));

                // 2. Отрисовка "головы" капли (белым)
                attron(COLOR_PAIR(2) | A_BOLD);
                mvaddch(d.y, d.x, 33 + rand() % 94);
                attroff(COLOR_PAIR(2) | A_BOLD);

                // 3. Стирание хвоста
                int tail = d.y - d.length;
                if (tail >= 0)
                    mvaddch(tail, d.x, ' ');
                else
                    mvaddch(tail + rows, d.x, ' ');

                d.y++;
                if (d.y >= rows)
                    d.y = 0;
            }
        }

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(speed));
        if (getch() == 'q')
            running = false;
    }

    endwin();
    return 0;
}