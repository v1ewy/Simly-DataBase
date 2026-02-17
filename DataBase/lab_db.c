#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define FIELD_COUNT 7
#define MAX_STATUS 5

//enum for a status
typedef enum {
    well,
    wearlow,
    wearhigh,
    broken,
    notcheck
} Status;

// structure for a date
typedef struct {
    int day;
    int month;
    int year;
} Date;

// the basic structure of the database
typedef struct Node {
    int unit_id;
    char unit_model[256];
    char carnum[256];
    Date chk_date;
    Status status;
    char mechanic[256];
    char driver[256];
    struct Node* next;
} Node;

typedef struct Queue {
    struct Node* head;
    struct Node* tail;
    int size;
} Queue;

typedef enum {
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_IN,
    OP_NOT_IN
} Operator;

typedef struct {
    int field;
    Operator op;

    union {
        int i;
        char str[256];
        char carnum[256];
        Date date;
        struct {
            Status list[MAX_STATUS];
            int count;
        } status;
    } value;

} Condition;

typedef struct {
    int field;
    union {
        int i;
        char str[256];
        char carnum[256];
        Date date;
        Status status;
    } value;
} Update;

typedef struct {
    int field;
    enum {
        ORDER_ASC,
        ORDER_DESC
    } order;
} SortKey;


// array with the names of the arguments
const char* field_names[FIELD_COUNT] = {
    "unit_id",
    "unit_model",
    "car_id",
    "chk_date",
    "status",
    "mechanic",
    "driver"
};


void init_queue (struct Queue* queue) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}

// function of removing spaces
char* trim(char* s)
{
    while (*s == ' ')
        s++;

    char* end = s + strlen(s) - 1;

    while (end > s && *end == ' ') {
        *end = '\0';
        end--;
    }

    return s;
}

// function of parsing int
int parse_int(const char* s, int* out) {
    char* end;
    long val = strtol(s, &end, 10);

    if (*s == '\0' || *end != '\0')
        return 0;

    *out = (int)val;
    return 1;
}

// function of parsing string with double quotes
int parse_double_quoted_string(char* value, char* dest, size_t max)
{
    size_t len = strlen(value);

    if (len < 2)
        return 0;

    if (value[0] != '"' || value[len - 1] != '"')
        return 0;

    value[len - 1] = '\0';
    value++;

    if (len >= max)
        return 0;

    strcpy(dest, value);

    return 1;
}

// letter verification function for carnum
int valid_letter(char c) {
    const char* allowed = "ABCEHKMOPTXY";
    return strchr(allowed, c) != NULL;
}

// function of parsing carnum
int parse_carnum(char* value, char* dest, size_t max) {
    size_t len = strlen(value);

    if (len < 2 || value[0] != '\'' || value[len - 1] != '\'')
        return 0;

    value[len - 1] = '\0';
    value++;

    len = strlen(value);

    if (len != 8 && len != 9)
        return 0;

    if (!valid_letter(value[0]))
        return 0;

    for (int i = 1; i <= 3; i++)
        if (!isdigit(value[i]))
            return 0;

    if (!valid_letter(value[4]) || !valid_letter(value[5]))
        return 0;

    for (int i = 6; i < len; i++)
        if (!isdigit(value[i]))
            return 0;

    if (len >= max)
        return 0;

    strcpy(dest, value);

    return 1;
}

// cheaking leap year
int is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

// checking day
int days_in_month(int m, int y) {
    static int d[] = {
        31,28,31,30,31,30,
        31,31,30,31,30,31
    };

    if (m == 2 && is_leap(y))
        return 29;

    return d[m - 1];
}

// function of parsing date
int parse_date(char* value, Date* out) {
    size_t len = strlen(value);

    if (len < 8)
        return 0;

    if (value[0] != '\'' || value[len - 1] != '\'')
        return 0;

    value[len - 1] = '\0';
    value++;

    int d, m, y;

    if (sscanf(value, "%d.%d.%d", &d, &m, &y) != 3)
        return 0;

    if (y < 1000 || y > 2026)
        return 0;

    if (m < 1 || m > 12)
        return 0;

    int maxd = days_in_month(m, y);

    if (d < 1 || d > maxd)
        return 0;

    out->day = d;
    out->month = m;
    out->year = y;

    return 1;
}

// function of parsing status
int parse_status(char* value, Status* out) {
    size_t len = strlen(value);

    if (len < 2)
        return 0;

    if (value[0] != '\'' || value[len - 1] != '\'')
        return 0;

    value[len - 1] = '\0';
    value++;

    if (strcmp(value, "well") == 0)
        *out = well;
    else if (strcmp(value, "wearlow") == 0)
        *out = wearlow;
    else if (strcmp(value, "wearhigh") == 0)
        *out = wearhigh;
    else if (strcmp(value, "broken") == 0)
        *out = broken;
    else if (strcmp(value, "notcheck") == 0)
        *out = notcheck;
    else
        return 0;

    return 1;
}


const char* status_to_string(Status s) {
    switch (s) {
        case well: return "'well'";
        case wearlow: return "'wearlow'";
        case wearhigh: return "'wearhigh'";
        case broken: return "'broken'";
        case notcheck: return "'notcheck'";
    }
    return "'unknown'";
}

char* next_token(char** src, char sep)
{
    if (!*src || !**src)
        return NULL;

    char* start = *src;
    char* p = start;

    int in_quotes = 0;

    while (*p) {
        if (*p == '\"')
            in_quotes = !in_quotes;

        else if (*p == sep && !in_quotes)
            break;

        p++;
    }

    if (*p == ',') {
        *p = '\0';
        *src = p + 1;
    } else {
        *src = NULL;
    }

    return start;
}


// function insert
void insert_db(char* line, FILE* output, Queue* queue) {
    Node* new_node = malloc(sizeof(Node));
    char* args = line + 6;
    char* copy = trim(strdup(args));

    if (*args == '\0') goto error;
    if (!copy) goto error;

    char* seen[FIELD_COUNT] = {0};
    
    char* token;
    
    while ((token = next_token(&copy, ','))) {

        char* eq = strchr(token, '=');
        if (!eq) goto error;

        *eq = '\0';
        char* field = trim(token);
        char* value = trim(eq + 1);

        if (*field == '\0' || *value == '\0')
            goto error;

        int found = 0;

        for (int i = 0; i < FIELD_COUNT; i++) {
            if (strcmp(field, field_names[i]) == 0) {
                if (seen[i]) goto error;
                seen[i] = value;
                found = 1;
                break;
            }
        }

        if (!found)
            goto error;
    }

    for (int i = 0; i < FIELD_COUNT; i++)
        if (!seen[i]) goto error;

    
    if (!parse_int(seen[0], &new_node->unit_id)) {
        goto error;
    }
    if (!parse_double_quoted_string(seen[1], new_node->unit_model, sizeof(new_node->unit_model))) {
        goto error;
    }
    if (!parse_carnum(seen[2], new_node->carnum, sizeof(new_node->carnum))) {
        goto error;
    }
    if (!parse_date(seen[3], &new_node->chk_date)) {
        goto error;
    }
    if (!parse_status(seen[4], &new_node->status)) {
        goto error;
    }
    if (!parse_double_quoted_string(seen[5], new_node->mechanic, sizeof(new_node->mechanic))) {
        goto error;
    }
    if (!parse_double_quoted_string(seen[6], new_node->driver, sizeof(new_node->driver))) {
        goto error;
    }
    new_node->next = NULL;

    if (queue->head == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        (*queue->tail).next = new_node;
        queue->tail = new_node;
    }

    fprintf(output, "insert:%d\n", ++queue->size);
    
    free(copy);
    return;

error:
    fprintf(output, "incorrect:'%.20s'\n", line);
    free(copy);
    free(new_node);
}


int parse_field_list(char* str, int** fields, int* count) {
    *fields = NULL;
    *count = 0;
    
    char* token;

    while ((token = next_token(&str, ','))) {

        token = trim(token);

        int field = -1;

        for (int i = 0; i < FIELD_COUNT; i++) {
            if (strcmp(token, field_names[i]) == 0) {
                field = i;
                break;
            }
        }

        if (field == -1)
            return 0;

        int* tmp = realloc(*fields, (*count + 1) * sizeof(int));

        if (!tmp)
            return 0;

        *fields = tmp;
        (*fields)[*count] = field;

        (*count)++;
    }

    return *count > 0;
}



void print_field(FILE* out, Node* n, int field) {
    switch (field) {

    case 0:
        fprintf(out, "unit_id=%d", n->unit_id);
        break;

    case 1:
        fprintf(out, "unit_model=\"%s\"", n->unit_model);
        break;

    case 2:
        fprintf(out, "car_id='%s'", n->carnum);
        break;

    case 3:
        fprintf(out, "chk_date='%02d.%02d.%d'",
                n->chk_date.day, n->chk_date.month, n->chk_date.year);
        break;

    case 4:
        fprintf(out, "status=%s",
                status_to_string(n->status));
        break;

    case 5:
        fprintf(out, "mechanic=\"%s\"", n->mechanic);
        break;

    case 6:
        fprintf(out, "driver=\"%s\"", n->driver);
        break;
    }
}

Operator parse_operator(char** p) {
    char* s = *p;

    if (strncmp(s, "/not_in/", 8) == 0) {
        **p = '\0';
        *p += 8;
        return OP_NOT_IN;
    }

    if (strncmp(s, "/in/", 4) == 0) {
        **p = '\0';
        *p += 4;
        return OP_IN;
    }

    if (strncmp(s, ">=", 2) == 0) {
        **p = '\0';
        *p += 2;
        return OP_GE;
    }

    if (strncmp(s, "<=", 2) == 0) {
        **p = '\0';
        *p += 2;
        return OP_LE;
    }

    if (strncmp(s, "==", 2) == 0) {
        **p = '\0';
        *p += 2;
        return OP_EQ;
    }

    if (strncmp(s, "!=", 2) == 0) {
        **p = '\0';
        *p += 2;
        return OP_NE;
    }

    if (*s == '>') {
        **p = '\0';
        (*p)++;
        return OP_GT;
    }

    if (*s == '<') {
        **p = '\0';
        (*p)++;
        return OP_LT;
    }

    return -1;
}

int parse_condition_value(Condition* c, char* value) {
    switch (c->field) {
        case 0:
            return parse_int(value, &c->value.i);

        case 1:
            return parse_double_quoted_string(value, c->value.str, sizeof(c->value.str));
            
        case 2:
            return parse_carnum(value, c->value.carnum, sizeof(c->value.carnum));

        case 3:
            return parse_date(value, &c->value.date);

        case 4:
            if (value[0] == '[') {
                size_t len = strlen(value);
                value[len-1] = '\0';
                value++;

                c->value.status.count = 0;
                
                if (*value == '\0')
                    return 1;
                
                char* token;
                while ((token = next_token(&value, ','))) {
                    if (c->value.status.count >= MAX_STATUS)
                         return 0;
                    
                    token = trim(token);
                    if (!parse_status(token, &c->value.status.list[c->value.status.count])) return 0;
                    c->value.status.count++;
                }
                return c->value.status.count > 0;;
            } else {
                c->value.status.count = 1;
                return parse_status(value, &c->value.status.list[0]);
            }
        case 5:
        case 6:
            return parse_double_quoted_string(value, c->value.str, sizeof(c->value.str));
    }

    return 0;
}


int parse_conditions(char* cond_str, Condition** conds, int* count) {
    *conds = NULL;
    *count = 0;

    char* token;

    while ((token = next_token(&cond_str, ' '))) {
        Condition* tmp = realloc(*conds, (*count + 1) * sizeof(Condition));

        if (!tmp)
            return 0;

        *conds = tmp;

        Condition* c = &(*conds)[*count];

        char* field = token;

        while (*token && !strchr("=!<>/", *token))
            token++;

        c->op = parse_operator(&token);
        if (c->op == -1)
            return 0;

        field = trim(field);

        c->field = -1;

        for (int i = 0; i < FIELD_COUNT; i++) {
            if (strcmp(field, field_names[i]) == 0) {
                c->field = i;
                break;
            }
        }

        if (c->field == -1)
            return 0;

        char* value = trim(token);

        if (!parse_condition_value(c, value))
            return 0;

        (*count)++;
    }

    return 1;
}

int cmp_int(int a, int b, Operator op) {
    switch (op) {
        case OP_EQ: return a == b;
        case OP_NE: return a != b;
        case OP_LT: return a < b;
        case OP_LE: return a <= b;
        case OP_GT: return a > b;
        case OP_GE: return a >= b;
        default: return 0;
    }
}


int cmp_str(const char* a, const char* b, Operator op) {
    int r = strcmp(a, b);

    switch (op) {
        case OP_EQ: return r == 0;
        case OP_NE: return r != 0;
        case OP_LT: return r < 0;
        case OP_LE: return r <= 0;
        case OP_GT: return r > 0;
        case OP_GE: return r >= 0;
        default: return 0;
    }
}

int carnum_digits(const char* s, int start, int len) {
    int v = 0;

    for (int i = 0; i < len; i++)
        v = v * 10 + (s[start + i] - '0');

    return v;
}

void carnum_letters(const char* s, char out[4]) {
    out[0] = s[0];
    out[1] = s[4];
    out[2] = s[5];
    out[3] = '\0';
}

int cmp_carnum(const char* a, const char* b, Operator op) {
    int numA = carnum_digits(a, 1, 3);
    int numB = carnum_digits(b, 1, 3);

    if (numA != numB)
        return cmp_int(numA, numB, op);

    char lettersA[4];
    char lettersB[4];

    carnum_letters(a, lettersA);
    carnum_letters(b, lettersB);

    int cmp = strcmp(lettersA, lettersB);

    if (cmp != 0)
        return cmp_str(lettersA, lettersB, op);

    int lenA = (int)strlen(a);
    int lenB = (int)strlen(b);

    int regA = carnum_digits(a, 6, lenA - 6);
    int regB = carnum_digits(b, 6, lenB - 6);

    return cmp_int(regA, regB, op);
}

int date_to_int(Date d) {
    return d.year * 10000 + d.month * 100 + d.day;
}

int cmp_date(Date a, Date b, Operator op) {
    return cmp_int(date_to_int(a), date_to_int(b), op);
}

int status_in(Status s, Status* list, int count) {
    for (int i = 0; i < count; i++)
        if (list[i] == s)
            return 1;

    return 0;
}

int check_condition(Node* n, Condition* c) {
    switch (c->field) {
        case 0:
            return cmp_int(n->unit_id, c->value.i, c->op);

        case 1:
            return cmp_str(n->unit_model, c->value.str, c->op);

        case 2:
            return cmp_carnum(n->carnum, c->value.carnum, c->op);

        case 3:
            return cmp_date(n->chk_date, c->value.date, c->op);

        case 4: {
            Status s = n->status;
            
            if (c->value.status.count == 0) {

                if (c->op == OP_IN)
                    return 0;

                if (c->op == OP_NOT_IN)
                    return 1;
            }
            
            if (c->op == OP_IN)
                return status_in(s, c->value.status.list, c->value.status.count);
            
            if (c->op == OP_NOT_IN)
                return !status_in(s, c->value.status.list, c->value.status.count);
            
            return cmp_int(s, c->value.status.list[0], c->op);
        }
        case 5:
            return cmp_str(n->mechanic, c->value.str, c->op);
            
        case 6:
            return cmp_str(n->driver, c->value.str, c->op);
    }

    return 0;
}



int check_conditions(Node* n, Condition* conds, int count) {
    for (int i = 0; i < count; i++)
        if (!check_condition(n, &conds[i]))
            return 0;

    return 1;
}


void select_db(char* line, FILE* output, Queue* queue) {
    char* args = line + 6;
    
    int* fields = NULL;
    int field_count;
    
    Condition* conds = NULL;
    int cond_count = 0;

    if (*args == '\0') goto error;
    
    args = trim(args);
    
    char* cond = strchr(args, ' ');
    
    if (cond) {
        *cond++ = '\0';
        if (!parse_conditions(cond, &conds, &cond_count)) goto error;
    }
    
    if (!parse_field_list(args, &fields, &field_count)) goto error;
    
    int found = 0;
    
    for (Node* cur = queue->head; cur; cur = cur->next) {
        if (cond_count && !check_conditions(cur, conds, cond_count)) continue;
        found++;
    }

    fprintf(output, "select:%d\n", found);
    
    for (Node* cur = queue->head; cur; cur = cur->next) {
        if (cond_count && !check_conditions(cur, conds, cond_count)) continue;
        
        for (int i = 0; i < field_count; i++) {
            print_field(output, cur, fields[i]);

            if (i + 1 < field_count)
                fprintf(output, " ");
        }

        fprintf(output, "\n");
    }
    
    free(fields);
    free(conds);
    return;

error:
    fprintf(output, "incorrect:'%.20s'\n", line);
    free(fields);
    free(conds);
    return;
}

void delete_db(char* line, FILE* output, Queue* queue) {
    char* args = line + 6;

    Condition* conds = NULL;
    int cond_count = 0;

    if (*args == '\0') goto error;

    args = trim(args);

    if (!parse_conditions(args, &conds, &cond_count))
        goto error;

    Node* cur = queue->head;
    Node* prev = NULL;

    int deleted = 0;

    while (cur) {
        Node* next = cur->next;

        if (check_conditions(cur, conds, cond_count)) {
            if (prev)
                prev->next = next;
            else
                queue->head = next;

            if (queue->tail == cur) queue->tail = prev;

            free(cur);
            deleted++;
            
        } else {
            prev = cur;
        }

        cur = next;
    }

    fprintf(output, "delete:%d\n", deleted);

    free(conds);
    return;

error:
    fprintf(output, "incorrect:'%.20s'\n", line);
    free(conds);
}

int parse_updates(char* str, Update** upds, int* count) {
    *upds = NULL;
    *count = 0;

    char* token;

    while ((token = next_token(&str, ','))) {
        token = trim(token);

        char* eq = strchr(token, '=');
        if (!eq) return 0;

        *eq = '\0';

        char* field = trim(token);
        char* value = trim(eq + 1);

        int id = -1;
        for (int i = 0; i < FIELD_COUNT; i++)
            if (!strcmp(field, field_names[i])) {
                id = i;
                break;
            }

        if (id == -1) return 0;

        Update* tmp = realloc(*upds, (*count + 1) * sizeof(Update));
        if (!tmp) return 0;

        *upds = tmp;

        Update* u = &(*upds)[*count];
        u->field = id;

        Condition fake;
        fake.field = id;

        if (!parse_condition_value(&fake, value))
            return 0;

        memcpy(&u->value, &fake.value, sizeof(u->value));

        (*count)++;
    }

    return *count > 0;
}

void apply_update(Node* n, Update* upds, int count) {
    for (int i = 0; i < count; i++) {
        switch (upds[i].field) {

        case 0:
            n->unit_id = upds[i].value.i;
            break;
                
        case 1:
            strcpy(n->unit_model, upds[i].value.str);
            break;
        
        case 2:
            strcpy(n->carnum, upds[i].value.carnum);
            break;
                
        case 3:
            n->chk_date = upds[i].value.date;
            break;

        case 4:
            n->status = upds[i].value.status;
            break;

        case 5:
            strcpy(n->mechanic, upds[i].value.str);
            break;
                
        case 6:
            strcpy(n->driver, upds[i].value.str);
            break;
        }
    }
}

void update_db(char* line, FILE* out, Queue* q) {
    char* args = trim(line + 6);
    
    Update* upds = NULL;
    int upd_count = 0;

    Condition* conds = NULL;
    int cond_count = 0;

    if (*args == '\0') goto error;

    char* cond = strchr(args, ' ');

    if (cond) {
        *cond++ = '\0';

        if (!parse_conditions(cond, &conds, &cond_count))
            goto error;
    }

    if (!parse_updates(args, &upds, &upd_count))
        goto error;

    int updated = 0;

    for (Node* cur = q->head; cur; cur = cur->next) {

        if (cond_count && !check_conditions(cur, conds, cond_count))
            continue;

        apply_update(cur, upds, upd_count);
        updated++;
    }

    fprintf(out, "update:%d\n", updated);

    free(upds);
    free(conds);
    return;

error:
    fprintf(out, "incorrect:'%.20s'\n", line);
    free(upds);
    free(conds);
}

void reverse_queue(Queue* q) {
    Node* prev = NULL;
    Node* cur = q->head;

    while (cur) {
        Node* next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }

    q->head = prev;
}

int check_carnum(char* a, char* b) {
    if (strncmp(a, b, 6)) return 1;
    int reg_a = atoi(a + 6);
    int reg_b = atoi(b + 6);

    return reg_a != reg_b;
}

int nodes_equal(Node* a, Node* b, int* fields, int count) {
    for (int i = 0; i < count; i++) {
        switch (fields[i]) {

        case 0:
            if (a->unit_id != b->unit_id) return 0;
            break;
                
        case 1:
            if (strcmp(a->unit_model, b->unit_model)) return 0;
            break;
                
        case 2:
            if (check_carnum(a->carnum, b->carnum)) return 0;
            break;
            
        case 3:
            if (memcmp(&a->chk_date, &b->chk_date, sizeof(Date)))
                return 0;
            break;

        case 4:
            if (a->status != b->status) return 0;
            break;

        case 5:
            if (strcmp(a->mechanic, b->mechanic)) return 0;
            break;
        case 6:
            if (strcmp(a->driver, b->driver)) return 0;
            break;
        }
    }

    return 1;
}


void uniq_db(char* args, FILE* out, Queue* q) {
    args = trim(args + 4);
    reverse_queue(q);
    
    int* fields = NULL;
    int field_count;

    Node** seen = NULL;
    int seen_count = 0;

    Node* prev = NULL;
    Node* cur = q->head;

    int removed = 0;
    
    if (!parse_field_list(args, &fields, &field_count)) goto error;

    while (cur) {

        int duplicate = 0;

        for (int i = 0; i < seen_count; i++) {
            if (nodes_equal(cur, seen[i], fields, field_count)) {
                duplicate = 1;
                break;
            }
        }

        if (duplicate) {

            Node* del = cur;

            if (prev)
                prev->next = cur->next;
            else
                q->head = cur->next;

            cur = cur->next;

            free(del);
            removed++;
            continue;
        }

        Node** tmp = realloc(seen, (seen_count + 1) * sizeof(Node*));

        if (!tmp) break;

        seen = tmp;
        seen[seen_count++] = cur;

        prev = cur;
        cur = cur->next;
    }

    reverse_queue(q);

    free(seen);

    fprintf(out, "uniq:%d\n", removed);
    return;
    
error:
    fprintf(out, "incorrect:'%.20s'\n", args);
    free(seen);
    return;
}


int parse_sort_keys(char* str, SortKey** keys, int* count)
{
    *keys = NULL;
    *count = 0;

    char* token;

    while ((token = next_token(&str, ','))) {

        char* eq = strchr(token, '=');
        if (!eq) return 0;

        *eq = '\0';

        char* field = trim(token);
        char* ord   = trim(eq + 1);

        int f = -1;

        for (int i = 0; i < FIELD_COUNT; i++) {
            if (strcmp(field, field_names[i]) == 0) {
                f = i;
                break;
            }
        }

        if (f == -1)
            return 0;

        if (f == 4)
            return 0;

        for (int i = 0; i < *count; i++)
            if ((*keys)[i].field == f)
                return 0;

        SortKey* tmp = realloc(*keys, (*count + 1) * sizeof(SortKey));
        if (!tmp) return 0;

        *keys = tmp;

        (*keys)[*count].field = f;
        if (!strcmp(ord, "asc")) (*keys)[*count].order = ORDER_ASC;
        else if (!strcmp(ord, "desc")) (*keys)[*count].order = ORDER_DESC;
        else return 0;

        (*count)++;
    }

    return *count > 0;
}


Node* split(Node* head) {
    Node* slow = head;
    Node* fast = head->next;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    Node* mid = slow->next;
    slow->next = NULL;

    return mid;
}


int compare_nodes(Node* a, Node* b, SortKey* keys, int n) {
    for (int i = 0; i < n; i++) {

        int cmp = 0;

        switch (keys[i].field) {

        case 0:
            cmp = (a->unit_id > b->unit_id) - (a->unit_id < b->unit_id);
            break;

        case 1:
            cmp = strcmp(a->unit_model, b->unit_model);
            break;

        case 2:
            cmp = cmp_carnum(a->carnum, b->carnum, OP_GT) - cmp_carnum(a->carnum, b->carnum, OP_LT);
            break;

        case 3:
            cmp = cmp_date(a->chk_date, b->chk_date, OP_GT) - cmp_date(a->chk_date, b->chk_date, OP_LT);
            break;

        case 5:
            cmp = strcmp(a->mechanic, b->mechanic);
            break;

        case 6:
            cmp = strcmp(a->driver, b->driver);
            break;
        }

        if (cmp != 0) {

            if (keys[i].order == ORDER_DESC)
                cmp = -cmp;

            return cmp;
        }
    }

    return 0;
}


Node* merge(Node* a, Node* b, SortKey* keys, int n) {
    Node dummy;
    Node* tail = &dummy;

    while (a && b) {
        if (compare_nodes(a, b, keys, n) <= 0) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }

        tail = tail->next;
    }

    tail->next = a ? a : b;

    return dummy.next;
}


Node* merge_sort(Node* head, SortKey* keys, int n) {
    if (!head || !head->next)
        return head;

    Node* mid = split(head);

    head = merge_sort(head, keys, n);
    mid  = merge_sort(mid, keys, n);

    return merge(head, mid, keys, n);
}


void sort_db(char* line, FILE* out, Queue* q) {
    char* args = trim(line + 4);

    SortKey* keys = NULL;
    int key_count = 0;

    if (!parse_sort_keys(args, &keys, &key_count))
        goto error;

    q->head = merge_sort(q->head, keys, key_count);

    fprintf(out, "sort:%d\n", q->size);

    free(keys);
    return;

error:
    fprintf(out, "incorrect:'%.20s'\n", line);
    free(keys);
}


void read_input(FILE* input, FILE* output, Queue* queue) {
    char* line = NULL;
    size_t cap = 0;
    
    while (getline(&line, &cap, input) != -1) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }
        
        if (strncmp(line, "insert", 6) == 0 && line[6] == ' ') {
            insert_db(line, output, queue);
            
        } else if (strncmp(line, "select", 6) == 0 && line[6] == ' ') {
            select_db(line, output, queue);
            
        } else if (strncmp(line, "delete", 6) == 0 && line[6] == ' ') {
            delete_db(line, output, queue);
            
        } else if (strncmp(line, "update", 6) == 0 && line[6] == ' ') {
            update_db(line, output, queue);
            
        } else if (strncmp(line, "uniq", 4) == 0 && line[4] == ' ') {
            uniq_db(line, output, queue);
            
        } else if (strncmp(line, "sort", 4) == 0 && line[4] == ' ') {
            sort_db(line, output, queue);
            
        } else {
            fprintf(output, "incorrect:'%.20s'\n", line);
        }
        
    }
    
    free(line);
}

void free_db(struct Queue* queue) {
    int cnt_task = 0;
    struct Node* current = queue->head;
    struct Node* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
        cnt_task++;
    }
    
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    
}

int main(void) {
    FILE* input = fopen("input.txt", "r");
    FILE* output = fopen("output.txt", "w");
    if (!input || !output) {
        return 1;
    }
    
    struct Queue queue;
    init_queue(&queue);
    
    read_input(input, output, &queue);
    
    free_db(&queue);
    
    fclose(input);
    fclose(output);
    
    return 0;
}
