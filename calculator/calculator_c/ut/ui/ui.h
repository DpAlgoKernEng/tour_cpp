#ifndef UI_H
#define UI_H

#include <stddef.h>

#define INPUT_OK     0
#define INPUT_EOF    1
#define INPUT_TRUNC  2
#define INPUT_ERROR  3

void show_welcome(void);
void show_help(void);
int get_user_input(char *buffer, size_t size);
char *get_user_input_dynamic(void);
void show_result(double result);
void show_error(const char *error);

#endif