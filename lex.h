#ifndef LEX_H
#define LEX_H

void init_lex(char *);

const char *curr_string();
int curr_pos();

char curr_char();
char next_char();

void bump_char();

#endif /* LEX_H */
