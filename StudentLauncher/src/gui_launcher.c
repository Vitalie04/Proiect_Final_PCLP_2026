#include "gui_launcher.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "student_logic.h"
#include "utils.h"

#define printf(...) do { fprintf(stdout, __VA_ARGS__); fflush(stdout); } while (0)

static int exista_fisier(const char *cale) {
    FILE *f;

    f = fopen(cale, "r");
    if (f == NULL) {
        return 0;
    }

    fclose(f);
    return 1;
}

static void construieste_cale_student(
    char *dest,
    size_t dim_dest,
    const char *baza_studenti,
    const DateStudent *student,
    int pune_prenumele_intai
) {
    snprintf(
        dest,
        dim_dest,
        "%s/an_%d/grupa_%d/%d_%s_%s.txt",
        baza_studenti,
        student->an_studiu,
        student->grupa,
        student->cod_personal,
        pune_prenumele_intai ? student->prenume : student->nume,
        pune_prenumele_intai ? student->nume : student->prenume
    );
}

static int rezolva_fisier_student(
    const char *baza_studenti,
    const DateStudent *student,
    char *cale_finala,
    size_t dim_cale_finala
) {
    char cale_normala[700];
    char cale_alternativa[700];

    construieste_cale_student(cale_normala, sizeof(cale_normala), baza_studenti, student, 0);
    if (exista_fisier(cale_normala)) {
        strncpy(cale_finala, cale_normala, dim_cale_finala - 1);
        cale_finala[dim_cale_finala - 1] = '\0';
        return 1;
    }

    construieste_cale_student(cale_alternativa, sizeof(cale_alternativa), baza_studenti, student, 1);
    if (exista_fisier(cale_alternativa)) {
        strncpy(cale_finala, cale_alternativa, dim_cale_finala - 1);
        cale_finala[dim_cale_finala - 1] = '\0';
        return 1;
    }

    return 0;
}

static int adauga_fisier_unic(char fisiere_studenti[][700], int *nr_fisiere_lansare, const char *cale) {
    int i;

    for (i = 0; i < *nr_fisiere_lansare; i++) {
        if (strcmp(fisiere_studenti[i], cale) == 0) {
            return 0;
        }
    }

    if (*nr_fisiere_lansare >= 5) {
        return 0;
    }

    strncpy(fisiere_studenti[*nr_fisiere_lansare], cale, 699);
    fisiere_studenti[*nr_fisiere_lansare][699] = '\0';
    (*nr_fisiere_lansare)++;
    return 1;
}

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

static void obtine_radacina_proiect(char *radacina_proiect, size_t dim_radacina_proiect) {
    char dir_executabil[512];
    char *ultim_sep;

    obtine_cale_executabil(dir_executabil, sizeof(dir_executabil));
    strncpy(radacina_proiect, dir_executabil, dim_radacina_proiect - 1);
    radacina_proiect[dim_radacina_proiect - 1] = '\0';

    while (1) {
        ultim_sep = strrchr(radacina_proiect, '\\');
        if (ultim_sep == NULL) {
            break;
        }

        if (egal_ignore_case(ultim_sep + 1, "Debug") ||
            egal_ignore_case(ultim_sep + 1, "Release") ||
            egal_ignore_case(ultim_sep + 1, "src")) {
            *ultim_sep = '\0';
            continue;
        }

        break;
    }

    ultim_sep = strrchr(radacina_proiect, '\\');
    if (ultim_sep != NULL && egal_ignore_case(ultim_sep + 1, "StudentLauncher")) {
        *ultim_sep = '\0';
    }
}

void deschide_mod_grafic_student(StudentStorage *storage) {
    int cod;
    int i;
    int j;
    int mod_comparare;
    int cod_comparat;
    int numar_comparatii;
    int comparatii_adaugate;
    int nr_fisiere_lansare;
    DateStudent *student;
    DateStudent *student_comparat;
    char fisier_rel[700];
    char fisier_comparat[700];
    char fisiere_studenti[5][700];
    char radacina_proiect[512];
    char baza_studenti[512];
    char exe_grafic[512];
    char parametri_lansare[4096];
    char comanda_build[1024];
    HINSTANCE rezultat_lansare;
    int scris;
    size_t len_parametri;
    DWORD atribute;
    int rezultat_build;

    if (storage->nr_studenti == 0) {
        printf("Nu exista studenti inregistrati.\n");
        return;
    }

    printf("Introdu codul personal pentru modul grafic (0=Cancel): ");
    scanf("%d", &cod);
    if (cod == 0) {
        printf("Deschiderea modului grafic a fost anulata.\n");
        return;
    }

    student = cauta_student_dupa_cod(storage, cod);
    if (student == NULL) {
        printf("Student inexistent.\n");
        return;
    }

    obtine_radacina_proiect(radacina_proiect, sizeof(radacina_proiect));

    scris = snprintf(
        baza_studenti,
        sizeof(baza_studenti),
        "%s\\StudentLauncher\\StudentData",
        radacina_proiect
    );
    if (scris < 0 || (size_t)scris >= sizeof(baza_studenti)) {
        printf("Calea catre StudentData este prea lunga.\n");
        return;
    }

    atribute = GetFileAttributesA(baza_studenti);
    if (atribute == INVALID_FILE_ATTRIBUTES || (atribute & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        printf("Nu exista folderul StudentData: %s\n", baza_studenti);
        return;
    }

    if (!rezolva_fisier_student(baza_studenti, student, fisier_rel, sizeof(fisier_rel))) {
        printf("Nu exista fisierul studentului.\n");
        return;
    }

    strncpy(fisiere_studenti[0], fisier_rel, sizeof(fisiere_studenti[0]) - 1);
    fisiere_studenti[0][sizeof(fisiere_studenti[0]) - 1] = '\0';
    nr_fisiere_lansare = 1;
    comparatii_adaugate = 0;

    printf("Mod comparare: 1=manual, 2=toata grupa (max 4 suplimentari): ");
    scanf("%d", &mod_comparare);

    if (mod_comparare == 2) {
        for (i = 0; i < storage->nr_studenti && nr_fisiere_lansare < 5; i++) {
            student_comparat = &storage->studenti[i];

            if (student_comparat->cod_personal == student->cod_personal) {
                continue;
            }
            if (student_comparat->an_studiu != student->an_studiu ||
                student_comparat->grupa != student->grupa) {
                continue;
            }

            if (!rezolva_fisier_student(baza_studenti, student_comparat, fisier_comparat, sizeof(fisier_comparat))) {
                continue;
            }

            if (adauga_fisier_unic(fisiere_studenti, &nr_fisiere_lansare, fisier_comparat)) {
                comparatii_adaugate++;
            }
        }

        if (comparatii_adaugate == 0) {
            printf("Nu am gasit alti studenti din aceeasi grupa cu fisier valid.\n");
        }
    } else {
        if (mod_comparare != 1) {
            printf("Optiune invalida, se foloseste comparare manuala.\n");
        }

        printf("Numar studenti suplimentari pentru comparatie (0-4): ");
        scanf("%d", &numar_comparatii);
        if (numar_comparatii < 0) {
            numar_comparatii = 0;
        }
        if (numar_comparatii > 4) {
            numar_comparatii = 4;
        }

        for (i = 0; i < numar_comparatii; i++) {
            printf("Cod student comparat #%d (0=Skip): ", i + 1);
            scanf("%d", &cod_comparat);

            if (cod_comparat == 0) {
                continue;
            }
            if (cod_comparat == cod) {
                printf("Codul %d este deja selectat ca student principal.\n", cod_comparat);
                continue;
            }

            student_comparat = cauta_student_dupa_cod(storage, cod_comparat);
            if (student_comparat == NULL) {
                printf("Student inexistent pentru codul %d.\n", cod_comparat);
                continue;
            }

            if (!rezolva_fisier_student(baza_studenti, student_comparat, fisier_comparat, sizeof(fisier_comparat))) {
                printf("Nu exista fisier pentru codul %d.\n", cod_comparat);
                continue;
            }

            if (adauga_fisier_unic(fisiere_studenti, &nr_fisiere_lansare, fisier_comparat)) {
                comparatii_adaugate++;
            } else {
                printf("Studentul cu codul %d este deja adaugat la comparatie sau lista este plina.\n", cod_comparat);
            }

            if (nr_fisiere_lansare >= 5) {
                break;
            }
        }
    }

    scris = snprintf(
        exe_grafic,
        sizeof(exe_grafic),
        "%s\\medii_opengl\\Debug\\medii_opengl.exe",
        radacina_proiect
    );
    if (scris < 0 || (size_t)scris >= sizeof(exe_grafic)) {
        printf("Calea catre executabilul grafic este prea lunga.\n");
        return;
    }

    atribute = GetFileAttributesA(exe_grafic);
    if (atribute == INVALID_FILE_ATTRIBUTES || (atribute & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        printf("Executabilul grafic nu exista. Se incearca build automat...\n");

        snprintf(comanda_build, sizeof(comanda_build), "cd %s\\medii_opengl && mingw32-make Debug 2>&1", radacina_proiect);
        rezultat_build = system(comanda_build);
        if (rezultat_build != 0) {
            printf("Esuare build medii_opengl (cod=%d). Compileaza manual proiectul grafic.\n", rezultat_build);
            return;
        }

        printf("Build grafic finalizat cu succes. Verific din nou...\n");
        atribute = GetFileAttributesA(exe_grafic);
        if (atribute == INVALID_FILE_ATTRIBUTES || (atribute & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            printf("Dupa build, executabilul grafic inca lipseste: %s\n", exe_grafic);
            return;
        }
    }

    printf("\nLansez modul grafic pentru codul %d...\n", student->cod_personal);
    printf("Fisier student: %s\n", fisier_rel);
    printf("Executabil grafic: %s\n", exe_grafic);

    if (comparatii_adaugate > 0) {
        printf("Comparatie activa cu %d studenti suplimentari.\n", comparatii_adaugate);
    }

    parametri_lansare[0] = '\0';
    len_parametri = 0;
    for (i = 0; i < nr_fisiere_lansare; i++) {
        scris = snprintf(
            parametri_lansare + len_parametri,
            sizeof(parametri_lansare) - len_parametri,
            "%s\"%s\"",
            (i == 0) ? "" : " ",
            fisiere_studenti[i]
        );
        if (scris < 0 || (size_t)scris >= sizeof(parametri_lansare) - len_parametri) {
            printf("Lista fisierelor pentru comparatie este prea lunga.\n");
            return;
        }
        len_parametri += (size_t)scris;
    }

    rezultat_lansare = ShellExecuteA(NULL, "open", exe_grafic, parametri_lansare, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)rezultat_lansare <= 32) {
        printf("Nu am putut lansa modulul grafic prin ShellExecuteA (cod=%lld).\n", (long long)(INT_PTR)rezultat_lansare);
    }
}

#else

void deschide_mod_grafic_student(StudentStorage *storage) {
    int cod;
    DateStudent *student;
    char baza_studenti[256];
    char fisier_rel[700];
    const char *executabile_posibile[] = {
        "../medii_opengl/Debug/medii_opengl",
        "medii_opengl/Debug/medii_opengl",
        "../medii_opengl/Debug/medii_opengl.exe",
        "medii_opengl/Debug/medii_opengl.exe"
    };
    const char *exe_grafic;
    char comanda[1800];
    size_t i;
    int scris;

    if (storage->nr_studenti == 0) {
        printf("Nu exista studenti inregistrati.\n");
        return;
    }

    printf("Introdu codul personal pentru modul grafic (0=Cancel): ");
    scanf("%d", &cod);
    if (cod == 0) {
        printf("Deschiderea modului grafic a fost anulata.\n");
        return;
    }

    student = cauta_student_dupa_cod(storage, cod);
    if (student == NULL) {
        printf("Student inexistent.\n");
        return;
    }

    obtine_cale_date_studenti(baza_studenti, sizeof(baza_studenti));

    if (!rezolva_fisier_student(baza_studenti, student, fisier_rel, sizeof(fisier_rel))) {
        printf("Nu exista fisierul studentului.\n");
        return;
    }

    exe_grafic = NULL;
    for (i = 0; i < sizeof(executabile_posibile) / sizeof(executabile_posibile[0]); i++) {
        if (exista_fisier(executabile_posibile[i])) {
            exe_grafic = executabile_posibile[i];
            break;
        }
    }

    if (exe_grafic == NULL) {
        printf("Nu am gasit executabilul grafic. Cautat in:\n");
        for (i = 0; i < sizeof(executabile_posibile) / sizeof(executabile_posibile[0]); i++) {
            printf("  - %s\n", executabile_posibile[i]);
        }
        printf("Compileaza proiectul medii_opengl si incearca din nou.\n");
        return;
    }

    scris = snprintf(comanda, sizeof(comanda), "\"%s\" \"%s\"", exe_grafic, fisier_rel);
    if (scris < 0 || (size_t)scris >= sizeof(comanda)) {
        printf("Comanda de lansare este prea lunga.\n");
        return;
    }

    printf("Lansez modul grafic: %s\n", exe_grafic);
    if (system(comanda) != 0) {
        printf("Lansarea modulului grafic a esuat.\n");
    }
}

#endif
