#include "cui.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <ctime>

#define MAP_WIDTH 20
#define MAP_HEIGHT 20

char ch = ' ';
bool upd = false;
int frame = 0;

// ゲームの進行状況を表すenumを定義
enum class GameState
{
  Title,
  GameStart,
  Playing,
  GameOver,
  StageClear
};

GameState game_state = GameState::Title;

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
struct Door
{
};
struct UpStairs
{
};
struct DownStairs
{
};

using Cell = std::variant<std::monostate, Wall, Rock, Floor, Door, UpStairs, DownStairs, Player, Enemy>;

struct Player player = {5, 5, 30, 5};
struct Enemy enemies[] = {
    {4, 15, 'A', 10, 3},
    {9, 4, 'B', 10, 3},
    {10, 16, 'C', 10, 3},
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
// '+': 扉 (Door)
// '<': 上り階段(UpStairs)
// '>': 下り階段(DownStairs)

const std::string map[] = {
    // 345678901234567890
    "####################", // 1
    "#......#....#....>.#", // 2
    "#......+....#......#", // 3
    "#.<....#....#......#", // 4
    "#......#....#......#", // 5
    "#......#....######+#", // 6
    "#......#......#....#", // 7
    "#......######.#....#", // 8
    "###+#########.#+####", // 9
    "#......######.#....#", // 10
    "#......+..#...#....#", // 11
    "#......##...###....#", // 12
    "#......###+####....#", // 13
    "#......#......#....#", // 14
    "#......#......#....#", // 15
    "#......#......+....#", // 16
    "#......#......#....#", // 17
    "#......#......#....#", // 18
    "#......#......#....#", // 19
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
  else if (std::holds_alternative<Door>(cell))
  {
    return '+';
  }
  else if (std::holds_alternative<UpStairs>(cell))
  {
    return '<';
  }
  else if (std::holds_alternative<DownStairs>(cell))
  {
    return '>';
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
  case '+':
    cell = Door{};
    break;
  case '<':
    cell = UpStairs{};
    break;
  case '>':
    cell = DownStairs{};
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
  char obj = get_from_buffer(x, y);
  if (obj == '.' || obj == '+')
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

bool spaceKeyPressed = false;
int gameStartFrame = 0;

void update_title(void)
{
  int key = cui_getch_nowait();
  if (key != -1)
  {
    ch = key;
    if (spaceKeyPressed != true && ch == ' ')
    {
      spaceKeyPressed = true;
      gameStartFrame = frame;
    }
  }
  if (spaceKeyPressed == true && frame - gameStartFrame > 30)
  {
    game_state = GameState::Playing;
  }
  updated();
}

void update_playing(void)
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

void update(void)
{
  if (game_state == GameState::Title)
  {
    update_title();
  }
  else if (game_state == GameState::Playing)
  {
    update_playing();
  }
}

void draw_title(void)
{
  if (!is_updated())
    return;

  if (!spaceKeyPressed)
  {
    cui_gotoxy(5, 10);
    std::cout << "M A P E R   G A M E";
  }
  else
  {
    int count = frame - gameStartFrame;
    if (count < 30)
    {
      cui_gotoxy(1, 10);
      for (int i = 0; i < count; i++)
      {
        std::cout << ".";
      }
    }
    else
    {
      cui_clear_screen();
    }
  }
  drawn();
}

void draw_playing(void)
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

void draw(void)
{
  // 関数ポインタ draw_func によって描画関数を切り替える
  void (*draw_func)() = nullptr;

  if (game_state == GameState::Title)
  {
    draw_func = draw_title;
  }
  else if (game_state == GameState::Playing)
  {
    draw_func = draw_playing;
  }

  draw_func();
}

//
void wait(std::timespec ts1, std::timespec ts2, long interval)
{
  time_t sec = ts2.tv_sec - ts1.tv_sec;
  long nsec = ts2.tv_nsec - ts1.tv_nsec;
  if (sec == 0 && nsec < interval)
  {
    std::timespec remaining_time;
    remaining_time.tv_sec = 0;
    remaining_time.tv_nsec = interval - nsec;
    nanosleep(&remaining_time, NULL);
  }
}

int main()
{
  const long CYCLE = 33333333L; // 1/30秒 (ナノ秒単位)
  std::timespec ts1, ts2 = {0};

  cui_clear_screen();
  cui_cursor_off();
  init_screen_buffer(MAP_WIDTH + 2, MAP_HEIGHT + 2);
  updated();
  frame = 0;
  spaceKeyPressed = false;

  while (true)
  {
    timespec_get(&ts1, TIME_UTC);
    update();
    draw();
    timespec_get(&ts2, TIME_UTC);
    wait(ts1, ts2, CYCLE);
    frame++;
  }
}
