#include "cui.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <cassert>
#include <optional>
#include <ctime>
#include <unordered_map>

#define MAP_WIDTH 20
#define MAP_HEIGHT 20

char ch = ' ';
bool upd = false;
int frame = 0;
int gameStartFrame = 0;
bool is_state_changed = false;

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
GameState next_game_state = GameState::Title;

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

struct Door
{
};

struct UpStairs
{
};

struct DownStairs
{
};

struct Rock
{
  int x;
  int y;
  int dx;
  int dy;
};

using Cell = std::variant<std::monostate, Wall, Door, UpStairs, DownStairs, Rock, Player, Enemy>;

struct Player player = {5, 5, 30, 5};
std::vector<Enemy> enemies;
std::vector<Rock> rocks;
std::vector<std::pair<int, int>> old_positions;

void quit();

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
// '+': 扉 (Door)
// '<': 上り階段(UpStairs)
// '>': 下り階段(DownStairs)

const std::string map[] = {
    // 345678901234567890
    "####################", // 1
    "#...#..*....#....>.#", // 2
    "#.<.#..*....#......#", // 3
    "#.@.#..*....#.C....#", // 4
    "#...#..*....#......#", // 5
    "#...#..*....######+#", // 6
    "#...#..*......#....#", // 7
    "#...#..*......#....#", // 8
    "#***#..*......#+####", // 9
    "#..................#", // 10
    "#..#***#......#....#", // 11
    "#..#...#......#....#", // 12
    "#..#...#******#....#", // 13
    "#..#...#.A....#....#", // 14
    "#..#...#......#..B.#", // 15
    "#..#...#......+....#", // 16
    "#..#...#......#....#", // 17
    "#..#...########....#", // 18
    "#.X#...............#", // 19
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
  case 'A':
  case 'B':
  case 'C':
  case 'X':
    cell = Enemy{0, 0, c, 0, 0};
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
  if (!is_in_screen(x, y))
  {
    return ' ';
  }
  // Playerがいる場所はPlayerの文字を返す
  if (player.x == x && player.y == y)
  {
    return '@';
  }
  // Enemyがいる場所はEnemyの文字を返す
  for (const auto &enemy : enemies)
  {
    if (enemy.x == x && enemy.y == y)
    {
      return enemy.c;
    }
  }

    Cell cell = field[field_index(x, y)];
    return cell_to_char(cell);
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
  // プレイヤーを表示
  screen_buffer[player.y][player.x] = '@';
  // 敵を表示
  for (const auto &enemy : enemies)
  {
    screen_buffer[enemy.y][enemy.x] = enemy.c;
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
      else if (std::holds_alternative<Player>(cell))
      {
        player.x = x;
        player.y = y;
        old_positions.push_back({x, y});
        put_object_to_field(x, y, std::monostate{});
      }
      else if (std::holds_alternative<Enemy>(cell))
      {
        Enemy enemy;
        enemy.x = x;
        enemy.y = y;
        enemy.c = c;
        enemy.hp = 10;
        enemy.attack = 3;
        enemies.push_back(enemy);
        old_positions.push_back({x, y});
        put_object_to_field(x, y, std::monostate{});
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
  cui_gotoxy(0, 25);
  // 敵の情報を表示
  for (const auto &enemy : enemies)
  {
    std::cout << "Enemy: " << enemy.c << " HP: " << enemy.hp << std::endl;
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
    char c = get_field_symbol(x2, y2);
    if (is_in_screen(x2, y2) && c == '.')
    {
      auto rock_opt = get_rock(x, y);
      if (rock_opt.has_value())
      {
        Rock &rock = rock_opt.value();
        rock.dx = dx;
        rock.dy = dy;
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

void update_rock_positions()
{
  for (auto &rock : rocks)
  {
    // 位置を更新するロジック
    int new_x = rock.x + rock.dx;
    int new_y = rock.y + rock.dy;
    // 端に達した場合は移動を止める
    if (new_x < 0 || new_x >= MAP_WIDTH)
    {
      rock.dx = 0;
    }
    if (new_y < 0 || new_y >= MAP_HEIGHT)
    {
      rock.dy = 0;
    }
    // 衝突判定 壁にぶつかったら移動を止める
    char c = get_field_symbol(new_x, new_y);
    if (c == '#')
    {
      rock.dx = 0;
      rock.dy = 0;
    }
    auto rock_opt = get_rock(new_x, new_y);
    if (rock_opt)
    {
      rock.dx = 0;
      rock.dy = 0;
    }

    // 移動判定
    if (rock.dx != 0 || rock.dy != 0)
    {
      rock.x = new_x;
      rock.y = new_y;
      updated();
    }
    put_object_to_field(rock.x, rock.y, rock);

    // 新しい位置を保存
    old_positions.push_back({rock.x, rock.y});
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

  old_positions.push_back({player.x, player.y});
  for (auto &enemy : enemies)
  {
    old_positions.push_back({enemy.x, enemy.y});
  }

  update_rock_positions();
}

void update_title(void)
{
  int key = cui_getch_nowait();
  if (key != -1)
  {
    ch = key;
    if (ch == ' ')
    {
      next_game_state = GameState::GameStart;
      gameStartFrame = frame;
    }
    else if (ch == 'q')
    {
      quit();
    }
  }
  updated();
}

void update_gamestart(void)
{
  if (frame - gameStartFrame > 19)
  {
    next_game_state = GameState::Playing;
    gameStartFrame = 0;
  }
  updated();
}

void update_gameover(void)
{
  int key = cui_getch_nowait();
  if (key != -1)
  {
    ch = key;
    if (ch == ' ')
    {
      next_game_state = GameState::Title;
      updated();
    }
    else if (ch == 'q')
    {
      quit();
    }
  }
}

void update_stageclear(void)
{
  int key = cui_getch_nowait();
  if (key != -1)
  {
    ch = key;
    if (ch == ' ')
    {
      next_game_state = GameState::Title;
      updated();
    }
    else if (ch == 'q')
    {
      quit();
    }
  }
}

void update_playing(void)
{
  int key = cui_getch_nowait();
  if (key != -1 || is_state_changed)
  {
    ch = key;
    move(ch);
    updated();
  }
  if (ch == 'q')
  {
    quit();
  }
  update_objects();
}

using UpdateFunc = void (*)();
UpdateFunc update_funcs[] = {
    update_title,      // GameState::Title
    update_gamestart,  // GameState::GameStart
    update_playing,    // GameState::Playing
    update_gameover,   // GameState::GameOver
    update_stageclear, // GameState::StageClear
};

void update(void)
{
  UpdateFunc update_func = update_funcs[static_cast<int>(game_state)];
  if (update_func)
  {
    update_func();
  }
}

// 関数宣言
void draw_title();
void draw_gamestart();
void draw_playing();
void draw_gameover();
void draw_stageclear();

// 関数ポインタの型を定義
using DrawFunc = void (*)();

// 関数ポインタの配列を定義
DrawFunc draw_funcs[] = {
    draw_title,      // GameState::Title
    draw_gamestart,  // GameState::GameStart
    draw_playing,    // GameState::Playing
    draw_gameover,   // GameState::GameOver
    draw_stageclear  // GameState::StageClear
};

void draw(void)
{
  // 関数ポインタ draw_func によって描画関数を切り替える
  DrawFunc draw_func = draw_funcs[static_cast<int>(game_state)];

  if (draw_func)
  {
    draw_func();
  }
}

void draw_title(void)
{
  if (!is_updated())
    return;

  cui_gotoxy(5, 10);
  std::cout << "M A P E R   G A M E";
  drawn();
}

void draw_gamestart(void)
{
  cui_gotoxy(5, 10);
  std::cout << "Game Start";

  if (!is_updated())
    return;

  int count = frame - gameStartFrame;
  if (count < 19)
  {
    cui_gotoxy(5, 10);
    for (int i = 0; i < count; i++)
    {
      std::cout << "*";
    }
  }
  else
  {
    cui_clear_screen();
  }
  drawn();
}

void draw_gameover(void)
{
  if (!is_updated())
    return;

  cui_gotoxy(5, 10);
  std::cout << "Game Over" << std::endl;
  drawn();
}

void draw_stageclear(void)
{
  if (!is_updated())
    return;

  cui_gotoxy(5, 10);
  std::cout << "Stage Clear" << std::endl;
  drawn();
}

void draw_playing(void)
{
  if (!is_updated())
    return;

  render_screen(1, 3);
  draw_info();
  drawn();
}

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

void quit()
{
  cui_cursor_on();
  exit(0);
}

int main()
{
  const long CYCLE = 33333333L; // 1/30秒 (ナノ秒単位)
  std::timespec ts1, ts2 = {0};
  frame = 0;

  cui_clear_screen();
  cui_cursor_off();
  init_field(MAP_WIDTH, MAP_HEIGHT);
  init_map();
  updated();

  while (true)
  {
    timespec_get(&ts1, TIME_UTC);
    update();
    draw();
    timespec_get(&ts2, TIME_UTC);
    wait(ts1, ts2, CYCLE);
    frame++;
    if (frame < 0)
    {
      // オーバーフロー時はゼロに戻す
      frame = 0;
    }
    // ゲームの状態が変わったらフラグを立てる
    if (game_state != next_game_state)
    {
      game_state = next_game_state;
      is_state_changed = true;
    }
    else
    {
      is_state_changed = false;
    }
  }
}
