#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static void taie_dupa_ultimul_sep(char *cale, char sep) {
    char *ultim_sep;

    ultim_sep = strrchr(cale, sep);
    if (ultim_sep != NULL) {
        *ultim_sep = '\0';
    }
}

void obtine_cale_executabil(char *buffer, size_t dim_buffer) {
    if (buffer == NULL || dim_buffer == 0) {
        return;
    }

#ifdef _WIN32
    if (GetModuleFileNameA(NULL, buffer, (DWORD)dim_buffer) == 0) {
        strncpy(buffer, ".", dim_buffer - 1);
        buffer[dim_buffer - 1] = '\0';
        return;
    }
    buffer[dim_buffer - 1] = '\0';
#else
    ssize_t len;

    len = readlink("/proc/self/exe", buffer, dim_buffer - 1);
    if (len < 0) {
        strncpy(buffer, ".", dim_buffer - 1);
        buffer[dim_buffer - 1] = '\0';
        return;
    }

    buffer[len] = '\0';
#endif

    taie_dupa_ultimul_sep(buffer, '\\');
    taie_dupa_ultimul_sep(buffer, '/');
}

int egal_ignore_case(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

void obtine_cale_date_studenti(char *buffer, size_t dim_buffer) {
    char dir_executabil[512];
    int scris;

    if (buffer == NULL || dim_buffer == 0) {
        return;
    }

#ifdef _WIN32
    obtine_cale_executabil(dir_executabil, sizeof(dir_executabil));

    while (1) {
        char *ultimul_fol;

        ultimul_fol = strrchr(dir_executabil, '\\');
        if (ultimul_fol == NULL) {
            break;
        }

        *ultimul_fol = '\0';
        ultimul_fol = strrchr(dir_executabil, '\\');
        if (ultimul_fol == NULL) {
            break;
        }

        if (egal_ignore_case(ultimul_fol + 1, "Debug") ||
            egal_ignore_case(ultimul_fol + 1, "Release") ||
            egal_ignore_case(ultimul_fol + 1, "src")) {
            continue;
        }

        break;
    }

    scris = snprintf(buffer, dim_buffer, "%s\\StudentData", dir_executabil);
    if (scris < 0 || (size_t)scris >= dim_buffer) {
        buffer[dim_buffer - 1] = '\0';
    }
#else
    obtine_cale_executabil(dir_executabil, sizeof(dir_executabil));
    snprintf(buffer, dim_buffer, "%s/StudentData", dir_executabil);
    buffer[dim_buffer - 1] = '\0';
#endif
}

void construieste_email(const char *prenume, const char *nume, char *email, size_t dim_email) {
    char prenume_mic[50];
    char nume_mic[50];
    size_t i;
    char ch;

    for (i = 0; i < sizeof(prenume_mic) - 1 && prenume[i] != '\0'; i++) {
        ch = prenume[i];
        if (ch >= 'A' && ch <= 'Z') {
            ch = (char)(ch + ('a' - 'A'));
        }
        prenume_mic[i] = ch;
    }
    prenume_mic[i] = '\0';

    for (i = 0; i < sizeof(nume_mic) - 1 && nume[i] != '\0'; i++) {
        ch = nume[i];
        if (ch >= 'A' && ch <= 'Z') {
            ch = (char)(ch + ('a' - 'A'));
        }
        nume_mic[i] = ch;
    }
    nume_mic[i] = '\0';

    snprintf(email, dim_email, "%s.%s@student.tuiasi.ro", prenume_mic, nume_mic);
}
