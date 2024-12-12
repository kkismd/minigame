#include "cui.hpp"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

// global variable to store terminal settings
static struct termios oldt;
static int oldf;

// goto non-canonical mode
void change_non_canonical()
{
  // backup current terminal settings
  tcgetattr(STDIN_FILENO, &oldt);
  struct termios newt = oldt;
  // set terminal settings to non-canonical mode
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

// restore terminal settings
void restore_terminal_settings()
{
  // restore terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

// set terminal to non-blocking mode
void change_non_block()
{
  // backup current file status flags
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  // set terminal to non-blocking mode
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
}

// restore terminal file status flags
void restore_fcntl()
{
  // restore file status flags
  fcntl(STDIN_FILENO, F_SETFL, oldf);
}

// input single character
// no need to press enter
// no echo back
char cui_getch()
{
  change_non_canonical();
  char ch = getchar();
  restore_terminal_settings();
  return ch;
}

// input single character
// non blocking
// no echo back
int cui_getch_nowait()
{
  int ch;

  change_non_canonical();
  change_non_block();

  ch = getchar();

  restore_terminal_settings();
  restore_fcntl();

  return ch; // Return the character read, or EOF if no input is available
}

void cui_clear_screen()
{
  printf("\033[H\033[J");
}

void cui_gotoxy(int x, int y)
{
  printf("\033[%d;%dH", y, x);
}

void cui_cursor_off()
{
  printf("\033[?25l");
}

void cui_cursor_on()
{
  printf("\033[?25h");
}
