#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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
    char unit_model[256];
    char carnum[16];
    Date chk_date;
    Status status;
    char mechanic[256];
    char driver[256];
    struct Node* next;
} Node;




void insert_db(char* line, FILE* output, Node** head, int* size_db) {
    Node* temp = NULL;
    char* args = line + 6;

    while (*args == ' ')
        args++;

    if (*args == '\0') {
        fprintf(output, "incorrect:'%.20s'\n", line);
        return;
    }

    char* copy = strdup(args);
    if (!copy) return;

    int has_unit_id = 0;
    int has_unit_model = 0;
    int has_car_id = 0;
    int has_chk_date = 0;
    int has_status = 0;
    int has_mechanic = 0;
    int has_driver = 0;

    char* token = strtok(copy, ",");

    while (token != NULL) {
        while (*token == ' ')
            token++;

        char* eq = strchr(token, '=');
        if (eq == NULL) {
            fprintf(output, "incorrect:'%.20s'\n", line);
            free(copy);
            return;
        }

        *eq = '\0';
        char* field = token;
        char* value = eq + 1;

        if (*field == '\0' || *value == '\0') {
            fprintf(output, "incorrect:'%.20s'\n", line);
            free(copy);
            return;
        }

        if (strcmp(field, "unit_id") == 0) {
            if (has_unit_id) {
                fprintf(output, "incorrect:'%.20s'\n", line);
                free(copy);
                return;
            }
            has_unit_id = 1;

        }
        else if (strcmp(field, "unit_model") == 0) {
            if (has_unit_model) {
                fprintf(output, "incorrect:'%.20s'\n", line);
                free(copy);
                return;
            }
            has_unit_model = 1;
        }
        else if (strcmp(field, "car_id") == 0) {
            if (has_car_id) {
                fprintf(output, "incorrect:'%.20s'\n", line);
                free(copy);
                return;
            }
            has_car_id = 1;
        }
        else if (strcmp(field, "chk_date") == 0) {
            if (has_chk_date) {
                fprintf(output, "incorrect:'%.20s'\n", line);
                free(copy);
                return;
            }
            has_chk_date = 1;
        }
        else if (strcmp(field, "status") == 0) {
            if (has_status) {
                fprintf(output, "incorrect:'%.20s'\n", line);
                free(copy);
                return;
            }
            has_status = 1;
        }
        else if (strcmp(field, "mechanic") == 0) {
            if (has_mechanic) {
                fprintf(output, "incorrect:'%.20s'\n", line);
                free(copy);
                return;
            }
            has_mechanic = 1;
        }
        else if (strcmp(field, "driver") == 0) {
            if (has_driver) {
                fprintf(output, "incorrect:'%.20s'\n", line);
                free(copy);
                return;
            }
            has_driver = 1;
        }
        else {
            fprintf(output, "incorrect:'%.20s'\n", line);
            free(copy);
            return;
        }

        token = strtok(NULL, ",");
    }

    free(copy);

    if (!has_unit_id || !has_unit_model || !has_car_id ||
        !has_chk_date || !has_status || !has_mechanic || !has_driver) {
        fprintf(output, "incorrect:'%.20s'\n", line);
        return;
    }

    fprintf(output, "insert:0\n");
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
