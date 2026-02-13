#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define FIELD_COUNT 7

typedef enum {
    well,
    wearlow,
    wearhigh,
    broken,
    notcheck
} Status;

typedef struct {
    int day;
    int month;
    int year;
} Date;

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


const char* field_names[FIELD_COUNT] = {
    "unit_id",
    "unit_model",
    "car_id",
    "chk_date",
    "status",
    "mechanic",
    "driver"
};

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

int parse_int(const char* s, int* out) {
    char* end;
    long val = strtol(s, &end, 10);

    if (*s == '\0' || *end != '\0')
        return 0;

    *out = (int)val;
    return 1;
}

int parse_quoted_string(char* value, char* dest, size_t max)
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
    if (!parse_quoted_string(seen[1], new_node->unit_model, sizeof(&new_node->unit_model))) {
        goto error;
    }
    
    new_node->next = *head;
    *head = new_node;

    fprintf(output, "insert:%d, %d, %s\n", new_node->unit_id, ++(*size_db), new_node->unit_model);
    
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
