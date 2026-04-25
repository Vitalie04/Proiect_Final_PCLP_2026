#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define printf(...) do { fprintf(stdout, __VA_ARGS__); fflush(stdout); } while (0)

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(_WIN32)
#if defined(__has_include)
#if __has_include(<GL/freeglut.h>)
#include <GL/freeglut.h>
#elif __has_include(<freeglut.h>)
#include <freeglut.h>
#elif __has_include(<GL/glut.h>)
#include <GL/glut.h>
#else
#include <GL/glut.h>
#endif
#else
#include <GL/glut.h>
#endif
#else
#include <GL/glut.h>
#endif

#define MAX_MEDII 40
#define MAX_STUDENTI_COMPARARE 5

typedef struct {
    char nume[50];
    char prenume[50];
    int cod;
    float valori[MAX_MEDII];
    float initiale[MAX_MEDII];
    int count;
    float media_generala;
    float min_medie;
    float max_medie;
} StudentMedii;

typedef struct {
    StudentMedii studenti[MAX_STUDENTI_COMPARARE];
    int nr_studenti;
    int selectat_student;  /* studentul activ */
    int selectat_medie;    /* media selectata in cadrul studentului */
    int sortat_mode;       /* 0 = original, 1 = crescator, 2 = descrescator */
    int mod_afisare;       /* 0 = single, 1 = compare */
    int mod_interactiune;  /* pentru single: 0 = edit medii, 1 = statistici avansate */
} ModelMedii;

static ModelMedii g_model;
static int g_win_w = 1100;
static int g_win_h = 700;

void calculeaza_statistici(ModelMedii *m);

int incarca_medii_din_fisier(StudentMedii *s, const char *cale_fisier) {
    FILE *f;
    char linie[512];
    int count;
    char *p;
    float medie;

    f = fopen(cale_fisier, "r");
    if (f == NULL) {
        return 0;
    }

    memset(s, 0, sizeof(*s));
    count = 0;

    while (fgets(linie, sizeof(linie), f) != NULL) {
        p = strstr(linie, "Medie ponderata:");
        if (p != NULL) {
            p += (int)strlen("Medie ponderata:");
            medie = (float)strtod(p, NULL);
            if (medie >= 0.0f && medie <= 10.0f && count < MAX_MEDII) {
                s->valori[count] = medie;
                s->initiale[count] = medie;
                count++;
            }
        }
    }

    fclose(f);

    if (count <= 0) {
        return 0;
    }

    s->count = count;
    strcpy(s->nume, "Student");
    strcpy(s->prenume, "");
    s->cod = 0;
    return 1;
}

void calculeaza_statistici(ModelMedii *m) {
    int i, j;
    float suma;
    StudentMedii *s;

    for (j = 0; j < m->nr_studenti; j++) {
        s = &m->studenti[j];
        
        if (s->count <= 0) {
            s->media_generala = 0.0f;
            s->min_medie = 0.0f;
            s->max_medie = 0.0f;
            continue;
        }

        suma = 0.0f;
        s->min_medie = s->valori[0];
        s->max_medie = s->valori[0];

        for (i = 0; i < s->count; i++) {
            suma += s->valori[i];
            if (s->valori[i] < s->min_medie) {
                s->min_medie = s->valori[i];
            }
            if (s->valori[i] > s->max_medie) {
                s->max_medie = s->valori[i];
            }
        }

        s->media_generala = suma / (float)s->count;
    }
}

void reset_la_initial(ModelMedii *m) {
    int i;
    StudentMedii *s;

    s = &m->studenti[m->selectat_student];

    for (i = 0; i < s->count; i++) {
        s->valori[i] = s->initiale[i];
    }
    m->sortat_mode = 0;
    calculeaza_statistici(m);
}

void sorteaza(ModelMedii *m, int crescator) {
    int i, j;
    float tmp;
    StudentMedii *s;

    s = &m->studenti[m->selectat_student];

    for (i = 0; i < s->count - 1; i++) {
        for (j = i + 1; j < s->count; j++) {
            if (crescator) {
                if (s->valori[i] > s->valori[j]) {
                    tmp = s->valori[i];
                    s->valori[i] = s->valori[j];
                    s->valori[j] = tmp;
                }
            } else {
                if (s->valori[i] < s->valori[j]) {
                    tmp = s->valori[i];
                    s->valori[i] = s->valori[j];
                    s->valori[j] = tmp;
                }
            }
        }
    }

    m->sortat_mode = crescator ? 1 : 2;
    calculeaza_statistici(m);
}

void draw_text(float x, float y, void *font, const char *text) {
    int i;
    glRasterPos2f(x, y);
    for (i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(font, text[i]);
    }
}

void draw_ui_panel(void) {
    char buff[512];
    StudentMedii *s;

    glColor3f(0.08f, 0.11f, 0.16f);
    
    if (g_model.mod_afisare == 0) {
        draw_text(20.0f, g_win_h - 34.0f, GLUT_BITMAP_HELVETICA_18, "Mod Grafic - Student Individual");
    } else {
        draw_text(20.0f, g_win_h - 34.0f, GLUT_BITMAP_HELVETICA_18, "Mod Grafic - Comparare Studenti");
    }

    s = &g_model.studenti[g_model.selectat_student];

    snprintf(buff, sizeof(buff), "Student: %s %s (cod: %d) | Media generala: %.2f | Min: %.2f | Max: %.2f",
             s->nume, s->prenume, s->cod, s->media_generala, s->min_medie, s->max_medie);
    glColor3f(0.15f, 0.20f, 0.24f);
    draw_text(20.0f, g_win_h - 62.0f, GLUT_BITMAP_HELVETICA_18, buff);

    if (g_model.sortat_mode == 0) {
        strcpy(buff, "Ordine: originala");
    } else if (g_model.sortat_mode == 1) {
        strcpy(buff, "Ordine: crescator");
    } else {
        strcpy(buff, "Ordine: descrescator");
    }
    glColor3f(0.30f, 0.30f, 0.30f);
    draw_text(20.0f, g_win_h - 86.0f, GLUT_BITMAP_HELVETICA_18, buff);

    if (g_model.mod_afisare == 0) {
        if (g_model.mod_interactiune == 0) {
            draw_text(20.0f, 50.0f, GLUT_BITMAP_HELVETICA_12, "MOD: EDITARE [A/D] selectie [W/S] ±medie [C] sort asc [V] sort desc [R] reset [T] comparare [E] statistici [ESC] iesire");
        } else {
            draw_text(20.0f, 50.0f, GLUT_BITMAP_HELVETICA_12, "MOD: STATISTICI [E] editare [T] comparare [ESC] iesire");
        }
    } else {
        draw_text(20.0f, 50.0f, GLUT_BITMAP_HELVETICA_12, "[←/→] : selecteaza student | [T] : single view | [ESC] : iesire");
    }
}

void draw_axes(float left, float bottom, float right, float top) {
    float y;
    int i;
    char buff[32];

    glColor3f(0.78f, 0.78f, 0.78f);
    glBegin(GL_LINES);
    glVertex2f(left, bottom);
    glVertex2f(left, top);
    glVertex2f(left, bottom);
    glVertex2f(right, bottom);
    glEnd();

    for (i = 0; i <= 10; i++) {
        y = bottom + (top - bottom) * ((float)i / 10.0f);
        glColor3f(0.90f, 0.90f, 0.90f);
        glBegin(GL_LINES);
        glVertex2f(left, y);
        glVertex2f(right, y);
        glEnd();

        snprintf(buff, sizeof(buff), "%d", i);
        glColor3f(0.35f, 0.35f, 0.35f);
        draw_text(left - 24.0f, y - 4.0f, GLUT_BITMAP_HELVETICA_10, buff);
    }
}

void draw_bars_single(float left, float bottom, float right, float top) {
    int i;
    float chart_w;
    float bar_w;
    float gap;
    float x0;
    float x1;
    float h;
    float y1;
    char label[64];
    StudentMedii *s;

    s = &g_model.studenti[g_model.selectat_student];

    if (s->count <= 0) {
        return;
    }

    chart_w = right - left;
    gap = 10.0f;
    bar_w = (chart_w - gap * (float)(s->count + 1)) / (float)s->count;

    if (bar_w < 8.0f) {
        bar_w = 8.0f;
    }

    for (i = 0; i < s->count; i++) {
        x0 = left + gap + (bar_w + gap) * (float)i;
        x1 = x0 + bar_w;
        h = (top - bottom) * (s->valori[i] / 10.0f);
        y1 = bottom + h;

        if (i == g_model.selectat_medie) {
            glColor3f(0.90f, 0.32f, 0.20f);
        } else {
            glColor3f(0.20f, 0.55f, 0.85f);
        }

        glBegin(GL_QUADS);
        glVertex2f(x0, bottom);
        glVertex2f(x1, bottom);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();

        glColor3f(0.05f, 0.05f, 0.05f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x0, bottom);
        glVertex2f(x1, bottom);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();

        snprintf(label, sizeof(label), "S%d", i + 1);
        glColor3f(0.20f, 0.20f, 0.20f);
        draw_text(x0 + 2.0f, bottom - 16.0f, GLUT_BITMAP_HELVETICA_12, label);

        snprintf(label, sizeof(label), "%.2f", s->valori[i]);
        draw_text(x0 + 2.0f, y1 + 8.0f, GLUT_BITMAP_HELVETICA_10, label);
    }
}

void draw_bars_compare(float left, float bottom, float right, float top) {
    int i, j;
    float chart_w;
    float bar_w;
    float gap;
    float group_gap;
    float x0, x1;
    float h, y1;
    char label[64];
    StudentMedii *s;
    int max_count;
    float colors[MAX_STUDENTI_COMPARARE][3] = {
        {0.20f, 0.55f, 0.85f},  /* albastru */
        {0.90f, 0.32f, 0.20f},  /* rosu */
        {0.20f, 0.85f, 0.35f},  /* verde */
        {0.85f, 0.75f, 0.20f},  /* galben */
        {0.85f, 0.20f, 0.75f}   /* mov */
    };

    if (g_model.nr_studenti <= 0) {
        return;
    }

    /* gaseste count-ul maxim */
    max_count = 0;
    for (j = 0; j < g_model.nr_studenti; j++) {
        if (g_model.studenti[j].count > max_count) {
            max_count = g_model.studenti[j].count;
        }
    }

    chart_w = right - left;
    group_gap = 15.0f;
    gap = 5.0f;
    bar_w = (chart_w - group_gap * (float)(max_count + 1)) / ((float)max_count * (float)g_model.nr_studenti);

    if (bar_w < 4.0f) {
        bar_w = 4.0f;
    }

    /* deseneaza barurile */
    for (i = 0; i < max_count; i++) {
        for (j = 0; j < g_model.nr_studenti; j++) {
            s = &g_model.studenti[j];
            if (i >= s->count) {
                continue;
            }

            x0 = left + group_gap + (float)i * (group_gap + bar_w * (float)g_model.nr_studenti + gap * (float)(g_model.nr_studenti - 1)) + (bar_w + gap) * (float)j;
            x1 = x0 + bar_w;
            h = (top - bottom) * (s->valori[i] / 10.0f);
            y1 = bottom + h;

            glColor3f(colors[j][0], colors[j][1], colors[j][2]);

            glBegin(GL_QUADS);
            glVertex2f(x0, bottom);
            glVertex2f(x1, bottom);
            glVertex2f(x1, y1);
            glVertex2f(x0, y1);
            glEnd();

            glColor3f(0.05f, 0.05f, 0.05f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x0, bottom);
            glVertex2f(x1, bottom);
            glVertex2f(x1, y1);
            glVertex2f(x0, y1);
            glEnd();

            snprintf(label, sizeof(label), "%.1f", s->valori[i]);
            glColor3f(0.05f, 0.05f, 0.05f);
            draw_text(x0 + 1.0f, y1 + 2.0f, GLUT_BITMAP_HELVETICA_10, label);
        }

        /* label pentru grup de semestre */
        snprintf(label, sizeof(label), "S%d", i + 1);
        glColor3f(0.20f, 0.20f, 0.20f);
        float group_center = left + group_gap + (float)i * (group_gap + bar_w * (float)g_model.nr_studenti + gap * (float)(g_model.nr_studenti - 1)) + (bar_w * (float)g_model.nr_studenti + gap * (float)(g_model.nr_studenti - 1)) / 2.0f;
        draw_text(group_center - 8.0f, bottom - 16.0f, GLUT_BITMAP_HELVETICA_12, label);
    }

    /* legenda studenti */
    for (j = 0; j < g_model.nr_studenti; j++) {
        s = &g_model.studenti[j];
        snprintf(label, sizeof(label), "■ %s %s", s->nume, s->prenume);
        glColor3f(colors[j][0], colors[j][1], colors[j][2]);
        draw_text(right - 250.0f, g_win_h - 120.0f - (float)j * 20.0f, GLUT_BITMAP_HELVETICA_10, label);
    }
}

void draw_bars(float left, float bottom, float right, float top) {
    if (g_model.mod_afisare == 0) {
        draw_bars_single(left, bottom, right, top);
    } else {
        draw_bars_compare(left, bottom, right, top);
    }
}

void display(void) {
    float left;
    float right;
    float bottom;
    float top;

    glClearColor(0.98f, 0.98f, 0.97f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, (float)g_win_w, 0.0f, (float)g_win_h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    left = 75.0f;
    right = (float)g_win_w - 40.0f;
    bottom = 90.0f;
    top = (float)g_win_h - 120.0f;

    draw_ui_panel();
    draw_axes(left, bottom, right, top);
    draw_bars(left, bottom, right, top);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    g_win_w = (w > 500) ? w : 500;
    g_win_h = (h > 400) ? h : 400;
    glViewport(0, 0, g_win_w, g_win_h);
    glutPostRedisplay();
}

void key_normal(unsigned char key, int x, int y) {
    (void)x;
    (void)y;

    StudentMedii *s;

    s = &g_model.studenti[g_model.selectat_student];

    switch (key) {
        case 27:  /* ESC */
            exit(0);
            break;
        case 't':
        case 'T':  /* Toggle compare mode */
            g_model.mod_afisare = (g_model.mod_afisare == 0) ? 1 : 0;
            break;
        case 'e':
        case 'E':  /* Toggle interactiune mode (single only) */
            if (g_model.mod_afisare == 0) {
                g_model.mod_interactiune = (g_model.mod_interactiune == 0) ? 1 : 0;
            }
            break;
        case 'm':
        case 'M':  /* Recalculate statistics */
            if (g_model.mod_afisare == 0) {
                calculeaza_statistici(&g_model);
            }
            break;
        case 'c':
        case 'C':  /* Sort ascending (crescrescator) */
            if (g_model.mod_afisare == 0) {
                sorteaza(&g_model, 1);
            }
            break;
        case 'v':
        case 'V':  /* Sort descending */
            if (g_model.mod_afisare == 0) {
                sorteaza(&g_model, 0);
            }
            break;
        case 'r':
        case 'R':  /* Reset to initial */
            if (g_model.mod_afisare == 0) {
                reset_la_initial(&g_model);
            }
            break;
        case 'w':
        case 'W':  /* Increase selected grade */
            if (g_model.mod_afisare == 0 && g_model.mod_interactiune == 0) {
                s->valori[g_model.selectat_medie] += 0.10f;
                if (s->valori[g_model.selectat_medie] > 10.0f) {
                    s->valori[g_model.selectat_medie] = 10.0f;
                }
                calculeaza_statistici(&g_model);
            }
            break;
        case 's':
        case 'S':  /* Decrease selected grade */
            if (g_model.mod_afisare == 0 && g_model.mod_interactiune == 0) {
                s->valori[g_model.selectat_medie] -= 0.10f;
                if (s->valori[g_model.selectat_medie] < 0.0f) {
                    s->valori[g_model.selectat_medie] = 0.0f;
                }
                calculeaza_statistici(&g_model);
            }
            break;
        default:
            break;
    }

    glutPostRedisplay();
}

void key_special(int key, int x, int y) {
    (void)x;
    (void)y;

    if (g_model.mod_afisare == 0) {
        /* Single student mode */
        if (key == GLUT_KEY_LEFT) {
            g_model.selectat_medie--;
            if (g_model.selectat_medie < 0) {
                g_model.selectat_medie = g_model.studenti[g_model.selectat_student].count - 1;
            }
        } else if (key == GLUT_KEY_RIGHT) {
            g_model.selectat_medie++;
            if (g_model.selectat_medie >= g_model.studenti[g_model.selectat_student].count) {
                g_model.selectat_medie = 0;
            }
        }
    } else {
        /* Compare mode */
        if (key == GLUT_KEY_LEFT) {
            g_model.selectat_student--;
            if (g_model.selectat_student < 0) {
                g_model.selectat_student = g_model.nr_studenti - 1;
            }
        } else if (key == GLUT_KEY_RIGHT) {
            g_model.selectat_student++;
            if (g_model.selectat_student >= g_model.nr_studenti) {
                g_model.selectat_student = 0;
            }
        }
    }

    glutPostRedisplay();
}

int citeste_medii_initiale(StudentMedii *s) {
    int i;

    memset(s, 0, sizeof(*s));

    printf("Numar medii (max %d): ", MAX_MEDII);
    if (scanf("%d", &s->count) != 1) {
        return 0;
    }

    if (s->count < 1 || s->count > MAX_MEDII) {
        printf("Numar invalid.\n");
        return 0;
    }

    for (i = 0; i < s->count; i++) {
        printf("Media semestrul %d (0..10): ", i + 1);
        if (scanf("%f", &s->valori[i]) != 1) {
            return 0;
        }
        if (s->valori[i] < 0.0f || s->valori[i] > 10.0f) {
            printf("Valoare invalida.\n");
            return 0;
        }
        s->initiale[i] = s->valori[i];
    }

    strcpy(s->nume, "Student");
    strcpy(s->prenume, "");
    s->cod = 0;
    return 1;
}

int main(int argc, char **argv) {
    int ok, i, nr_fisiere;
    const char *cale_fisier;
    const char *p, *inceput;
    int lungime;
    char fisier_curent[1024];

    /* Parse argumente: fisierele pot fi separate prin | */
    cale_fisier = NULL;
    if (argc > 1) {
        cale_fisier = argv[1];
    }

    memset(&g_model, 0, sizeof(g_model));
    g_model.nr_studenti = 0;
    g_model.selectat_student = 0;
    g_model.selectat_medie = 0;
    g_model.sortat_mode = 0;
    g_model.mod_afisare = 0;  /* single student mode by default - schimbam mai jos daca sunt mai multi */
    g_model.mod_interactiune = 0;  /* edit mode by default */

    if (argc > 1) {
        nr_fisiere = 0;
        ok = 0;

        /* Format nou: fiecare cale este argument separat. */
        for (i = 1; i < argc && nr_fisiere < MAX_STUDENTI_COMPARARE; i++) {
            /* Compatibilitate cu formatul vechi: un singur argument cu separator | */
            if (strchr(argv[i], '|') != NULL) {
                inceput = argv[i];
                while (*inceput != '\0' && nr_fisiere < MAX_STUDENTI_COMPARARE) {
                    p = strchr(inceput, '|');

                    if (p == NULL) {
                        strncpy(fisier_curent, inceput, sizeof(fisier_curent) - 1);
                        fisier_curent[sizeof(fisier_curent) - 1] = '\0';
                    } else {
                        lungime = (int)(p - inceput);
                        if (lungime > (int)sizeof(fisier_curent) - 1) {
                            lungime = sizeof(fisier_curent) - 1;
                        }
                        strncpy(fisier_curent, inceput, lungime);
                        fisier_curent[lungime] = '\0';
                    }

                    ok = incarca_medii_din_fisier(&g_model.studenti[nr_fisiere], fisier_curent);
                    if (ok) {
                        g_model.nr_studenti++;
                        nr_fisiere++;
                    } else {
                        printf("Nu am putut incarca mediile din fisier: %s\n", fisier_curent);
                    }

                    if (p == NULL) {
                        break;
                    }
                    inceput = p + 1;
                }
            } else {
                ok = incarca_medii_din_fisier(&g_model.studenti[nr_fisiere], argv[i]);
                if (ok) {
                    g_model.nr_studenti++;
                    nr_fisiere++;
                } else {
                    printf("Nu am putut incarca mediile din fisier: %s\n", argv[i]);
                }
            }
        }
        
        /* Daca sunt mai multi studenti, activeaza modul comparare */
        if (g_model.nr_studenti > 1) {
            g_model.mod_afisare = 1;
        }
        
        if (g_model.nr_studenti == 0) {
            printf("Nu am putut incarca niciun student din fisiere.\n");
            printf("Comut pe introducere manuala.\n");
        } else {
            ok = 1;
        }
    } else {
        ok = 0;
    }

    if (!ok) {
        ok = citeste_medii_initiale(&g_model.studenti[0]);
        if (ok) {
            g_model.nr_studenti = 1;
        }
    }

    if (!ok || g_model.nr_studenti <= 0) {
        printf("Date de intrare invalide. Programul se opreste.\n");
        return 1;
    }

    calculeaza_statistici(&g_model);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(g_win_w, g_win_h);
    glutCreateWindow("Mod grafic medii - OpenGL");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key_normal);
    glutSpecialFunc(key_special);

    glutMainLoop();
    return 0;
}
