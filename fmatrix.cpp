#include <iostream>
#include <vector>
#include <ncurses.h>
#include <getopt.h>
#include <ctime>
#include <chrono>
#include <thread>

#define DEFAULT_SPEED 50

struct Droplet {
    int x, y, length, speed, counter;
    int color_offset; // Для режима радуги
};

void print_help() {
    std::cout << "fmatrix - Fast Matrix C++ Implementation\n"
              << "Usage: fmatrix [options]\n"
              << "Options:\n"
              << "  -u delay      Screen update delay (1-100, default 50ms)\n"
              << "  -C color      Color: green, red, blue, white, yellow, cyan, magenta\n"
              << "  -r            Rainbow mode: colors shift as they fall\n"
              << "  -b            Enable bold characters\n"
              << "  -s            Screensaver mode: exit on any key\n"
              << "  -h            Print usage and exit\n"
              << "  -V            Print version information and exit\n";
}

int main(int argc, char **argv) {
    int speed = DEFAULT_SPEED;
    int base_color = COLOR_GREEN;
    bool rainbow = false;
    bool screensaver = false;
    bool bold = false;

    static struct option long_options[] = {
        {"speed", required_argument, 0, 'u'},
        {"color", required_argument, 0, 'C'},
        {"rainbow", no_argument, 0, 'r'},
        {"bold", no_argument, 0, 'b'},
        {"screensaver", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "u:C:rbshV", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'u': speed = std::stoi(optarg); break;
            case 'r': rainbow = true; break;
            case 'b': bold = true; break;
            case 's': screensaver = true; break;
            case 'C': {
                std::string c = optarg;
                if (c == "red") base_color = COLOR_RED;
                else if (c == "blue") base_color = COLOR_BLUE;
                else if (c == "yellow") base_color = COLOR_YELLOW;
                else if (c == "cyan") base_color = COLOR_CYAN;
                else if (c == "magenta") base_color = COLOR_MAGENTA;
                else if (c == "white") base_color = COLOR_WHITE;
                else base_color = COLOR_GREEN;
                break;
            }
            case 'h': print_help(); return 0;
            case 'V': std::cout << "fmatrix v1.2.0 (Pro Edition)\n"; return 0;
            default: return 1;
        }
    }

    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    start_color();
    use_default_colors();

    // Инициализация цветов
    init_pair(1, base_color, -1); // Основной
    init_pair(2, COLOR_WHITE, -1); // Голова
    // Пары для радуги
    for(int i = 1; i <= 6; i++) init_pair(10+i, i, -1);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    std::vector<Droplet> droplets;
    auto init_droplets = [&]() {
        droplets.clear();
        for (int i = 0; i < cols; i += 2) {
            droplets.push_back({i, rand() % rows, rand() % (rows / 2) + 5, rand() % 3 + 1, 0, rand() % 6});
        }
    };

    init_droplets();
    srand(time(NULL));

    while (true) {
        int new_rows, new_cols;
        getmaxyx(stdscr, new_rows, new_cols);
        if (new_rows != rows || new_cols != cols) {
            rows = new_rows; cols = new_cols;
            init_droplets();
            clear();
        }

        for (auto &d : droplets) {
            if (++d.counter >= d.speed) {
                d.counter = 0;

                // Выбор цвета (Радуга или статика)
                int pair = rainbow ? (11 + (d.y + d.color_offset) % 6) : 1;
                attron(COLOR_PAIR(pair));
                if (bold) attron(A_BOLD);

                int body_pos = (d.y - 1 < 0) ? rows - 1 : d.y - 1;
                mvaddch(body_pos, d.x, 33 + rand() % 94);
                
                if (bold) attroff(A_BOLD);
                attroff(COLOR_PAIR(pair));

                // Голова всегда белая и жирная
                attron(COLOR_PAIR(2) | A_BOLD);
                mvaddch(d.y, d.x, 33 + rand() % 94);
                attroff(COLOR_PAIR(2) | A_BOLD);

                int tail = d.y - d.length;
                if (tail >= 0) mvaddch(tail, d.x, ' ');
                else mvaddch(tail + rows, d.x, ' ');

                d.y++;
                if (d.y >= rows) {
                    d.y = 0;
                    d.color_offset = rand() % 6; // Смена начального цвета радуги
                }
            }
        }

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(speed));
        
        int ch = getch();
        if (screensaver && ch != ERR) break;
        if (ch == 'q') break;
    }

    endwin();
    return 0;
}