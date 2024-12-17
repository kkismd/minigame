#include "cui.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <time.h>

#define MAP_WIDTH 20
#define MAP_HEIGHT 20

static char ch = ' ';
static bool upd = false;

struct Player
{
  int x;
  int y;
  int hp;
  int attack;
};

struct Enemy
{
  int x;
  int y;
  char c;
  int hp;
  int attack;
};

struct Wall
{
};
struct Rock
{
};
struct Floor
{
};

using Cell = std::variant<std::monostate, Wall, Rock, Floor, Player, Enemy>;

struct Player player = {5, 5, 30, 5};
struct Enemy enemies[] = {
    {12, 5, 'A', 10, 3},
    {10, 10, 'B', 10, 3},
    {14, 14, 'C', 10, 3},
    {17, 17, 'X', 10, 3},
};

// 仮想画面バッファ

std::vector<Cell> screen_buffer;

int screen_width = 0;
int screen_height = 0;

// マップデータ
// '.': 通路 (Floor)
// '#': 壁 (Wall)
// '*': 岩 (Rock)

const std::string map[] = {
    // 345678901234567890
    "####################", // 1
    "#..................#", // 2
    "#..*****...........#", // 3
    "#......*...........#", // 4
    "#......*...........#", // 5
    "#......*...........#", // 6
    "#......*...........#", // 7
    "#..................#", // 8
    "#..................#", // 9
    "#..................#", // 10
    "#..................#", // 11
    "#............*.....#", // 12
    "#............*.....#", // 13
    "#............*.....#", // 14
    "#............*.....#", // 15
    "#............*.....#", // 16
    "#....*********.....#", // 17
    "#..................#", // 18
    "#..................#", // 19
    "####################", // 20
};

// 画面バッファを初期化
void init_screen_buffer(int width, int height)
{
  screen_width = width;
  screen_height = height;
  screen_buffer.resize(height * width, std::monostate{});
}

void clear_screen_buffer()
{
  screen_buffer.assign(screen_width * screen_height, std::monostate{});
}

char cell_to_char(Cell cell)
{
  if (std::holds_alternative<Wall>(cell))
  {
    return '#';
  }
  else if (std::holds_alternative<Rock>(cell))
  {
    return '*';
  }
  else if (std::holds_alternative<Floor>(cell))
  {
    return '.';
  }
  else if (std::holds_alternative<Player>(cell))
  {
    return '@';
  }
  else if (std::holds_alternative<Enemy>(cell))
  {
    return std::get<Enemy>(cell).c;
  }
  else
  {
    return ' ';
  }
}

void char_to_cell(char c, Cell &cell)
{
  switch (c)
  {
  case '#':
    cell = Wall{};
    break;
  case '*':
    cell = Rock{};
    break;
  case '.':
    cell = Floor{};
    break;
  case '@':
    cell = player;
    break;
  default:
    cell = std::monostate{};
    break;
  }
}

int buffer_index(int x, int y)
{
  return y * screen_width + x;
}

// 画面バッファに文字を描画
void draw_to_buffer(int x, int y, Cell cell)
{
  if (y >= 0 && y < screen_height && x >= 0 && x < screen_width)
  {
    screen_buffer[buffer_index(x, y)] = cell;
  }
}

bool is_in_screen(int x, int y)
{
  return x >= 0 && x < screen_width && y >= 0 && y < screen_height;
}

// 画面バッファの指定位置の文字を取得
char get_from_buffer(int x, int y)
{
  if (is_in_screen(x, y))
  {
    Cell cell = screen_buffer[buffer_index(x, y)];
    return cell_to_char(cell);
  }
  return ' ';
}

// 画面バッファを端末に出力
void render_screen(int x, int y)
{
  cui_gotoxy(x, y);
  for (int j = 0; j < screen_height; j++)
  {
    for (int i = 0; i < screen_width; i++)
    {
      // 画面バッファの属性を適用して出力
      Cell cell = screen_buffer[buffer_index(i, j)];
      std::cout << cell_to_char(cell);
    }
    std::cout << std::endl;
  }
}

bool is_updated()
{
  return upd;
}

void updated()
{
  upd = true;
}
void drawn()
{
  upd = false;
}

void draw_map(void)
{
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      // mapから画面バッファにコピー
      char c = map[y][x];
      Cell cell;
      char_to_cell(c, cell);
      draw_to_buffer(x, y, cell);
    }
  }
}

void draw_objects(void)
{
  draw_to_buffer(player.x, player.y, player);

  // 画面バッファにオブジェクトを描画
  for (const auto &enemy : enemies)
  {
    draw_to_buffer(enemy.x, enemy.y, enemy);
  }
}

void draw_info(void)
{
  cui_gotoxy(1, 1);
  std::cout << "ch = " << ch;
}

bool check_collision(int x, int y)
{
  if (get_from_buffer(x, y) == '.')
  {
    return false;
  }
  return true;
}

void move(char input)
{
  int x = player.x;
  int y = player.y;
  switch (ch)
  {
  case 'h':
    if (x > 1)
    {
      x--;
    }
    break;
  case 'j':
    if (y < MAP_HEIGHT)
    {
      y++;
    }
    break;
  case 'k':
    if (y > 1)
    {
      y--;
    }
    break;
  case 'l':
    if (x < MAP_WIDTH)
    {
      x++;
    }
    break;
  default:
    break;
  }
  if (!check_collision(x, y))
  {
    player.x = x;
    player.y = y;
  }
}

void update(void)
{
  int key = cui_getch_nowait();
  if (key != -1)
  {
    ch = key;
    move(ch);
    updated();
  }
  else if (ch == 'q')
  {
    cui_cursor_on();
    exit(0);
  }
  else
  {
    return;
  }
}

void draw(void)
{
  if (!is_updated())
    return;

  clear_screen_buffer();
  draw_map();
  draw_objects();
  draw_info();
  render_screen(1, 3);
  cui_clear_line();
  cui_gotoxy(1, 1);
  drawn();
}

int main()
{
  struct timespec req = {0};
  req.tv_sec = 0;
  req.tv_nsec = 33333333L; // 1/30秒 (33.33ミリ秒)

  cui_clear_screen();
  cui_cursor_off();
  init_screen_buffer(MAP_WIDTH + 2, MAP_HEIGHT + 2);
  updated();

  while (true)
  {
    update();
    draw();
    nanosleep(&req, NULL); // 1/30秒スリープ
  }
}
