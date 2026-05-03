#ifndef STUDENT_LOGIC_H
#define STUDENT_LOGIC_H

#include "student_types.h"

void initializare_situatie(SituatieStudent *s);
void elibereaza_situatie(SituatieStudent *s);
void elibereaza_toata_memoria(StudentStorage *storage);
int asigura_capacitate_studenti(StudentStorage *storage);
DateStudent *cauta_student_dupa_cod(StudentStorage *storage, int cod);

void calculeaza_indicatori_scolari(SituatieStudent *s);
double media_generala(const DateStudent *student);

void adaugare_studenti(StudentStorage *storage);
void afisare_studenti(const StudentStorage *storage);
void cauta_student_dupa_nume_prenume(const StudentStorage *storage);
void introdu_situatie_pentru_student(StudentStorage *storage);
void afiseaza_raport_student(StudentStorage *storage);
void afiseaza_clasament_dupa_medie(StudentStorage *storage);

#endif
