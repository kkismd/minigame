#include "cui.hpp"
#include <iostream>
#include <time.h>

#define MAP_WIDTH 20
#define MAP_HEIGHT 20

static char ch = ' ';
static bool upd = false;

int user_x = 1;
int user_y = 1;

// 文字列定数でマップを定義
const std::string map[] = {
  //2345678901234567890
  "                    " // 1
  "                    " // 2
  "                    " // 3
  "                    " // 4
  "                    " // 5
  "                    " // 6
  "                    " // 7
  "                    " // 8
  "                    " // 9
  "                    " // 10
  "                    " // 11
  "                    " // 12
  "                    " // 13
  "                    " // 14
  "                    " // 15
  "                    " // 16
  "                    " // 17
  "                    " // 18
  "                    " // 19
  "                    " // 20
};

// 仮想画面バッファ
std::vector<std::string> screen_buffer;

// 画面バッファを初期化
void init_screen_buffer(int width, int height) {
    screen_buffer.resize(height, std::string(width, '?'));
}

// 画面バッファに文字を描画
void draw_to_buffer(int x, int y, const std::string& str) {
  if (y >= 0 && y < screen_buffer.size() && x >= 0 && x + str.size() <= screen_buffer[y].size())
  {
    screen_buffer[y].replace(x, str.size(), str);
  }
}

// 画面バッファの指定位置の文字を取得
char get_from_buffer(int x, int y) {
  if (y >= 0 && y < screen_buffer.size() && x >= 0 && x < screen_buffer[y].size())
  {
    return screen_buffer[y][x];
  }
  return '?';
}

// 画面バッファを端末に出力
void render_screen(int x, int y) {
    cui_gotoxy(x, y);
    for (const auto& line : screen_buffer) {
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
  for (int y = 0; y <= MAP_HEIGHT + 1; y++)
  {
    for (int x = 0; x <= MAP_WIDTH + 1; x++)
    {
      draw_to_buffer(x, y, ".");
    }
  }
}

void draw_objects(void)
{
  // 画面バッファにオブジェクトを描画
  draw_to_buffer(user_x, user_y, "@");
  draw_to_buffer(5, 5, "O");
  draw_to_buffer(10, 10, "X");
  draw_to_buffer(15, 15, "Y");
}

void controll(char input)
{
  switch (ch)
  {
  case 'h':
    if (user_x > 1)
      user_x--;
    break;
  case 'j':
    if (user_y < MAP_HEIGHT)
      user_y++;
    break;
  case 'k':
    if (user_y > 1)
      user_y--;
    break;
  case 'l':
    if (user_x < MAP_WIDTH)
      user_x++;
    break;
  default:
    break;
  }
}

void update(void)
{
  int key = cui_getch_nowait();
  if (key != -1)
  {
    ch = key;
    controll(ch);
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
  cui_gotoxy(1, 1);
  std::cout << "ch = " << ch;

  cui_gotoxy(1, 3);
  draw_map();
  draw_objects();
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
  updated();
  cui_cursor_off();
  init_screen_buffer(MAP_WIDTH + 2, MAP_HEIGHT + 2);

  while (true)
  {
    update();
    draw();
    nanosleep(&req, NULL); // 1/30秒スリープ
  }
}
