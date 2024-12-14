#include "cui.hpp"
#include <iostream>
#include <time.h>
#include <vector>

#define MAP_WIDTH 20
#define MAP_HEIGHT 20

static char ch = ' ';
static bool upd = false;

struct object
{
  int x;
  int y;
  char c;
  int hp;
  int attack;
};

struct object user = {5, 5, '@'};
struct object objects[] = {
    {10, 10, 'X', 10, 10},
    {15, 15, 'Y', 10, 10},
    {5, 15, 'Z', 10, 10},
    {15, 5, 'W', 10, 10},
};

// マップデータ
// '.': 通路
// '#': 壁
// '*': 岩

const std::string map[] = {
    // 2345678901234567890
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

// 仮想画面バッファ
std::vector<std::string> screen_buffer;

int screen_width = 0;
int screen_height = 0;

// 画面バッファを初期化
void init_screen_buffer(int width, int height)
{
  screen_width = width;
  screen_height = height;
  screen_buffer.resize(height, std::string(width, ' '));
}

void clear_screen_buffer()
{
  for (auto &line : screen_buffer)
  {
    line = std::string(screen_width, ' ');
  }
}

// 画面バッファに文字を描画
void draw_to_buffer(int x, int y, const std::string &str)
{
  if (y >= 0 && y < screen_buffer.size() && x >= 0 && x + str.size() <= screen_buffer[y].size())
  {
    screen_buffer[y].replace(x, str.size(), str);
  }
}

// 画面バッファの指定位置の文字を取得
char get_from_buffer(int x, int y)
{
  if (y >= 0 && y < screen_buffer.size() && x >= 0 && x < screen_buffer[y].size())
  {
    return screen_buffer[y][x];
  }
  return '?';
}

// 画面バッファを端末に出力
void render_screen(int x, int y)
{
  cui_gotoxy(x, y);
  for (const auto &line : screen_buffer)
  {
    std::cout << line << std::endl;
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
      std::string c = map[y].substr(x, 1);
      draw_to_buffer(x, y, c);
    }
  }
}

void draw_objects(void)
{
  draw_to_buffer(user.x, user.y, std::string(1, user.c));
  // 画面バッファにオブジェクトを描画
  for (const auto &obj : objects)
  {
    draw_to_buffer(obj.x, obj.y, std::string(1, obj.c));
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
  int x = user.x;
  int y = user.y;
  switch (ch)
  {
  case 'h':
    if (x > 1)
      x--;
    break;
  case 'j':
    if (y < MAP_HEIGHT)
      y++;
    break;
  case 'k':
    if (y > 1)
      y--;
    break;
  case 'l':
    if (x < MAP_WIDTH)
      x++;
    break;
  default:
    break;
  }
  if (!check_collision(x, y))
  {
    user.x = x;
    user.y = y;
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

  cui_clear_screen();
  clear_screen_buffer();
  draw_map();
  draw_objects();
  draw_info();
  render_screen(1, 3);
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
