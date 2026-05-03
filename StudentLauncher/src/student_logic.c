#include "student_logic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "student_io.h"
#include "utils.h"

#define printf(...) do { fprintf(stdout, __VA_ARGS__); fflush(stdout); } while (0)

void initializare_situatie(SituatieStudent *s) {
    s->semestre = 0;
    s->discipline = 0;
    s->note = NULL;
    s->credite = NULL;
    s->medii_ponderate = NULL;
    s->credite_semestru = NULL;
    s->nr_discipline_peste_4_credite = 0;
    s->nr_restante_absente = 0;
    s->nr_semestre_scadere = 0;
    s->semestre_scadere = NULL;
}

void elibereaza_situatie(SituatieStudent *s) {
    int i;

    if (s->note != NULL) {
        for (i = 0; i < s->semestre; i++) {
            free(s->note[i]);
        }
        free(s->note);
    }

    if (s->credite != NULL) {
        for (i = 0; i < s->semestre; i++) {
            free(s->credite[i]);
        }
        free(s->credite);
    }

    free(s->medii_ponderate);
    free(s->credite_semestru);
    free(s->semestre_scadere);

    initializare_situatie(s);
}

void elibereaza_toata_memoria(StudentStorage *storage) {
    int i;

    for (i = 0; i < storage->nr_studenti; i++) {
        elibereaza_situatie(&storage->studenti[i].situatie);
    }

    free(storage->studenti);
    storage->studenti = NULL;
    storage->nr_studenti = 0;
    storage->capacitate_studenti = 0;
}

int asigura_capacitate_studenti(StudentStorage *storage) {
    DateStudent *nou_vector;
    int capacitate_noua;

    if (storage->nr_studenti < storage->capacitate_studenti) {
        return 1;
    }

    capacitate_noua = (storage->capacitate_studenti == 0) ? 4 : storage->capacitate_studenti * 2;
    nou_vector = (DateStudent *)realloc(storage->studenti, (size_t)capacitate_noua * sizeof(DateStudent));

    if (nou_vector == NULL) {
        printf("Eroare: nu s-a putut realoca memorie pentru studenti.\n");
        return 0;
    }

    storage->studenti = nou_vector;
    storage->capacitate_studenti = capacitate_noua;
    return 1;
}

DateStudent *cauta_student_dupa_cod(StudentStorage *storage, int cod) {
    int i;

    for (i = 0; i < storage->nr_studenti; i++) {
        if (storage->studenti[i].cod_personal == cod) {
            return &storage->studenti[i];
        }
    }
    return NULL;
}

void calculeaza_indicatori_scolari(SituatieStudent *s) {
    int sem;
    int d;
    int suma_credite;
    int suma_produs;
    int nota;
    int credit;

    if (s->note == NULL || s->credite == NULL || s->semestre <= 0 || s->discipline <= 0) {
        return;
    }

    s->nr_discipline_peste_4_credite = 0;
    s->nr_restante_absente = 0;
    s->nr_semestre_scadere = 0;

    free(s->medii_ponderate);
    free(s->credite_semestru);
    free(s->semestre_scadere);

    s->medii_ponderate = (double *)calloc((size_t)s->semestre, sizeof(double));
    s->credite_semestru = (int *)calloc((size_t)s->semestre, sizeof(int));
    s->semestre_scadere = (int *)calloc((size_t)s->semestre, sizeof(int));

    if (s->medii_ponderate == NULL || s->credite_semestru == NULL || s->semestre_scadere == NULL) {
        printf("Eroare: memorie insuficienta la calculul indicatorilor.\n");
        return;
    }

    for (sem = 0; sem < s->semestre; sem++) {
        suma_credite = 0;
        suma_produs = 0;

        for (d = 0; d < s->discipline; d++) {
            nota = s->note[sem][d];
            credit = s->credite[sem][d];

            if (nota == -1 || credit == -1) {
                continue;
            }
            if (credit > 4) {
                s->nr_discipline_peste_4_credite++;
            }
            if (nota >= 0 && nota < 5) {
                s->nr_restante_absente++;
            }

            suma_credite += credit;
            suma_produs += nota * credit;
        }

        s->credite_semestru[sem] = suma_credite;
        s->medii_ponderate[sem] = (suma_credite > 0) ? (double)suma_produs / (double)suma_credite : 0.0;

        if (sem > 0 && s->medii_ponderate[sem] < s->medii_ponderate[sem - 1]) {
            s->semestre_scadere[s->nr_semestre_scadere++] = sem + 1;
        }
    }
}

double media_generala(const DateStudent *student) {
    int sem;
    double suma = 0.0;
    int count = 0;
    const SituatieStudent *s = &student->situatie;

    if (s->medii_ponderate == NULL || s->semestre == 0) {
        return 0.0;
    }

    for (sem = 0; sem < s->semestre; sem++) {
        if (s->credite_semestru != NULL && s->credite_semestru[sem] > 0) {
            suma += s->medii_ponderate[sem];
            count++;
        }
    }

    return (count > 0) ? (suma / (double)count) : 0.0;
}

static void alege_an_si_grupa(int *an_studiu, int *grupa) {
    int an;
    int g;

    do {
        printf("Alege anul de studiu (1-4): ");
        scanf("%d", &an);
    } while (an < 1 || an > 4);

    do {
        printf("Alege grupa (1-15): ");
        scanf("%d", &g);
    } while (g < 1 || g > 15);

    *an_studiu = an;
    *grupa = g;
}

void adaugare_studenti(StudentStorage *storage) {
    int continua = 1;
    int incepe;
    DateStudent *student;

    while (continua == 1) {
        printf("\n=== Adaugare student nou ===\n");
        printf("Continui adaugarea? (1=Da, 0=Cancel): ");
        scanf("%d", &incepe);

        if (incepe == 0) {
            printf("Adaugarea a fost anulata.\n");
            break;
        }

        if (incepe != 1) {
            printf("Optiune invalida. Incearca din nou.\n");
            continue;
        }

        if (!asigura_capacitate_studenti(storage)) {
            return;
        }

        student = &storage->studenti[storage->nr_studenti];
        initializare_situatie(&student->situatie);

        printf("Nume (0=Cancel): ");
        scanf("%49s", student->nume);
        if (strcmp(student->nume, "0") == 0) {
            printf("Adaugarea a fost anulata.\n");
            break;
        }

        printf("Prenume (0=Cancel): ");
        scanf("%49s", student->prenume);
        if (strcmp(student->prenume, "0") == 0) {
            printf("Adaugarea a fost anulata.\n");
            break;
        }

        printf("Varsta (0=Cancel): ");
        scanf("%d", &student->varsta);
        if (student->varsta == 0) {
            printf("Adaugarea a fost anulata.\n");
            break;
        }

        student->cod_personal = storage->urmator_cod_personal++;
        construieste_email(student->prenume, student->nume, student->email, sizeof(student->email));

        alege_an_si_grupa(&student->an_studiu, &student->grupa);
        scrie_fisa_student(student);

        storage->nr_studenti++;

        printf("Student adaugat. Cod personal: %d\n", student->cod_personal);
        printf("Email: %s\n", student->email);

        printf("Mai adaugi un student? (1=Da, 0=Nu): ");
        scanf("%d", &continua);
    }
}

void afisare_studenti(const StudentStorage *storage) {
    int i;

    if (storage->nr_studenti == 0) {
        printf("Nu exista studenti inregistrati.\n");
        return;
    }

    printf("\n=== Lista studenti ===\n");
    for (i = 0; i < storage->nr_studenti; i++) {
        printf(
            "Cod %d | %s %s | An %d Grupa %d\n",
            storage->studenti[i].cod_personal,
            storage->studenti[i].nume,
            storage->studenti[i].prenume,
            storage->studenti[i].an_studiu,
            storage->studenti[i].grupa
        );
    }
}

void cauta_student_dupa_nume_prenume(const StudentStorage *storage) {
    char nume_cautat[50];
    char prenume_cautat[50];
    int i;
    int gasit = 0;

    if (storage->nr_studenti == 0) {
        printf("Nu exista studenti inregistrati.\n");
        return;
    }

    printf("Introdu numele: ");
    scanf("%49s", nume_cautat);
    printf("Introdu prenumele: ");
    scanf("%49s", prenume_cautat);

    printf("\nRezultate cautare:\n");
    for (i = 0; i < storage->nr_studenti; i++) {
        if (egal_ignore_case(storage->studenti[i].nume, nume_cautat) &&
            egal_ignore_case(storage->studenti[i].prenume, prenume_cautat)) {
            printf(
                "Cod %d | %s %s | An %d Grupa %d | Email %s\n",
                storage->studenti[i].cod_personal,
                storage->studenti[i].nume,
                storage->studenti[i].prenume,
                storage->studenti[i].an_studiu,
                storage->studenti[i].grupa,
                storage->studenti[i].email
            );
            gasit = 1;
        }
    }

    if (!gasit) {
        printf("Nu exista student cu acest nume si prenume.\n");
    }
}

void introdu_situatie_pentru_student(StudentStorage *storage) {
    int cod;
    int sem;
    int d;
    int nota;
    int credit;
    DateStudent *student;
    SituatieStudent *s;

    if (storage->nr_studenti == 0) {
        printf("Nu exista studenti.\n");
        return;
    }

    printf("Introdu codul personal al studentului: ");
    scanf("%d", &cod);

    student = cauta_student_dupa_cod(storage, cod);
    if (student == NULL) {
        printf("Student inexistent.\n");
        return;
    }

    s = &student->situatie;
    elibereaza_situatie(s);

    printf("Numar semestre incheiate: ");
    scanf("%d", &s->semestre);
    printf("Numar discipline pe semestru: ");
    scanf("%d", &s->discipline);

    if (s->semestre <= 0 || s->discipline <= 0) {
        printf("Date invalide.\n");
        initializare_situatie(s);
        return;
    }

    s->note = (int **)malloc((size_t)s->semestre * sizeof(int *));
    s->credite = (int **)malloc((size_t)s->semestre * sizeof(int *));
    if (s->note == NULL || s->credite == NULL) {
        printf("Eroare la alocare.\n");
        elibereaza_situatie(s);
        return;
    }

    for (sem = 0; sem < s->semestre; sem++) {
        s->note[sem] = (int *)malloc((size_t)s->discipline * sizeof(int));
        s->credite[sem] = (int *)malloc((size_t)s->discipline * sizeof(int));
        if (s->note[sem] == NULL || s->credite[sem] == NULL) {
            printf("Eroare la alocare pe linii.\n");
            elibereaza_situatie(s);
            return;
        }
    }

    printf("Pentru locurile libere introdu -1 la nota.\n");
    for (sem = 0; sem < s->semestre; sem++) {
        printf("\nSemestrul %d\n", sem + 1);
        for (d = 0; d < s->discipline; d++) {
            do {
                printf("Nota disciplina %d (0..10 sau -1): ", d + 1);
                scanf("%d", &nota);
            } while (!((nota >= 0 && nota <= 10) || nota == -1));

            if (nota == -1) {
                s->note[sem][d] = -1;
                s->credite[sem][d] = -1;
                continue;
            }

            do {
                printf("Credite disciplina %d (1..30): ", d + 1);
                scanf("%d", &credit);
            } while (credit < 1 || credit > 30);

            s->note[sem][d] = nota;
            s->credite[sem][d] = credit;
        }
    }

    calculeaza_indicatori_scolari(s);
    scrie_fisa_student(student);
    printf("Situatia scolara a fost salvata.\n");
}

void afiseaza_raport_student(StudentStorage *storage) {
    int cod;
    int sem;
    int d;
    int restante_sem;
    int idx_restanta;
    int suma_credite;
    int suma_produs;
    int credit_rest;
    int punctaj_rest;
    int credite_fara;
    double medie_fara_restanta;
    DateStudent *student;
    SituatieStudent *s;
    int are_date_detaliate;

    if (storage->nr_studenti == 0) {
        printf("Nu exista studenti.\n");
        return;
    }

    printf("Introdu codul personal: ");
    scanf("%d", &cod);

    student = cauta_student_dupa_cod(storage, cod);
    if (student == NULL) {
        printf("Student inexistent.\n");
        return;
    }

    s = &student->situatie;
    if (s->semestre == 0 || s->medii_ponderate == NULL || s->credite_semestru == NULL) {
        printf("Studentul nu are situatie scolara.\n");
        return;
    }

    are_date_detaliate = (s->discipline > 0 && s->note != NULL && s->credite != NULL);
    if (are_date_detaliate) {
        calculeaza_indicatori_scolari(s);
    }

    printf("\n=== Raport ===\n");
    printf("Student: %s %s (cod %d)\n", student->nume, student->prenume, student->cod_personal);
    for (sem = 0; sem < s->semestre; sem++) {
        printf(
            "Semestrul %d -> Credite: %d | Medie ponderata: %.2f\n",
            sem + 1,
            s->credite_semestru[sem],
            s->medii_ponderate[sem]
        );
    }

    printf("Discipline cu peste 4 credite: %d\n", s->nr_discipline_peste_4_credite);
    printf("Discipline picate sau absente: %d\n", s->nr_restante_absente);

    if (s->nr_semestre_scadere == 0) {
        printf("Nu exista scaderi intre semestre.\n");
    } else {
        printf("Semestre cu scadere: ");
        for (sem = 0; sem < s->nr_semestre_scadere; sem++) {
            printf("%d ", s->semestre_scadere[sem]);
        }
        printf("\n");
    }

    if (!are_date_detaliate) {
        printf("Detaliul restantelor apare doar daca ai note+credite pe discipline.\n");
        return;
    }

    for (sem = 0; sem < s->semestre; sem++) {
        restante_sem = 0;
        idx_restanta = -1;
        suma_credite = 0;
        suma_produs = 0;

        for (d = 0; d < s->discipline; d++) {
            if (s->note[sem][d] == -1 || s->credite[sem][d] == -1) {
                continue;
            }

            suma_credite += s->credite[sem][d];
            suma_produs += s->note[sem][d] * s->credite[sem][d];

            if (s->note[sem][d] >= 0 && s->note[sem][d] < 5) {
                restante_sem++;
                idx_restanta = d;
            }
        }

        if (restante_sem == 1 && idx_restanta >= 0) {
            credit_rest = s->credite[sem][idx_restanta];
            punctaj_rest = s->note[sem][idx_restanta] * credit_rest;
            credite_fara = suma_credite - credit_rest;
            medie_fara_restanta = (credite_fara > 0) ? (double)(suma_produs - punctaj_rest) / (double)credite_fara : 0.0;

            printf(
                "Semestrul %d are exact o restanta. Medie curenta %.2f, medie fara disciplina %.2f\n",
                sem + 1,
                s->medii_ponderate[sem],
                medie_fara_restanta
            );
        }
    }
}

void afiseaza_clasament_dupa_medie(StudentStorage *storage) {
    int i;
    int j;
    int *selectat;
    int best;
    double best_medie;
    double mg;

    if (storage->nr_studenti == 0) {
        printf("Nu exista studenti pentru clasament.\n");
        return;
    }

    for (i = 0; i < storage->nr_studenti; i++) {
        if (storage->studenti[i].situatie.semestre > 0 &&
            storage->studenti[i].situatie.note != NULL &&
            storage->studenti[i].situatie.credite != NULL) {
            calculeaza_indicatori_scolari(&storage->studenti[i].situatie);
        }
    }

    selectat = (int *)calloc((size_t)storage->nr_studenti, sizeof(int));
    if (selectat == NULL) {
        printf("Eroare: memorie insuficienta pentru clasament.\n");
        return;
    }

    printf("\n=== Clasament studenti dupa media generala ===\n");
    for (i = 0; i < storage->nr_studenti; i++) {
        best = -1;
        best_medie = -1.0;

        for (j = 0; j < storage->nr_studenti; j++) {
            if (selectat[j]) {
                continue;
            }

            mg = media_generala(&storage->studenti[j]);
            if (mg > best_medie) {
                best_medie = mg;
                best = j;
            }
        }

        if (best != -1) {
            printf(
                "%d. Cod %d | %s %s | Media generala %.2f\n",
                i + 1,
                storage->studenti[best].cod_personal,
                storage->studenti[best].nume,
                storage->studenti[best].prenume,
                best_medie
            );
            selectat[best] = 1;
        }
    }

    free(selectat);
}
