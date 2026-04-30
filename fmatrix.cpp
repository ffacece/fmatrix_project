#include <iostream>
#include <vector>
#include <ncurses.h>
#include <getopt.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <clocale>

#define DEFAULT_SPEED 50

struct Droplet {
    int x, y, length, speed, counter;
    int color_offset;
};

const wchar_t kana[] = L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝ0123456789";

void print_help() {
    std::cout << "Usage: fmatrix -[bjLshVT] [-u delay] [-C color/-r]\n"
              << "fmatrix - Fast Matrix digital rain implementation\n\n"
              << "Options:\n"
              << "  -u delay      Set update delay in ms (1-100, default: 50)\n"
              << "  -C color      Set base color (green, red, blue, yellow, cyan, magenta, white)\n"
              << "  -T            Enable transparent background on startup\n"
              << "  -j            Use Japanese Katakana characters\n"
              << "  -r            Enable smooth rainbow mode\n"
              << "  -b            Enable bold characters\n"
              << "  -L            Lock mode (ignores input except system signals)\n"
              << "  -s            Screensaver mode (exit on any key press)\n"
              << "  -h            Display this help menu\n"
              << "  -V            Display version information\n\n"
              << "In-game controls:\n"
              << "  'q'           Quit the program\n"
              << "  't'           Toggle background transparency\n";
}

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");
    
    int speed = DEFAULT_SPEED;
    int base_color = COLOR_GREEN;
    bool is_transparent = false;
    bool rainbow = false, screensaver = false, bold = false, japanese = false, lock_mode = false;

    int opt;
    while ((opt = getopt(argc, argv, "u:C:rbjLshVT")) != -1) {
        switch (opt) {
            case 'u': speed = std::stoi(optarg); break;
            case 'r': rainbow = true; break;
            case 'b': bold = true; break;
            case 'j': japanese = true; break;
            case 'L': lock_mode = true; break;
            case 's': screensaver = true; break;
            case 'T': is_transparent = true; break; 
            case 'C': {
                std::string c = optarg;
                if (c == "red") base_color = COLOR_RED;
                else if (c == "blue") base_color = COLOR_BLUE;
                else if (c == "yellow") base_color = COLOR_YELLOW;
                else if (c == "cyan") base_color = COLOR_CYAN;
                else if (c == "magenta") base_color = COLOR_MAGENTA;
                else if (c == "white") base_color = COLOR_WHITE;
                break;
            }
            case 'h': print_help(); return 0;
            case 'V': std::cout << "fmatrix v1.2\n"; return 0;
            default: return 1;
        }
    }

    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    start_color();
    use_default_colors();

    /**
     * @param transparent If true, uses terminal's default background (-1).
     */
    auto update_colors = [&](bool transparent) {
        int bg = transparent ? -1 : COLOR_BLACK;
        init_pair(1, base_color, bg);
        init_pair(2, COLOR_WHITE, bg);
        if (rainbow) {
            std::vector<int> rb = {160, 196, 202, 208, 214, 220, 226, 190, 154, 118, 82, 46, 47, 48, 49, 45, 39, 33, 27, 21, 57, 93, 129, 165};
            for (int i = 0; i < (int)rb.size(); i++) init_pair(10 + i, rb[i], bg);
        }
        bkgd(COLOR_PAIR(1) | ' ');
    };

    update_colors(is_transparent);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    std::vector<Droplet> droplets;
    auto init_droplets = [&]() {
        droplets.clear();
        for (int i = 0; i < cols; i += 2) {
            droplets.push_back({i, rand() % rows, rand() % (rows / 2) + 5, rand() % 3 + 1, 0, rand() % 24});
        }
    };

    init_droplets();
    srand(time(NULL));

    auto get_char = [&](cchar_t* res, attr_t attr, short pair) {
        wchar_t wc = japanese ? kana[rand() % (sizeof(kana)/sizeof(wchar_t) - 1)] : (wchar_t)(33 + rand() % 94);
        setcchar(res, &wc, attr, pair, NULL);
    };

    while (true) {
        int new_rows, new_cols;
        getmaxyx(stdscr, new_rows, new_cols);
        
        if (new_rows != rows || new_cols != cols) {
            rows = new_rows; cols = new_cols;
            init_droplets();
            clear();
            bkgd(COLOR_PAIR(1) | ' '); 
        }

        for (auto &d : droplets) {
            if (++d.counter >= d.speed) {
                d.counter = 0;
                int pair = rainbow ? (10 + (d.y + d.color_offset) % 24) : 1;
                cchar_t gc;

                attr_t body_attr = (bold ? A_BOLD : A_NORMAL);
                get_char(&gc, body_attr, pair);
                mvadd_wch((d.y - 1 < 0 ? rows - 1 : d.y - 1), d.x, &gc);
                
                get_char(&gc, A_BOLD, 2);
                mvadd_wch(d.y, d.x, &gc);

                int tail = d.y - d.length;
                mvaddch((tail >= 0 ? tail : tail + rows), d.x, ' ');

                d.y = (d.y + 1) % rows;
                if (d.y == 0) d.color_offset = rand() % 24;
            }
        }

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(speed));
        
        int ch = getch();
        if (!lock_mode) {
            if (ch == 'q' || (screensaver && ch != ERR)) break;
            
            if (ch == 't') {
                is_transparent = !is_transparent;
                update_colors(is_transparent);
                clear(); 
            }
        }
    }

    endwin();
    return 0;
}