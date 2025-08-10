#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 100
#define MAX_LINE 256

typedef enum { TYPE_INT, TYPE_STR } VarType;

typedef struct {
    char name[32];
    char value[128];
    VarType type;
} Variable;

Variable vars[MAX_VARS];
int var_count = 0;

Variable* get_var(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0)
            return &vars[i];
    }
    return NULL;
}

int is_number(const char* s) {
    if (*s == '-' || *s == '+') s++;
    if (!*s) return 0;
    while (*s) {
        if (!isdigit(*s)) return 0;
        s++;
    }
    return 1;
}

void set_var(const char* name, const char* val) {
    Variable* v = get_var(name);
    if (!v) {
        if (var_count >= MAX_VARS) {
            printf("Error: too many variables\n");
            exit(1);
        }
        v = &vars[var_count++];
        strncpy(v->name, name, 31);
        v->name[31] = 0;
    }

    if (is_number(val)) {
        v->type = TYPE_INT;
        strncpy(v->value, val, 127);
    } else {
        v->type = TYPE_STR;
        strncpy(v->value, val, 127);
    }
    v->value[127] = 0;
}

const char* get_val(const char* token) {
    Variable* v = get_var(token);
    if (v) return v->value;
    else return token;
}

int to_int(const char* s) {
    return atoi(s);
}

int compare_values(const char* a, const char* op, const char* b) {
    Variable* va = get_var(a);
    Variable* vb = get_var(b);

    int a_is_num = 0;
    int b_is_num = 0;
    int a_num = 0;
    int b_num = 0;

    if (va) {
        a_is_num = (va->type == TYPE_INT);
        if (a_is_num) a_num = to_int(va->value);
    } else {
        a_is_num = is_number(a);
        if (a_is_num) a_num = to_int(a);
    }

    if (vb) {
        b_is_num = (vb->type == TYPE_INT);
        if (b_is_num) b_num = to_int(vb->value);
    } else {
        b_is_num = is_number(b);
        if (b_is_num) b_num = to_int(b);
    }

    if (a_is_num && b_is_num) {
        if (strcmp(op, "==") == 0) return a_num == b_num;
        if (strcmp(op, "!=") == 0) return a_num != b_num;
        if (strcmp(op, ">") == 0) return a_num > b_num;
        if (strcmp(op, "<") == 0) return a_num < b_num;
        if (strcmp(op, ">=") == 0) return a_num >= b_num;
        if (strcmp(op, "<=") == 0) return a_num <= b_num;
    } else {
        // Compare strings lex
        const char* aval = va ? va->value : a;
        const char* bval = vb ? vb->value : b;
        if (strcmp(op, "==") == 0) return strcmp(aval, bval) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(aval, bval) != 0;
        if (strcmp(op, ">") == 0) return strcmp(aval, bval) > 0;
        if (strcmp(op, "<") == 0) return strcmp(aval, bval) < 0;
        if (strcmp(op, ">=") == 0) return strcmp(aval, bval) >= 0;
        if (strcmp(op, "<=") == 0) return strcmp(aval, bval) <= 0;
    }
    return 0;
}

int find_end(char lines[][MAX_LINE], int start, int total_lines) {
    int depth = 1;
    for (int i = start; i < total_lines; i++) {
        if (strncmp(lines[i], "if ", 3) == 0 || strncmp(lines[i], "loop ", 5) == 0) depth++;
        else if (strcmp(lines[i], "end") == 0) {
            depth--;
            if (depth == 0) return i;
        }
    }
    return -1;
}

void run_lines(char lines[][MAX_LINE], int start, int end) {
    for (int i = start; i < end; i++) {
        char *line = lines[i];
        if (strlen(line) == 0 || line[0] == '#') continue;

        if (strncmp(line, "set ", 4) == 0) {
            char var[32], val[128];
            if (sscanf(line+4, "%31s %127[^\n]", var, val) == 2) {
                int len = strlen(val);
                if (val[0] == '"' && val[len-1] == '"') {
                    memmove(val, val+1, len-2);
                    val[len-2] = 0;
                }
                set_var(var, val);
            } else {
                printf("Syntax error in set\n");
                exit(1);
            }
        }
        else if (strncmp(line, "print ", 6) == 0) {
            char *p = line + 6;
            char *token = strtok(p, " ");
            while (token) {
                Variable* v = get_var(token);
                if (v) {
                    if (v->type == TYPE_INT) printf("%s ", v->value);
                    else printf("%s ", v->value);
                } else {
                    // In nguyên chuỗi nếu có ""
                    int len = strlen(token);
                    if (token[0] == '"' && token[len-1] == '"') {
                        printf("%.*s ", len-2, token+1);
                    } else {
                        printf("%s ", token);
                    }
                }
                token = strtok(NULL, " ");
            }
            printf("\n");
        }
        else if (strncmp(line, "input ", 6) == 0) {
            char var[32];
            if (sscanf(line+6, "%31s", var) == 1) {
                printf("> ");
                fflush(stdout);
                char buf[128];
                if (fgets(buf, 128, stdin)) {
                    buf[strcspn(buf, "\n")] = 0;
                    set_var(var, buf);
                }
            } else {
                printf("Syntax error in input\n");
                exit(1);
            }
        }
        else if (strncmp(line, "if ", 3) == 0) {
            char var[32], op[3], val[128];
            if (sscanf(line+3, "%31s %2s %127[^\n]", var, op, val) == 3) {
                int len = strlen(val);
                if (val[0] == '"' && val[len-1] == '"') {
                    memmove(val, val+1, len-2);
                    val[len-2] = 0;
                }
                if (compare_values(var, op, val)) {
                    int block_end = find_end(lines, i+1, end);
                    if (block_end == -1) {
                        printf("Error: missing end\n");
                        exit(1);
                    }
                    run_lines(lines, i+1, block_end);
                    i = block_end;
                } else {
                    int block_end = find_end(lines, i+1, end);
                    if (block_end == -1) {
                        printf("Error: missing end\n");
                        exit(1);
                    }
                    i = block_end;
                }
            } else {
                printf("Syntax error in if\n");
                exit(1);
            }
        }
        else if (strncmp(line, "loop ", 5) == 0) {
            int count = 0;
            if (sscanf(line+5, "%d", &count) == 1 && count > 0) {
                int block_end = find_end(lines, i+1, end);
                if (block_end == -1) {
                    printf("Error: missing end\n");
                    exit(1);
                }
                for (int c = 0; c < count; c++) {
                    run_lines(lines, i+1, block_end);
                }
                i = block_end;
            } else {
                printf("Syntax error in loop\n");
                exit(1);
            }
        }
        else if (strcmp(line, "end") == 0) {
            return;
        }
        else {
            printf("Unknown command: %s\n", line);
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: nh <file.nh>\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        printf("Cannot open file: %s\n", argv[1]);
        return 1;
    }

    char lines[1000][MAX_LINE];
    int line_count = 0;

    while (fgets(lines[line_count], MAX_LINE, f)) {
        // Remove trailing newline
        lines[line_count][strcspn(lines[line_count], "\n")] = 0;
        line_count++;
        if (line_count >= 1000) break;
    }
    fclose(f);

    run_lines(lines, 0, line_count);

    return 0;
}
