#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define WIDTH 40
#define HEIGHT 20
#define MAX_SNAKE (WIDTH * HEIGHT)

typedef enum {
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
} Direction;

typedef struct {
    int x;
    int y;
} Point;

static Point snake[MAX_SNAKE];
static Point food;
static int snake_length;
static int score;
static int game_over;
static Direction direction;
static int screen_width;
static int screen_height;
static int board_left;
static int board_top;
static int previous_board_left = -1;
static int previous_board_top = -1;

static void set_color(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

static void move_cursor_home(void) {
    COORD pos = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

static void move_cursor_to(int x, int y) {
    COORD pos;

    pos.X = (SHORT)x;
    pos.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

static void hide_cursor(void) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;

    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(console, &info);
}

static void clear_screen_buffer(void) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD home = {0, 0};
    DWORD written;
    DWORD cells;

    if (!GetConsoleScreenBufferInfo(console, &info)) {
        return;
    }

    cells = info.dwSize.X * info.dwSize.Y;
    FillConsoleOutputCharacter(console, ' ', cells, home, &written);
    FillConsoleOutputAttribute(console, info.wAttributes, cells, home, &written);
}

static void update_layout(void) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    int board_width = WIDTH + 2;
    int board_height = HEIGHT + 5;

    screen_width = 80;
    screen_height = 25;

    if (GetConsoleScreenBufferInfo(console, &info)) {
        screen_width = info.srWindow.Right - info.srWindow.Left + 1;
        screen_height = info.srWindow.Bottom - info.srWindow.Top + 1;
    }

    board_left = (screen_width - board_width) / 2;
    board_top = (screen_height - board_height) / 2;

    if (board_left < 0) {
        board_left = 0;
    }

    if (board_top < 0) {
        board_top = 0;
    }
}

static void print_indent(void) {
    int i;

    for (i = 0; i < board_left; i++) {
        putchar(' ');
    }
}

static void clear_if_layout_changed(void) {
    if (board_left == previous_board_left && board_top == previous_board_top) {
        return;
    }

    clear_screen_buffer();
    previous_board_left = board_left;
    previous_board_top = board_top;
}

static void center_text(int y, const char *text) {
    int x = (screen_width - (int)strlen(text)) / 2;

    if (x < 0) {
        x = 0;
    }

    move_cursor_to(x, y);
    printf("%s", text);
}

static int point_on_snake(int x, int y) {
    int i;

    for (i = 0; i < snake_length; i++) {
        if (snake[i].x == x && snake[i].y == y) {
            return 1;
        }
    }

    return 0;
}

static void spawn_food(void) {
    do {
        food.x = rand() % WIDTH;
        food.y = rand() % HEIGHT;
    } while (point_on_snake(food.x, food.y));
}

static void setup(void) {
    snake_length = 3;
    score = 0;
    game_over = 0;
    direction = RIGHT;

    snake[0].x = WIDTH / 2;
    snake[0].y = HEIGHT / 2;
    snake[1].x = snake[0].x - 1;
    snake[1].y = snake[0].y;
    snake[2].x = snake[0].x - 2;
    snake[2].y = snake[0].y;

    spawn_food();
}

static void draw(void) {
    int x;
    int y;
    int i;
    int printed;
    char head_char;
    char body_char;

    head_char = 'D';
    body_char = '-';

    update_layout();
    clear_if_layout_changed();

    move_cursor_to(board_left, board_top);
    set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    for (x = 0; x < WIDTH + 2; x++) {
        putchar('#');
    }
    putchar('\n');

    for (y = 0; y < HEIGHT; y++) {
        set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        move_cursor_to(board_left, board_top + y + 1);
        putchar('#');

        for (x = 0; x < WIDTH; x++) {
            printed = 0;

            if (snake[0].x == x && snake[0].y == y) {
                set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                if (direction == LEFT) {
                    head_char = 'C';
                } else if (direction == UP || direction == DOWN) {
                    head_char = 'O';
                    body_char = '|';
                } else {
                    head_char = 'D';
                }

                putchar(head_char);
                printed = 1;
            } else if (food.x == x && food.y == y) {
                set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
                putchar('*');
                printed = 1;
            } else {
                for (i = 1; i < snake_length; i++) {
                    if (snake[i].x == x && snake[i].y == y) {
                        set_color(FOREGROUND_GREEN);
                        putchar(body_char);
                        printed = 1;
                        break;
                    }
                }
            }

            if (!printed) {
                set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                putchar(' ');
            }
        }

        set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        putchar('#');
        putchar('\n');
    }

    set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    move_cursor_to(board_left, board_top + HEIGHT + 1);
    for (x = 0; x < WIDTH + 2; x++) {
        putchar('#');
    }
    putchar('\n');

    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    move_cursor_to(board_left, board_top + HEIGHT + 2);
    printf("Score: %d\n", score);
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    move_cursor_to(board_left, board_top + HEIGHT + 3);
    printf("Kontrol: W/A/S/D atau tombol panah | X untuk keluar\n");
}

static void input(void) {
    int key;

    if (!_kbhit()) {
        return;
    }

    key = _getch();

    if (key == 0 || key == 224) {
        key = _getch();

        switch (key) {
        case 72:
            if (direction != DOWN) direction = UP;
            break;
        case 80:
            if (direction != UP) direction = DOWN;
            break;
        case 75:
            if (direction != RIGHT) direction = LEFT;
            break;
        case 77:
            if (direction != LEFT) direction = RIGHT;
            break;
        default:
            break;
        }

        return;
    }

    switch (key) {
    case 'w':
    case 'W':
        if (direction != DOWN) direction = UP;
        break;
    case 's':
    case 'S':
        if (direction != UP) direction = DOWN;
        break;
    case 'a':
    case 'A':
        if (direction != RIGHT) direction = LEFT;
        break;
    case 'd':
    case 'D':
        if (direction != LEFT) direction = RIGHT;
        break;
    case 'x':
    case 'X':
        game_over = 1;
        break;
    default:
        break;
    }
}

static void update(void) {
    Point next_head = snake[0];
    int grows;
    int collision_limit;
    int i;

    switch (direction) {
    case LEFT:
        next_head.x--;
        break;
    case RIGHT:
        next_head.x++;
        break;
    case UP:
        next_head.y--;
        break;
    case DOWN:
        next_head.y++;
        break;
    default:
        break;
    }

    if (next_head.x < 0 || next_head.x >= WIDTH ||
        next_head.y < 0 || next_head.y >= HEIGHT) {
        game_over = 1;
        return;
    }

    grows = (next_head.x == food.x && next_head.y == food.y);
    collision_limit = grows ? snake_length : snake_length - 1;

    for (i = 0; i < collision_limit; i++) {
        if (snake[i].x == next_head.x && snake[i].y == next_head.y) {
            game_over = 1;
            return;
        }
    }

    if (grows) {
        if (snake_length >= MAX_SNAKE) {
            game_over = 1;
            return;
        }

        snake_length++;
    }

    for (i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = next_head;

    if (grows) {
        score += 10;

        if (snake_length >= MAX_SNAKE) {
            game_over = 1;
            return;
        }

        spawn_food();
    }
}

static void show_structure_table(void) {
    int key;
    int table_top;
    int table_left;

    update_layout();
    clear_screen_buffer();
    move_cursor_home();

    table_top = (screen_height - 13) / 2;
    table_left = (screen_width - 66) / 2;

    if (table_top < 0) {
        table_top = 0;
    }

    if (table_left < 0) {
        table_left = 0;
    }

    set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    center_text(table_top, "Contoh Penerapan Struktur Data pada Game Snake");

    set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    move_cursor_to(table_left, table_top + 2);
    printf("+----------------+-----------------------------------------------+");
    move_cursor_to(table_left, table_top + 3);
    printf("| Struktur Data  | Contoh pada Game                              |");
    move_cursor_to(table_left, table_top + 4);
    printf("+----------------+-----------------------------------------------+");

    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    move_cursor_to(table_left, table_top + 5);
    printf("| Stack          | Undo move, riwayat langkah                    |");
    move_cursor_to(table_left, table_top + 6);
    printf("| Queue          | Antrian input arah pemain                     |");
    move_cursor_to(table_left, table_top + 7);
    printf("| Array          | Penyimpanan posisi ular, map, dan skor        |");
    move_cursor_to(table_left, table_top + 8);
    printf("| Linked List    | Konsep badan ular yang saling terhubung       |");
    move_cursor_to(table_left, table_top + 9);
    printf("| Searching      | Mencari posisi makanan atau tabrakan badan    |");
    move_cursor_to(table_left, table_top + 10);
    printf("| Sorting        | Mengurutkan skor tertinggi                    |");
    move_cursor_to(table_left, table_top + 11);
    printf("| Tree           | Pemilihan menu: main, materi, keluar          |");
    move_cursor_to(table_left, table_top + 12);
    printf("| Graph          | Pergerakan pada grid/map permainan            |");

    set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    move_cursor_to(table_left, table_top + 13);
    printf("+----------------+-----------------------------------------------+");

    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    center_text(table_top + 15, "Tekan M untuk kembali ke menu");

    while (1) {
        key = _getch();

        if (key == 'm' || key == 'M') {
            return;
        }
    }
}

static int show_menu(void) {
    int key;
    int menu_top;

    while (1) {
        update_layout();
        clear_screen_buffer();
        move_cursor_home();

        menu_top = (screen_height - 9) / 2;
        if (menu_top < 0) {
            menu_top = 0;
        }

        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        center_text(menu_top, "==============================");
        center_text(menu_top + 1, "          SNAKE GAME          ");
        center_text(menu_top + 2, "==============================");

        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        center_text(menu_top + 4, "1. Mulai Main");

        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        center_text(menu_top + 5, "2. Keluar");
        center_text(menu_top + 6, "3. Struktur Data");
        center_text(menu_top + 8, "Pilih menu: 1 / 2 / 3");

        key = _getch();

        if (key == '1') {
            clear_screen_buffer();
            previous_board_left = -1;
            previous_board_top = -1;
            return 1;
        }

        if (key == '2' || key == 'x' || key == 'X') {
            return 0;
        }

        if (key == '3') {
            show_structure_table();
        }
    }
}

static void show_game_over(void) {
    int key;
    int message_top;

    update_layout();
    message_top = board_top + HEIGHT + 5;

    if (message_top + 2 >= screen_height) {
        message_top = board_top + HEIGHT + 2;
    }

    set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
    move_cursor_to(board_left, message_top);
    printf("Game over! Skor akhir: %d", score);

    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    move_cursor_to(board_left, message_top + 1);
    printf("Tekan M untuk menu, atau X untuk keluar.");

    while (1) {
        key = _getch();

        if (key == 'm' || key == 'M') {
            return;
        }

        if (key == 'x' || key == 'X') {
            exit(0);
        }
    }
}

int main(void) {
    srand((unsigned int)time(NULL));
    hide_cursor();
    system("cls");

    while (show_menu()) {
        setup();

        while (!game_over) {
            draw();
            input();
            update();
            Sleep(100);
        }

        draw();
        show_game_over();
    }

    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    clear_screen_buffer();
    move_cursor_home();
    return 0;
}
