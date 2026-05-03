#ifndef STUDENT_IO_H
#define STUDENT_IO_H

#include "student_types.h"

int creeaza_director_daca_lipseste(const char *cale);
void scrie_fisa_student(DateStudent *student);
int incarca_un_student_din_fisier(const char *cale_fisier, StudentStorage *storage);
int incarca_studenti_din_fisiere(StudentStorage *storage);
void sterge_student(StudentStorage *storage);

#endif
