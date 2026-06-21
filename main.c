#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
typedef long long ssize_t;
static ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos = 0;
    int c;
    if (!*lineptr || *n == 0) { *n = 128; *lineptr = malloc(*n); if (!*lineptr) return -1; }
    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {
            size_t nn = *n * 2;
            char *p = realloc(*lineptr, nn);
            if (!p) return -1;
            *lineptr = p; *n = nn;
        }
        (*lineptr)[pos++] = (char)c;
        if (c == '\n') break;
    }
    if (pos == 0 && c == EOF) return -1;
    (*lineptr)[pos] = '\0';
    return (ssize_t)pos;
}
static int file_access(const char *p) { return _access(p, 0) == 0; }
#else
#include <unistd.h>
static int file_access(const char *p) { return access(p, F_OK) == 0; }
#endif

static const char* FILE_NAME = "password.txt";

static char* in_line(void) {
    char* line = NULL;
    size_t cap = 0;
    ssize_t n = getline(&line, &cap, stdin);
    if (n < 0) { free(line); return calloc(1, 1); }
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = 0;
    return line;
}

static char* in_prompt(const char* prompt) {
    fputs(prompt, stdout);
    return in_line();
}

static char* str_lower_dup(const char* s) {
    size_t n = strlen(s);
    char* r = malloc(n + 1);
    for (size_t i = 0; i < n; i++) r[i] = (char)tolower((unsigned char)s[i]);
    r[n] = 0;
    return r;
}

static char* strip_inplace(char* s) {
    char* p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    char* e = p + strlen(p);
    while (e > p && isspace((unsigned char)e[-1])) *--e = 0;
    return p;
}

static int split_pipe(char* s, char* parts[], int maxparts) {
    int n = 0;
    parts[n++] = s;
    for (char* p = s; *p && n < maxparts; ++p)
        if (*p == '|') { *p = 0; parts[n++] = p + 1; }
    return n;
}

static int file_exists(const char* p) { return file_access(p); }

typedef struct { int a, b, size; } Match;

static Match find_longest_match(const char* a, const char* b,
                                int alo, int ahi, int blo, int bhi) {
    int besti = alo, bestj = blo, bestsize = 0;
    int* j2len = calloc((size_t)bhi + 1, sizeof(int));
    int* newj2len = calloc((size_t)bhi + 1, sizeof(int));
    for (int i = alo; i < ahi; ++i) {
        memset(newj2len, 0, ((size_t)bhi + 1) * sizeof(int));
        for (int j = blo; j < bhi; ++j) {
            if (b[j] != a[i]) continue;
            int k = 1;
            if (j - 1 >= 0) k = j2len[j - 1] + 1;
            newj2len[j] = k;
            if (k > bestsize) {
                besti = i - k + 1;
                bestj = j - k + 1;
                bestsize = k;
            }
        }
        int* tmp = j2len; j2len = newj2len; newj2len = tmp;
    }
    while (besti > alo && bestj > blo && a[besti - 1] == b[bestj - 1]) {
        --besti; --bestj; ++bestsize;
    }
    while (besti + bestsize < ahi && bestj + bestsize < bhi &&
           a[besti + bestsize] == b[bestj + bestsize]) {
        ++bestsize;
    }
    free(j2len);
    free(newj2len);
    Match m = { besti, bestj, bestsize };
    return m;
}

static int match_count(const char* a, const char* b,
                       int alo, int ahi, int blo, int bhi) {
    Match m = find_longest_match(a, b, alo, ahi, blo, bhi);
    if (m.size == 0) return 0;
    return m.size
         + match_count(a, b, alo, m.a, blo, m.b)
         + match_count(a, b, m.a + m.size, ahi, m.b + m.size, bhi);
}

static double ratio(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    int T = la + lb;
    if (T == 0) return 1.0;
    int M = match_count(a, b, 0, la, 0, lb);
    return 2.0 * M / T;
}

typedef struct { double r; char* name; char* pass; } Hit;

static int hit_cmp(const void* x, const void* y) {
    double rx = ((const Hit*)x)->r, ry = ((const Hit*)y)->r;
    if (rx < ry) return 1;
    if (rx > ry) return -1;
    return 0;
}

int main(void) {
    printf("Welcome to our Password Manager Version 1\n");
    printf("Would you like to add a password or view one?\n");

    int reset = 1;
    while (reset == 1) {
        printf("Type 1 to add a password\n");
        printf("Type 2 to view a password\n");

        char* response = in_prompt("> ");

        if (strcmp(response, "1") == 0) {
            int exists = file_exists(FILE_NAME);

            printf("please input the name of the password you would like to put\n");
            char* name = in_line();
            if (exists)
                printf("please input the password you would like to add that will be saved with %s\n", name);
            else
                printf("please input the password you would like to add that will be saved with the name %s\n", name);
            char* password = in_line();

            int next_id = 1;
            if (exists) {
                FILE* rf = fopen(FILE_NAME, "r");
                if (rf) {
                    char* line = NULL; size_t cap = 0;
                    while (getline(&line, &cap, rf) >= 0) {
                        char* s = strip_inplace(line);
                        if (*s == 0) continue;
                        char* parts[16];
                        split_pipe(s, parts, 16);
                        next_id = atoi(parts[0]) + 1;
                    }
                    free(line);
                    fclose(rf);
                }
            }

            FILE* f = fopen(FILE_NAME, "a");
            printf("%d|%s|%s\n", next_id, name, password);
            fprintf(f, "%d|%s|%s\n", next_id, name, password);
            fclose(f);

            printf("Adding a password...\n");
            printf("would you like to add or view another password\n");
            printf("if yes say 1 if no say 2 or another number\n");
            char* resetq = in_line();
            reset = (strcmp(resetq, "1") == 0) ? 1 : 0;

            free(name); free(password); free(resetq);
        }
        else if (strcmp(response, "2") == 0) {
            if (!file_exists(FILE_NAME)) {
                printf("No passwords saved yet. Add one first.\n");
            } else {
                int go_again = 1;
                while (go_again == 1) {
                    printf("what would you like to view type 1 if you would like to view a specific password or 2 if you would like to view all your passwords\n");
                    char* awsr = in_line();

                    if (strcmp(awsr, "1") == 0) {
                        int a = 1;
                        while (a == 1) {
                            printf("would you like to input a number that is in order of which password you added or search for name of the password\n");
                            printf("press 1 for search by order of adding (search by id) or 2 for name\n");
                            char* pon = in_line();

                            if (strcmp(pon, "1") == 0) {
                                printf("enter ID\n");
                                char* searchpassword = in_line();
                                FILE* rf = fopen(FILE_NAME, "r");
                                char* line = NULL; size_t cap = 0;
                                int found = 0;
                                while (rf && getline(&line, &cap, rf) >= 0) {
                                    char* s = strip_inplace(line);
                                    if (*s == 0) continue;
                                    char* parts[16];
                                    split_pipe(s, parts, 16);
                                    if (strcmp(parts[0], searchpassword) == 0) {
                                        printf("Name: %s\n", parts[1]);
                                        printf("Password: %s\n", parts[2]);
                                        found = 1;
                                        break;
                                    }
                                }
                                free(line);
                                if (rf) fclose(rf);
                                if (!found) printf("No password found with that ID.\n");
                                free(searchpassword);
                                a = 0;
                            }
                            else if (strcmp(pon, "2") == 0) {
                                printf("enter name to search\n");
                                char* raw = in_line();
                                char* search_name = str_lower_dup(raw);
                                free(raw);

                                FILE* rf = fopen(FILE_NAME, "r");
                                char* line = NULL; size_t cap = 0;
                                Hit* hits = NULL; size_t nhits = 0, hcap = 0;
                                while (rf && getline(&line, &cap, rf) >= 0) {
                                    char* s = strip_inplace(line);
                                    if (*s == 0) continue;
                                    char* parts[16];
                                    split_pipe(s, parts, 16);
                                    char* stored = str_lower_dup(parts[1]);
                                    double r = ratio(search_name, stored);
                                    if (strstr(stored, search_name) != NULL || r >= 0.6) {
                                        if (nhits == hcap) {
                                            hcap = hcap ? hcap * 2 : 8;
                                            hits = realloc(hits, hcap * sizeof(Hit));
                                        }
                                        hits[nhits].r = r;
                                        hits[nhits].name = strdup(parts[1]);
                                        hits[nhits].pass = strdup(parts[2]);
                                        nhits++;
                                    }
                                    free(stored);
                                }
                                free(line);
                                if (rf) fclose(rf);

                                qsort(hits, nhits, sizeof(Hit), hit_cmp);
                                if (nhits) {
                                    printf("Found %zu match(es):\n", nhits);
                                    for (size_t i = 0; i < nhits; i++)
                                        printf("  Name: %s  |  Password: %s\n",
                                               hits[i].name, hits[i].pass);
                                } else {
                                    printf("No matches found.\n");
                                }
                                for (size_t i = 0; i < nhits; i++) {
                                    free(hits[i].name);
                                    free(hits[i].pass);
                                }
                                free(hits);
                                free(search_name);
                                a = 0;
                            }
                            else {
                                a = 1;
                            }
                            free(pon);
                        }
                        go_again = 0;
                    }
                    else if (strcmp(awsr, "2") == 0) {
                        FILE* rf = fopen(FILE_NAME, "r");
                        char** lines = NULL; size_t nlines = 0, lcap = 0;
                        char* line = NULL; size_t cap = 0;
                        while (rf && getline(&line, &cap, rf) >= 0) {
                            char* s = strip_inplace(line);
                            if (*s == 0) continue;
                            char* dup = strdup(s);
                            if (nlines == lcap) {
                                lcap = lcap ? lcap * 2 : 8;
                                lines = realloc(lines, lcap * sizeof(char*));
                            }
                            lines[nlines++] = dup;
                        }
                        free(line);
                        if (rf) fclose(rf);

                        if (nlines == 0) {
                            printf("No passwords saved yet.\n");
                        } else {
                            printf("\n--- All Passwords (%zu total) ---\n", nlines);
                            for (size_t i = 0; i < nlines; i++) {
                                char* parts[16];
                                split_pipe(lines[i], parts, 16);
                                printf("  ID: %s  |  Name: %s  |  Password: %s\n",
                                       parts[0], parts[1], parts[2]);
                            }
                            printf("-----------------------------------\n\n");
                        }
                        for (size_t i = 0; i < nlines; i++) free(lines[i]);
                        free(lines);
                        go_again = 0;
                    }
                    else {
                        printf("invalid answer\n");
                    }
                    free(awsr);
                }
                printf("Viewing a password...\n");
            }
            reset = 0;
        }
        else {
            printf("Please enter either 1 or 2.\n");
        }
        free(response);
    }
    return 0;
}
