#include "cui.hpp"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>

const std::string ESC_SEQ = "\033[";

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
  std::cout << ESC_SEQ << "H" << ESC_SEQ << "J";
}

void cui_gotoxy(int x, int y)
{
  std::cout << ESC_SEQ << y << ";" << x << "H";
}

void cui_cursor_off()
{
  std::cout << ESC_SEQ << "?25l";
}

void cui_cursor_on()
{
  std::cout << ESC_SEQ << "?25h";
}

void cui_clear_line()
{
  std::cout << ESC_SEQ << "2K";
}

void cui_attribute(int attr)
{
  std::cout << ESC_SEQ << attr << "m";
}

void cui_attr_reverse()
{
  cui_attribute(7);
}

void cui_attr_normal()
{
  cui_attribute(0);
}

void cui_color_red()
{
  cui_attribute(31);
}

void cui_color_green()
{
  cui_attribute(32);
}

void cui_color_yellow()
{
  cui_attribute(33);
}

void cui_scroll_window_full()
{
  std::cout << ESC_SEQ << "r";
}

void cui_scroll_window(int start, int end)
{
  std::cout << ESC_SEQ << start << ";" << end << "r";
}

void cui_scroll_up()
{
  std::cout << "\033D";
}

void cui_scroll_down()
{
  std::cout << "\033M";
}
