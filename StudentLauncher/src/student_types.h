#ifndef STUDENT_TYPES_H
#define STUDENT_TYPES_H

#define MAX_SEM_FIS 40
#define MAX_DISC_FIS 80

typedef struct {
    int semestre;
    int discipline;
    int **note;
    int **credite;
    double *medii_ponderate;
    int *credite_semestru;
    int nr_discipline_peste_4_credite;
    int nr_restante_absente;
    int nr_semestre_scadere;
    int *semestre_scadere;
} SituatieStudent;

typedef struct {
    int cod_personal;
    char nume[50];
    char prenume[50];
    int varsta;
    int an_studiu;
    int grupa;
    char email[80];
    SituatieStudent situatie;
} DateStudent;

typedef struct {
    DateStudent *studenti;
    int nr_studenti;
    int capacitate_studenti;
    int urmator_cod_personal;
} StudentStorage;

#endif
