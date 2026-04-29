#include <iostream>
#include <vector>
#include <ncurses.h>
#include <getopt.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <algorithm>

#define DEFAULT_SPEED 50
#define DEFAULT_COLOR COLOR_GREEN

struct Droplet {
    int x, y, length, speed, counter;
};

void print_help() {
    std::cout << "Usage: fmatrix [options]\n"
              << "Options:\n"
              << "  -s, --speed <ms>   Delay between frames in ms (default: 50)\n"
              << "  -c, --color <num>  Color: 1:Gr, 2:Red, 3:Blue, 4:White, 5:Yellow (default: 1)\n"
              << "  -h, --help         Display this help and exit\n"
              << "  -v, --version      Display version information and exit\n";
}

int main(int argc, char** argv) {
    int speed = DEFAULT_SPEED;
    int color_choice = DEFAULT_COLOR;

    static struct option long_options[] = {
        {"speed",   required_argument, 0, 's'},
        {"color",   required_argument, 0, 'c'},
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "s:c:hv", long_options, nullptr)) != -1) {
        switch (opt) {
            case 's': speed = std::stoi(optarg); break;
            case 'c': 
                switch(std::stoi(optarg)) {
                    case 1: color_choice = COLOR_GREEN; break;
                    case 2: color_choice = COLOR_RED; break;
                    case 3: color_choice = COLOR_BLUE; break;
                    case 4: color_choice = COLOR_WHITE; break;
                    case 5: color_choice = COLOR_YELLOW; break;
                }
                break;
            case 'h': print_help(); return 0;
            case 'v': std::cout << "fmatrix version 1.0.0\n"; return 0;
            default: return 1;
        }
    }

    // Инициализация ncurses
    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    start_color();
    use_default_colors(); // Важно для прозрачности
    init_pair(1, color_choice, -1);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    std::vector<Droplet> droplets;
    auto init_droplets = [&]() {
        droplets.clear();
        for (int i = 0; i < cols; i += 2) {
            droplets.push_back({i, rand() % rows, rand() % (rows / 2) + 5, rand() % 3 + 1, 0});
        }
    };

    init_droplets();
    srand(time(NULL));

    bool running = true;
    while (running) {
        int new_rows, new_cols;
        getmaxyx(stdscr, new_rows, new_cols);

        // Обработка ресайза окна без ломания картинки
        if (new_rows != rows || new_cols != cols) {
            rows = new_rows;
            cols = new_cols;
            init_droplets();
            clear();
        }

        for (auto &d : droplets) {
            if (++d.counter >= d.speed) {
                d.counter = 0;
                
                attron(COLOR_PAIR(1));
                // Рисуем новый символ
                mvaddch(d.y, d.x, 33 + rand() % 94);
                
                // Стираем хвост
                int tail = d.y - d.length;
                if (tail >= 0) mvaddch(tail, d.x, ' ');
                else if (tail + rows >= 0 && tail + rows < rows) mvaddch(tail + rows, d.x, ' ');

                d.y++;
                if (d.y >= rows) d.y = 0;
                attroff(COLOR_PAIR(1));
            }
        }

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(speed));

        if (getch() == 'q') running = false;
    }

    endwin();
    return 0;
}