#include "student_io.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#define PATH_SEP '\\'
#else
#include <dirent.h>
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0777)
#define PATH_SEP '/'
#endif

#include "student_logic.h"
#include "utils.h"

#define printf(...) do { fprintf(stdout, __VA_ARGS__); fflush(stdout); } while (0)

static int are_extensie_txt(const char *nume_fisier) {
    size_t len;
    if (nume_fisier == NULL) {
        return 0;
    }

    len = strlen(nume_fisier);
    if (len < 4) {
        return 0;
    }

    return strcmp(nume_fisier + len - 4, ".txt") == 0;
}

static void construieste_director_an(char *dest, size_t dim_dest, const char *baza, int an) {
    snprintf(dest, dim_dest, "%s%can_%d", baza, PATH_SEP, an);
}

static void construieste_director_grupa(char *dest, size_t dim_dest, const char *baza, int an, int grupa) {
    snprintf(dest, dim_dest, "%s%can_%d%cgrupa_%d", baza, PATH_SEP, an, PATH_SEP, grupa);
}

static void construieste_fisier_student(
    char *dest,
    size_t dim_dest,
    const char *baza,
    int an,
    int grupa,
    int cod,
    const char *primul,
    const char *al_doilea
) {
    snprintf(dest, dim_dest, "%s%can_%d%cgrupa_%d%c%d_%s_%s.txt", baza, PATH_SEP, an, PATH_SEP, grupa, PATH_SEP, cod, primul, al_doilea);
}

int creeaza_director_daca_lipseste(const char *cale) {
    if (MKDIR(cale) == 0) {
        return 1;
    }
    if (errno == EEXIST) {
        return 1;
    }
    return 0;
}

void scrie_fisa_student(DateStudent *student) {
    char director_baza[512];
    char director_an[512];
    char director_grupa[512];
    char fisier[700];
    FILE *f;
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
    SituatieStudent *s = &student->situatie;

    obtine_cale_date_studenti(director_baza, sizeof(director_baza));
    construieste_director_an(director_an, sizeof(director_an), director_baza, student->an_studiu);
    construieste_director_grupa(director_grupa, sizeof(director_grupa), director_baza, student->an_studiu, student->grupa);

    if (!creeaza_director_daca_lipseste(director_baza) ||
        !creeaza_director_daca_lipseste(director_an) ||
        !creeaza_director_daca_lipseste(director_grupa)) {
        printf("Avertisment: nu am putut crea directoarele pentru fisierul studentului.\n");
        return;
    }

    snprintf(
        fisier,
        sizeof(fisier),
        "%s%c%d_%s_%s.txt",
        director_grupa,
        PATH_SEP,
        student->cod_personal,
        student->nume,
        student->prenume
    );

    f = fopen(fisier, "w");
    if (f == NULL) {
        printf("Avertisment: nu am putut deschide fisierul pentru student.\n");
        return;
    }

    fprintf(f, "=== Fisa student ===\n");
    fprintf(f, "Cod personal: %d\n", student->cod_personal);
    fprintf(f, "Nume: %s\n", student->nume);
    fprintf(f, "Prenume: %s\n", student->prenume);
    fprintf(f, "Varsta: %d\n", student->varsta);
    fprintf(f, "An studiu: %d\n", student->an_studiu);
    fprintf(f, "Grupa: %d\n", student->grupa);
    fprintf(f, "Email: %s\n", student->email);

    if (s->semestre > 0 && s->discipline > 0 && s->note != NULL && s->credite != NULL) {
        calculeaza_indicatori_scolari(s);
        fprintf(f, "\n=== Situatie scolara ===\n");

        for (sem = 0; sem < s->semestre; sem++) {
            fprintf(
                f,
                "Semestrul %d -> Credite: %d | Medie ponderata: %.2f\n",
                sem + 1,
                s->credite_semestru[sem],
                s->medii_ponderate[sem]
            );
        }

        fprintf(f, "Discipline cu peste 4 credite: %d\n", s->nr_discipline_peste_4_credite);
        fprintf(f, "Discipline picate sau absente: %d\n", s->nr_restante_absente);

        if (s->nr_semestre_scadere == 0) {
            fprintf(f, "Semestre cu scadere: niciunul\n");
        } else {
            fprintf(f, "Semestre cu scadere: ");
            for (sem = 0; sem < s->nr_semestre_scadere; sem++) {
                fprintf(f, "%d ", s->semestre_scadere[sem]);
            }
            fprintf(f, "\n");
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
                medie_fara_restanta =
                    (credite_fara > 0) ? (double)(suma_produs - punctaj_rest) / (double)credite_fara : 0.0;

                fprintf(
                    f,
                    "Semestrul %d are exact o restanta/absenta. Medie curenta %.2f, medie fara disciplina %.2f\n",
                    sem + 1,
                    s->medii_ponderate[sem],
                    medie_fara_restanta
                );
            }
        }

        fprintf(f, "\n=== Detalii discipline ===\n");
        for (sem = 0; sem < s->semestre; sem++) {
            for (d = 0; d < s->discipline; d++) {
                fprintf(
                    f,
                    "Semestrul %d, Disciplina %d: Nota=%d Credite=%d\n",
                    sem + 1,
                    d + 1,
                    s->note[sem][d],
                    s->credite[sem][d]
                );
            }
        }
    }

    fclose(f);
}

void sterge_student(StudentStorage *storage) {
    int cod;
    int idx;
    int i;
    int rezultat;
    DateStudent *student;
    char baza_date[256];
    char fisier[512];

    if (storage->nr_studenti == 0) {
        printf("Nu exista studenti inregistrati.\n");
        return;
    }

    printf("Introdu codul personal al studentului de sters: ");
    scanf("%d", &cod);

    idx = -1;
    for (i = 0; i < storage->nr_studenti; i++) {
        if (storage->studenti[i].cod_personal == cod) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("Student inexistent.\n");
        return;
    }

    student = &storage->studenti[idx];

    obtine_cale_date_studenti(baza_date, sizeof(baza_date));
    construieste_fisier_student(
        fisier,
        sizeof(fisier),
        baza_date,
        student->an_studiu,
        student->grupa,
        student->cod_personal,
        student->nume,
        student->prenume
    );

    rezultat = remove(fisier);
    if (rezultat == 0) {
        printf("Fisier sters: %s\n", fisier);
    } else {
        printf("Avertisment: fisierul nu a putut fi sters sau nu exista: %s\n", fisier);
    }

    elibereaza_situatie(&storage->studenti[idx].situatie);
    for (i = idx; i < storage->nr_studenti - 1; i++) {
        storage->studenti[i] = storage->studenti[i + 1];
    }

    storage->nr_studenti--;
    printf("Studentul cu codul %d a fost sters din baza de date.\n", cod);
}

int incarca_un_student_din_fisier(const char *cale_fisier, StudentStorage *storage) {
    FILE *f;
    char linie[256];
    char *p;
    char *endptr;
    DateStudent student_temp;
    int sem_idx;
    int disc_idx;
    int nota;
    int credit;
    int max_sem;
    int max_disc;
    int i;
    int j;
    int ok_linii;
    int in_detalii;
    int semestre_summary;
    int nr_scadere;
    int cred_sem;
    double medie_sem;
    int tmp_note[MAX_SEM_FIS][MAX_DISC_FIS];
    int tmp_credite_det[MAX_SEM_FIS][MAX_DISC_FIS];
    int tmp_credite_sum[MAX_SEM_FIS];
    double tmp_medii_sum[MAX_SEM_FIS];
    int tmp_scaderi[MAX_SEM_FIS];

    memset(&student_temp, 0, sizeof(student_temp));
    initializare_situatie(&student_temp.situatie);

    for (i = 0; i < MAX_SEM_FIS; i++) {
        tmp_credite_sum[i] = 0;
        tmp_medii_sum[i] = 0.0;
        tmp_scaderi[i] = 0;
        for (j = 0; j < MAX_DISC_FIS; j++) {
            tmp_note[i][j] = -1;
            tmp_credite_det[i][j] = -1;
        }
    }

    max_sem = 0;
    max_disc = 0;
    ok_linii = 0;
    in_detalii = 0;
    semestre_summary = 0;
    nr_scadere = 0;

    f = fopen(cale_fisier, "r");
    if (f == NULL) {
        return 0;
    }

    while (fgets(linie, sizeof(linie), f) != NULL) {
        if (strncmp(linie, "=== Detalii discipline ===", 26) == 0) {
            in_detalii = 1;
            continue;
        }

        if (strncmp(linie, "Cod personal:", 13) == 0) {
            sscanf(linie + 13, "%d", &student_temp.cod_personal);
        } else if (strncmp(linie, "Nume:", 5) == 0) {
            sscanf(linie + 5, "%49s", student_temp.nume);
        } else if (strncmp(linie, "Prenume:", 8) == 0) {
            sscanf(linie + 8, "%49s", student_temp.prenume);
        } else if (strncmp(linie, "Varsta:", 7) == 0) {
            sscanf(linie + 7, "%d", &student_temp.varsta);
        } else if (strncmp(linie, "An studiu:", 10) == 0) {
            sscanf(linie + 10, "%d", &student_temp.an_studiu);
        } else if (strncmp(linie, "Grupa:", 6) == 0) {
            sscanf(linie + 6, "%d", &student_temp.grupa);
        } else if (strncmp(linie, "Email:", 6) == 0) {
            sscanf(linie + 6, "%79s", student_temp.email);
        } else if (strncmp(linie, "Semestrul", 9) == 0 && strstr(linie, "Medie ponderata:") != NULL) {
            sem_idx = 0;
            cred_sem = 0;
            medie_sem = 0.0;
            if (sscanf(linie, "Semestrul %d -> Credite: %d | Medie ponderata: %lf", &sem_idx, &cred_sem, &medie_sem) == 3) {
                if (sem_idx >= 1 && sem_idx <= MAX_SEM_FIS) {
                    tmp_credite_sum[sem_idx - 1] = cred_sem;
                    tmp_medii_sum[sem_idx - 1] = medie_sem;
                    if (sem_idx > semestre_summary) {
                        semestre_summary = sem_idx;
                    }
                }
            }
        } else if (strncmp(linie, "Discipline cu peste 4 credite:", 29) == 0) {
            sscanf(linie + 29, "%d", &student_temp.situatie.nr_discipline_peste_4_credite);
        } else if (strncmp(linie, "Discipline picate sau absente:", 30) == 0) {
            sscanf(linie + 30, "%d", &student_temp.situatie.nr_restante_absente);
        } else if (strncmp(linie, "Semestre cu scadere:", 20) == 0) {
            p = linie + 20;
            while (*p != '\0') {
                while (*p == ' ' || *p == '\t') {
                    p++;
                }
                if (*p == '\0' || *p == '\n') {
                    break;
                }
                if (strncmp(p, "niciunul", 8) == 0) {
                    break;
                }
                sem_idx = (int)strtol(p, &endptr, 10);
                if (endptr == p) {
                    break;
                }
                if (nr_scadere < MAX_SEM_FIS) {
                    tmp_scaderi[nr_scadere] = sem_idx;
                    nr_scadere++;
                }
                p = endptr;
            }
        } else if (in_detalii) {
            sem_idx = 0;
            disc_idx = 0;
            nota = -1;
            credit = -1;

            if (sscanf(linie, "Semestrul %d, Disciplina %d: Nota=%d Credite=%d", &sem_idx, &disc_idx, &nota, &credit) == 4) {
                if (sem_idx >= 1 && sem_idx <= MAX_SEM_FIS && disc_idx >= 1 && disc_idx <= MAX_DISC_FIS) {
                    tmp_note[sem_idx - 1][disc_idx - 1] = nota;
                    tmp_credite_det[sem_idx - 1][disc_idx - 1] = credit;
                    if (sem_idx > max_sem) {
                        max_sem = sem_idx;
                    }
                    if (disc_idx > max_disc) {
                        max_disc = disc_idx;
                    }
                    ok_linii = 1;
                }
            }
        }
    }

    fclose(f);

    if (student_temp.cod_personal <= 0 || student_temp.nume[0] == '\0' || student_temp.prenume[0] == '\0') {
        return 0;
    }

    if (ok_linii && max_sem > 0 && max_disc > 0) {
        student_temp.situatie.semestre = max_sem;
        student_temp.situatie.discipline = max_disc;
        student_temp.situatie.note = (int **)malloc((size_t)max_sem * sizeof(int *));
        student_temp.situatie.credite = (int **)malloc((size_t)max_sem * sizeof(int *));
        if (student_temp.situatie.note == NULL || student_temp.situatie.credite == NULL) {
            elibereaza_situatie(&student_temp.situatie);
            return 0;
        }

        for (i = 0; i < max_sem; i++) {
            student_temp.situatie.note[i] = (int *)malloc((size_t)max_disc * sizeof(int));
            student_temp.situatie.credite[i] = (int *)malloc((size_t)max_disc * sizeof(int));
            if (student_temp.situatie.note[i] == NULL || student_temp.situatie.credite[i] == NULL) {
                elibereaza_situatie(&student_temp.situatie);
                return 0;
            }
            for (j = 0; j < max_disc; j++) {
                student_temp.situatie.note[i][j] = tmp_note[i][j];
                student_temp.situatie.credite[i][j] = tmp_credite_det[i][j];
            }
        }

        calculeaza_indicatori_scolari(&student_temp.situatie);
    } else if (semestre_summary > 0) {
        student_temp.situatie.semestre = semestre_summary;
        student_temp.situatie.discipline = 0;
        student_temp.situatie.medii_ponderate = (double *)calloc((size_t)semestre_summary, sizeof(double));
        student_temp.situatie.credite_semestru = (int *)calloc((size_t)semestre_summary, sizeof(int));
        if (nr_scadere > 0) {
            student_temp.situatie.semestre_scadere = (int *)calloc((size_t)nr_scadere, sizeof(int));
        }

        if (student_temp.situatie.medii_ponderate == NULL || student_temp.situatie.credite_semestru == NULL) {
            elibereaza_situatie(&student_temp.situatie);
            return 0;
        }

        for (i = 0; i < semestre_summary; i++) {
            student_temp.situatie.medii_ponderate[i] = tmp_medii_sum[i];
            student_temp.situatie.credite_semestru[i] = tmp_credite_sum[i];
        }

        student_temp.situatie.nr_semestre_scadere = nr_scadere;
        if (student_temp.situatie.semestre_scadere != NULL) {
            for (i = 0; i < nr_scadere; i++) {
                student_temp.situatie.semestre_scadere[i] = tmp_scaderi[i];
            }
        }
    }

    if (!asigura_capacitate_studenti(storage)) {
        return 0;
    }

    storage->studenti[storage->nr_studenti] = student_temp;
    storage->nr_studenti++;

    if (student_temp.cod_personal >= storage->urmator_cod_personal) {
        storage->urmator_cod_personal = student_temp.cod_personal + 1;
    }

    return 1;
}

int incarca_studenti_din_fisiere(StudentStorage *storage) {
    int an;
    int grupa;
    int total;

#ifdef _WIN32
    char baza_date[256];
    char director_grupa[512];
    char pattern[512];
    char cale_fisier[512];
    WIN32_FIND_DATAA data;
    HANDLE hfind;

    obtine_cale_date_studenti(baza_date, sizeof(baza_date));

    total = 0;
    for (an = 1; an <= 4; an++) {
        for (grupa = 1; grupa <= 15; grupa++) {
            construieste_director_grupa(director_grupa, sizeof(director_grupa), baza_date, an, grupa);
            snprintf(pattern, sizeof(pattern), "%s%c*.txt", director_grupa, PATH_SEP);
            hfind = FindFirstFileA(pattern, &data);
            if (hfind == INVALID_HANDLE_VALUE) {
                continue;
            }

            do {
                if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    snprintf(cale_fisier, sizeof(cale_fisier), "%s%c%s", director_grupa, PATH_SEP, data.cFileName);
                    if (incarca_un_student_din_fisier(cale_fisier, storage)) {
                        total++;
                    }
                }
            } while (FindNextFileA(hfind, &data));

            FindClose(hfind);
        }
    }

    return total;
#else
    char baza_date[256];
    char director_grupa[512];
    char cale_fisier[768];
    DIR *dir;
    struct dirent *entry;

    obtine_cale_date_studenti(baza_date, sizeof(baza_date));

    total = 0;
    for (an = 1; an <= 4; an++) {
        for (grupa = 1; grupa <= 15; grupa++) {
            construieste_director_grupa(director_grupa, sizeof(director_grupa), baza_date, an, grupa);
            dir = opendir(director_grupa);
            if (dir == NULL) {
                continue;
            }

            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] == '.') {
                    continue;
                }
                if (!are_extensie_txt(entry->d_name)) {
                    continue;
                }

                snprintf(cale_fisier, sizeof(cale_fisier), "%s%c%s", director_grupa, PATH_SEP, entry->d_name);
                if (incarca_un_student_din_fisier(cale_fisier, storage)) {
                    total++;
                }
            }

            closedir(dir);
        }
    }

    return total;
#endif
}
