#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void obtine_cale_executabil(char *buffer, size_t dim_buffer);
void obtine_cale_date_studenti(char *buffer, size_t dim_buffer);
void construieste_email(const char *prenume, const char *nume, char *email, size_t dim_email);
int egal_ignore_case(const char *a, const char *b);

#endif
