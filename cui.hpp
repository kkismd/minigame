#ifndef CUI_HPP
#define CUI_HPP

char cui_getch();
int cui_getch_nowait();
void cui_clear_screen();
void cui_gotoxy(int x, int y);
void cui_cursor_off();
void cui_cursor_on();

#endif