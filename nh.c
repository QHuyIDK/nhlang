#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 100
#define MAX_LINE 256
#define MAX_LINES 1000

typedef enum { TYPE_INT, TYPE_STR } VarType;

typedef struct {
    char name[32];
    VarType type;
    char str_val[128];
    int int_val;
} Variable;

Variable vars[MAX_VARS];
int var_count = 0;

Variable* find_var(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) return &vars[i];
    }
    return NULL;
}

void set_var(const char* name, const char* val) {
    Variable* v = find_var(name);
    if (!v) {
        if (var_count >= MAX_VARS) {
            printf("Too many variables!\n");
            exit(1);
        }
        v = &vars[var_count++];
        strcpy(v->name, name);
    }
    int is_num = 1;
    int len = strlen(val);
    if (len == 0) is_num = 0;
    else {
        for (int i = 0; i < len; i++) {
            if (!isdigit(val[i]) && !(i==0 && val[i]=='-')) {
                is_num = 0;
                break;
            }
        }
    }
    if (is_num) {
        v->type = TYPE_INT;
        v->int_val = atoi(val);
        v->str_val[0] = 0;
    } else {
        v->type = TYPE_STR;
        strncpy(v->str_val, val, 127);
        v->str_val[127] = 0;
        v->int_val = 0;
    }
}

Variable* get_var(const char* name) {
    return find_var(name);
}

char* trim_leading_spaces(char* line) {
    while (*line == ' ' || *line == '\t') line++;
    return line;
}

char* next_token(char** line_ptr) {
    char* p = *line_ptr;
    while (*p && isspace(*p)) p++;
    if (*p == 0) return NULL;

    char token[256];
    int i = 0;

    if (*p == '"') {
        p++;
        while (*p && *p != '"' && i < 255) {
            token[i++] = *p++;
        }
        token[i] = 0;
        if (*p == '"') p++;
    } else {
        while (*p && !isspace(*p) && i < 255) {
            token[i++] = *p++;
        }
        token[i] = 0;
    }

    *line_ptr = p;
    return strdup(token);
}

// Kiểm tra chuỗi là số nguyên
int is_integer(const char* s) {
    if (!s || *s == 0) return 0;
    int i = 0;
    if (s[0] == '-') i = 1;
    for (; s[i]; i++) {
        if (!isdigit(s[i])) return 0;
    }
    return 1;
}

// Tách tokens cho print, xử lý dấu + và chuỗi ""
char** split_print_expression(char* expr, int* count) {
    char** tokens = malloc(sizeof(char*) * 100);
    *count = 0;

    char* p = expr;
    while (*p) {
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        if (*p == '"') {
            p++;
            char buf[256];
            int i=0;
            while (*p && *p != '"' && i<255) {
                buf[i++] = *p++;
            }
            buf[i] = 0;
            if (*p == '"') p++;
            tokens[(*count)++] = strdup(buf);
        } else if (*p == '+') {
            tokens[(*count)++] = strdup("+");
            p++;
        } else {
            char buf[256];
            int i=0;
            while (*p && !isspace(*p) && *p != '+') {
                buf[i++] = *p++;
            }
            buf[i] = 0;
            tokens[(*count)++] = strdup(buf);
        }
    }
    return tokens;
}

void free_tokens(char** tokens, int count) {
    for (int i=0; i<count; i++) free(tokens[i]);
    free(tokens);
}

// Nối chuỗi hoặc cộng số
// Trả về 1 nếu thành công, 0 nếu lỗi kiểu dữ liệu
int eval_print_tokens(char** tokens, int count) {
    int expect_operand = 1; // true khi chờ toán hạng
    VarType cur_type = TYPE_INT; // lưu kiểu hiện tại (để check kiểu hợp lệ)
    int int_result = 0;
    char str_result[4096] = "";
    int have_result = 0;

    for (int i=0; i<count; i++) {
        char* t = tokens[i];

        if (expect_operand) {
            if (strcmp(t, "+") == 0) {
                printf("Syntax error: '+' unexpected\n");
                return 0;
            }
            // Lấy giá trị toán hạng
            Variable* v = get_var(t);
            if (v) {
                if (!have_result) {
                    cur_type = v->type;
                    if (cur_type == TYPE_INT) int_result = v->int_val;
                    else strcpy(str_result, v->str_val);
                    have_result = 1;
                } else {
                    if (cur_type != v->type) {
                        printf("Type error: mixing int and string\n");
                        return 0;
                    }
                    if (cur_type == TYPE_INT) int_result += v->int_val;
                    else {
                        strcat(str_result, v->str_val);
                    }
                }
            } else {
                // Là literal
                if (is_integer(t)) {
                    int val = atoi(t);
                    if (!have_result) {
                        cur_type = TYPE_INT;
                        int_result = val;
                        have_result = 1;
                    } else {
                        if (cur_type != TYPE_INT) {
                            printf("Type error: mixing int and string\n");
                            return 0;
                        }
                        int_result += val;
                    }
                } else {
                    // Chuỗi literal
                    if (!have_result) {
                        cur_type = TYPE_STR;
                        strcpy(str_result, t);
                        have_result = 1;
                    } else {
                        if (cur_type != TYPE_STR) {
                            printf("Type error: mixing int and string\n");
                            return 0;
                        }
                        strcat(str_result, t);
                    }
                }
            }
            expect_operand = 0;
        } else {
            if (strcmp(t, "+") != 0) {
                printf("Syntax error: expected '+' but got '%s'\n", t);
                return 0;
            }
            expect_operand = 1;
        }
    }
    if (expect_operand) {
        printf("Syntax error: expression ends with '+'\n");
        return 0;
    }
    if (!have_result) return 0;

    if (cur_type == TYPE_INT) {
        printf("%d\n", int_result);
    } else {
        printf("%s\n", str_result);
    }
    return 1;
}

int compare_int(int a, const char* op, int b) {
    if (strcmp(op, "==") == 0) return a == b;
    if (strcmp(op, "!=") == 0) return a != b;
    if (strcmp(op, ">") == 0) return a > b;
    if (strcmp(op, "<") == 0) return a < b;
    if (strcmp(op, ">=") == 0) return a >= b;
    if (strcmp(op, "<=") == 0) return a <= b;
    return 0;
}

int eval_condition(char* var_name, char* op, char* val) {
    Variable* v = get_var(var_name);
    if (!v) return 0;

    int cmp_val = atoi(val);
    if (v->type == TYPE_INT) {
        return compare_int(v->int_val, op, cmp_val);
    } else {
        if (strcmp(op, "==") == 0) return strcmp(v->str_val, val) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(v->str_val, val) != 0;
        return 0;
    }
}

void run_lines(char lines[][MAX_LINE], int start, int end);

void run_loop(char lines[][MAX_LINE], int start, int end, int times) {
    for (int i = 0; i < times; i++) {
        run_lines(lines, start, end);
    }
}

void run_lines(char lines[][MAX_LINE], int start, int end) {
    for (int i = start; i < end; i++) {
        char *line_raw = lines[i];
        char *line = trim_leading_spaces(line_raw);

        // Bỏ qua comment (#) và dòng trống
        if (line[0] == '#' || strlen(line) == 0) continue;

        if (strncmp(line, "set ", 4) == 0) {
            char *p = line + 4;
            char* var = next_token(&p);
            char* val = next_token(&p);
            if (!var || !val) {
                printf("Syntax error in set\n");
                exit(1);
            }
            set_var(var, val);
            free(var);
            free(val);
        } else if (strncmp(line, "print ", 6) == 0) {
            char *p = line + 6;
            int count;
            char** tokens = split_print_expression(p, &count);
            if (!eval_print_tokens(tokens, count)) {
                printf("Error evaluating print expression\n");
                free_tokens(tokens, count);
                exit(1);
            }
            free_tokens(tokens, count);
        } else if (strncmp(line, "input ", 6) == 0) {
            char *p = line + 6;
            char* var = next_token(&p);
            if (!var) {
                printf("Syntax error in input\n");
                exit(1);
            }
            printf("Input %s: ", var);
            char input_buf[128];
            if (!fgets(input_buf, sizeof(input_buf), stdin)) {
                printf("Input error\n");
                free(var);
                exit(1);
            }
            // Xóa newline cuối
            input_buf[strcspn(input_buf, "\n")] = 0;
            set_var(var, input_buf);
            free(var);
        } else if (strncmp(line, "if ", 3) == 0) {
            char *p = line + 3;
            char* var = next_token(&p);
            char* op = next_token(&p);
            char* val = next_token(&p);
            if (!var || !op || !val) {
                printf("Syntax error in if\n");
                exit(1);
            }
            // Tìm dòng end tương ứng
            int j = i + 1;
            int nested = 0;
            for (; j < end; j++) {
                char *l = trim_leading_spaces(lines[j]);
                if (strncmp(l, "if ", 3) == 0) nested++;
                else if (strcmp(l, "end") == 0) {
                    if (nested == 0) break;
                    nested--;
                }
            }
            if (j == end) {
                printf("Missing end for if\n");
                exit(1);
            }
            if (eval_condition(var, op, val)) {
                run_lines(lines, i+1, j);
            }
            i = j; // nhảy qua end
            free(var);
            free(op);
            free(val);
        } else if (strncmp(line, "loop ", 5) == 0) {
            char *p = line + 5;
            char* times_str = next_token(&p);
            if (!times_str || !is_integer(times_str)) {
                printf("Syntax error in loop\n");
                exit(1);
            }
            int times = atoi(times_str);
            free(times_str);
            // Tìm end tương ứng
            int j = i + 1;
            int nested = 0;
            for (; j < end; j++) {
                char *l = trim_leading_spaces(lines[j]);
                if (strncmp(l, "loop ", 5) == 0) nested++;
                else if (strcmp(l, "end") == 0) {
                    if (nested == 0) break;
                    nested--;
                }
            }
            if (j == end) {
                printf("Missing end for loop\n");
                exit(1);
            }
            run_loop(lines, i+1, j, times);
            i = j; // nhảy qua end
        } else if (strcmp(line, "end") == 0) {
            // end xử lý bởi if, loop nên bỏ qua ở đây
            continue;
        } else {
            printf("Unknown command: %s\n", line);
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: nh <filename>\n");
        return 1;
    }
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        printf("Cannot open file %s\n", argv[1]);
        return 1;
    }
    char lines[MAX_LINES][MAX_LINE];
    int line_count = 0;
    while (fgets(lines[line_count], MAX_LINE, f) && line_count < MAX_LINES) {
        // Xóa newline
        lines[line_count][strcspn(lines[line_count], "\n")] = 0;
        line_count++;
    }
    fclose(f);

    run_lines(lines, 0, line_count);

    return 0;
}
