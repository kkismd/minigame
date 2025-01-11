#ifndef CUI_HPP
#define CUI_HPP

char cui_getch();
int cui_getch_nowait();
void cui_out_non_buffered();
void cui_out_buffered();
void cui_clear_screen();
void cui_gotoxy(int x, int y);
void cui_cursor_off();
void cui_cursor_on();
void cui_clear_line();
void cui_attribute(int attr);
void cui_attr_reverse();
void cui_attr_normal();
void cui_color(int fg, int bg);
void cui_color_red();
void cui_color_green();
void cui_color_yellow();
void cui_scroll_window_full();
void cui_scroll_window(int start, int end);
void cup_scroll_up();
void cup_scroll_down();

#endif