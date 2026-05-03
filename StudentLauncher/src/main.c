#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "student_types.h"
#include "student_logic.h"
#include "student_io.h"
#include "gui_launcher.h"

#define printf(...) do { fprintf(stdout, __VA_ARGS__); fflush(stdout); } while (0)

int main(void) {
    int optiune;
    int incarcati;
    StudentStorage storage;

    storage.studenti = NULL;
    storage.nr_studenti = 0;
    storage.capacitate_studenti = 0;
    storage.urmator_cod_personal = 1;

    srand((unsigned int)time(NULL));

    incarcati = incarca_studenti_din_fisiere(&storage);
    if (incarcati > 0) {
        printf("Au fost incarcati %d studenti din fisiere.\n", incarcati);
    }

    do {
        printf("\n===== MENIU =====\n");
        printf("1. Adauga studenti\n");
        printf("2. Afiseaza studenti\n");
        printf("3. Introdu situatie scolara\n");
        printf("4. Raport situatie scolara\n");
        printf("5. Clasament dupa medie\n");
        printf("6. Cauta student dupa nume/prenume\n");
        printf("7. Mod grafic medii\n");
        printf("8. Sterge student\n");
        printf("0. Exit\n");
        printf("Alege optiunea: ");
        scanf("%d", &optiune);

        switch (optiune) {
            case 1:
                adaugare_studenti(&storage);
                break;
            case 2:
                afisare_studenti(&storage);
                break;
            case 3:
                introdu_situatie_pentru_student(&storage);
                break;
            case 4:
                afiseaza_raport_student(&storage);
                break;
            case 5:
                afiseaza_clasament_dupa_medie(&storage);
                break;
            case 6:
                cauta_student_dupa_nume_prenume(&storage);
                break;
            case 7:
                deschide_mod_grafic_student(&storage);
                break;
            case 8:
                sterge_student(&storage);
                break;
            case 0:
                elibereaza_toata_memoria(&storage);
                printf("Program inchis.\n");
                break;
            default:
                printf("Optiune invalida. Incearca din nou.\n");
                break;
        }
    } while (optiune != 0);

    return 0;
}
