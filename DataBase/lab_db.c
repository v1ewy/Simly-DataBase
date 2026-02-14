#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define FIELD_COUNT 7

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
    char unit_model[16];
    char carnum[16];
    Date chk_date;
    Status status;
    char mechanic[256];
    char driver[256];
    struct Node* next;
} Node;

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

    if (strlen(value) >= max)
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

// function insert
void insert_db(char* line, FILE* output, Node** head, int* size_db) {
    Node* new_node = malloc(sizeof(Node));
    char* args = line + 6;

    if (*args == '\0') {
        fprintf(output, "incorrect:'%.20s'\n", line);
        return;
    }

    char* copy = strdup(args);
    if (!copy) goto error;

    char* seen[FIELD_COUNT] = {0};

    char* token = strtok(copy, ",");

    while (token) {

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

        token = strtok(NULL, ",");
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
    
    
    new_node->next = *head;
    *head = new_node;

    fprintf(output, "insert:%d, %d, %s, %s, '%02d.%02d.%d',%s, %s, %s\n", ++(*size_db), (*head)->unit_id, new_node->unit_model, new_node->carnum,
            (*head)->chk_date.day, (*head)->chk_date.month, (*head)->chk_date.year, status_to_string(new_node->status)
            , new_node->mechanic, new_node->driver);
    
    free(copy);
    return;

error:
    fprintf(output, "incorrect:'%.20s'\n", line);
    free(copy);
    free(new_node);
}

void read_input(FILE* input, FILE* output, Node** head, int* db_size) {
    char* line = NULL;
    size_t cap = 0;
    
    while (getline(&line, &cap, input) != -1) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }
        
        if (strncmp(line, "insert", 6) == 0 && line[6] == ' ')
        {
            insert_db(line, output, head, db_size);
            
        }
        else if (strncmp(line, "select", 6) == 0 && line[6] == ' ')
        {
            fprintf(output, "select:0\n");
        }
        else if (strncmp(line, "delete", 6) == 0 && line[6] == ' ')
        {
            fprintf(output, "delete:0\n");
        }
        else if (strncmp(line, "uniq", 4) == 0 && line[4] == ' ')
        {
            fprintf(output, "uniq:0\n");
        }
        else if (strncmp(line, "sort", 4) == 0 && line[4] == ' ')
        {
            fprintf(output, "sort:0\n");
            
        } else {
            fprintf(output, "incorrect:'%.20s'\n", line);
        }
        
    }
    
    free(line);
}

void free_db(Node* head) {
    Node* tmp;

    while (head) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
    
    head = NULL;
}

int main(void) {
    FILE* input = fopen("input.txt", "r");
    FILE* output = fopen("output.txt", "w");
    if (!input || !output) {
        return 1;
    }
    
    Node* head = NULL;
    int db_size = 0;
    
    read_input(input, output, &head, &db_size);
    
    free_db(head);
    
    fclose(input);
    fclose(output);
    
    return 0;
}
