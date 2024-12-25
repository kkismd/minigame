#include "cui.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <time.h>
#include <cassert>
#include <optional>

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
  int x;
  int y;
  int dx;
  int dy;
};

using Cell = std::variant<std::monostate, Wall, Rock, Player, Enemy>;

struct Player player = {5, 5, 30, 5};
struct Enemy enemies[] = {
    {12, 5, 'A', 10, 3},
    {10, 10, 'B', 10, 3},
    {14, 14, 'C', 10, 3},
    {17, 17, 'X', 10, 3},
};
std::vector<Rock> rocks;
std::vector<std::pair<int, int>> old_positions;

// 岩を座標から取得
std::optional<std::reference_wrapper<Rock>> get_rock(int x, int y)
{
  for (Rock &rock : rocks)
  {
    if (rock.x == x && rock.y == y)
    {
      return rock;
    }
  }
  return std::nullopt;
}

// フィールド

std::vector<Cell> field;

int field_width = 0;
int field_height = 0;

// 文字列の配列で画面への描画をバッファするためのデータを定義する
// 名前は screen_buffer とする
// 画面のサイズは field_width x field_height とする
std::vector<std::string> screen_buffer;

// マップデータ
// '.': 通路 (Floor)
// '#': 壁 (Wall)
// '*': 岩 (Rock)

const std::string map[] = {
    // 345678901234567890
    "####################", // 1
    "#..................#", // 2
    "#..................#", // 3
    "#......*...........#", // 4
    "#......*...........#", // 5
    "#......*...........#", // 6
    "#......*...........#", // 7
    "#..................#", // 8
    "#..................#", // 9
    "#....*******.......#", // 10
    "#..................#", // 11
    "#..................#", // 12
    "#..................#", // 13
    "#..................#", // 14
    "#..................#", // 15
    "#..................#", // 16
    "#..................#", // 17
    "#..................#", // 18
    "#..................#", // 19
    "####################", // 20
};

// フィールドを初期化
void init_field(int width, int height)
{
  field_width = width;
  field_height = height;
  field.resize(height * width, std::monostate{});
  screen_buffer.resize(height, std::string(width, ' '));
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
    return '.';
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
    cell = std::monostate{};
    break;
  case '@':
    cell = player;
    break;
  default:
    cell = std::monostate{};
    break;
  }
}

int field_index(int x, int y)
{
  return y * field_width + x;
}

// フィールドにオブジェクトを配置
void put_object_to_field(int x, int y, Cell cell)
{
  if (y >= 0 && y < field_height && x >= 0 && x < field_width)
  {
    field[field_index(x, y)] = cell;
  }
}

void remove_object_from_field(int x, int y)
{
  put_object_to_field(x, y, std::monostate{});
}

bool is_in_screen(int x, int y)
{
  return x >= 0 && x < field_width && y >= 0 && y < field_height;
}

// フィールドの指定位置の文字を取得
char get_field_symbol(int x, int y)
{
  if (is_in_screen(x, y))
  {
    Cell cell = field[field_index(x, y)];
    return cell_to_char(cell);
  }
  return ' ';
}

// フィールドを端末に出力
void render_screen(int x, int y)
{
  cui_gotoxy(x, y);
  for (int j = 0; j < field_height; j++)
  {
    for (int i = 0; i < field_width; i++)
    {
      screen_buffer[j][i] = get_field_symbol(i, j);
    }
    std::cout << screen_buffer[j] << std::endl;
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

// マップデータからフィールドを初期化する
void init_map()
{
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      char c = map[y][x];
      Cell cell;
      char_to_cell(c, cell);
      if (std::holds_alternative<Rock>(cell))
      {
        Rock rock;
        rock.x = x;
        rock.y = y;
        rock.dx = 0;
        rock.dy = 0;
        rocks.push_back(rock);
        old_positions.push_back({x, y});
      }
      else
      {
        put_object_to_field(x, y, cell);
      }
    }
  }
}

void draw_info(void)
{
  // cui_gotoxy(1, 1);
  // std::cout << "ch = " << ch;
  cui_gotoxy(1, 25);
  for (Rock &rock : rocks)
  {
    std::cout << "Rock:(x=" << rock.x << ",y=" << rock.y << ",dx=" << rock.dx << ",dy=" << rock.dy << ")\n";
  }
}

// なにかに衝突したら true を返す
bool check_collision(int x, int y)
{
  char c = get_field_symbol(x, y);
  if (c == '.')
  {
    return false;
  }
  else if (c == '*')
  {
    // kick rock
    int dx = x - player.x;
    int dy = y - player.y;
    int x2 = x + dx;
    int y2 = y + dy;
    if (is_in_screen(x2, y2))
    {
      auto rock_opt = get_rock(x, y);
      if (rock_opt)
      {
        Rock &rock = rock_opt.value().get();
        remove_object_from_field(x, y);
        rock.dx = dx;
        rock.dy = dy;
        rock.x = x2;
        rock.y = y2;
        put_object_to_field(x2, y2, rock);
        cui_gotoxy(25, 3);
        std::cout << "Rock kicked!";
        cui_gotoxy(25, 4);
        std::cout << "Rock:(x=" << rock.x << ",y=" << rock.y << ",dx=" << rock.dx << ",dy=" << rock.dy << ")";
        return true;
      }
    }
  }
  return true;
}

void move(char input)
{
  int x = player.x;
  int y = player.y;
  if (ch == 'h' && x > 1)
  {
    x--;
  }
  else if (ch == 'j' && y < MAP_HEIGHT)
  {
    y++;
  }
  else if (ch == 'k' && y > 1)
  {
    y--;
  }
  else if (ch == 'l' && x < MAP_WIDTH)
  {
    x++;
  }
  if (!check_collision(x, y))
  {
    player.x = x;
    player.y = y;
  }
}

void update_objects()
{
  // 古い位置のオブジェクトを削除
  for (const auto &pos : old_positions)
  {
    remove_object_from_field(pos.first, pos.second);
  }
  old_positions.clear();

  put_object_to_field(player.x, player.y, player);
  old_positions.push_back({player.x, player.y});
  for (auto &enemy : enemies)
  {
    put_object_to_field(enemy.x, enemy.y, enemy);
    old_positions.push_back({enemy.x, enemy.y});
  }

  // 移動オブジェクトの位置を更新
  for (auto &rock : rocks)
  {
    // 位置を更新するロジック
    int new_x = rock.x + rock.dx;
    int new_y = rock.y + rock.dy;

    // 端で止まるようにする
    if (new_x < 0 || new_x >= MAP_WIDTH)
    {
      rock.dx = 0;
    }
    else
    {
      rock.x = new_x;
      updated();
    }

    if (new_y < 0 || new_y >= MAP_HEIGHT)
    {
      rock.dy = 0;
    }
    else
    {
      rock.y = new_y;
      updated();
    }

    put_object_to_field(rock.x, rock.y, rock);

    // 新しい位置を保存
    old_positions.push_back({rock.x, rock.y});
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
  if (ch == 'q')
  {
    cui_cursor_on();
    exit(0);
  }
  update_objects();
}

void draw(void)
{
  if (!is_updated())
    return;

  render_screen(1, 3);
  draw_info();
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
  init_field(MAP_WIDTH + 2, MAP_HEIGHT + 2);
  init_map();
  updated();

  while (true)
  {
    update();
    draw();
    nanosleep(&req, NULL); // 1/30秒スリープ
  }
}
