#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void insert_db(char* args, FILE* output) {
   
    char* copy = strdup(args);
    if (!copy) return;

    char* token = strtok(copy, ",");
    while (token != NULL) {

        if (strchr(token, '=') == NULL) {
            fprintf(output, "incorrect:'%.20s'\n", line);
            free(copy);
            return;
        }

        token = strtok(NULL, ",");
    }

    free(copy);

    fprintf(output, "insert:0\n");
}


void read_input(FILE* input, FILE* output) {
    char* line = NULL;
    size_t cap = 0;
    
    while (getline(&line, &cap, input) != -1) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }
        
        if (strncmp(line, "insert", 6) == 0 && line[6] == ' ')
        {
            char* args = line + 6;
            
            while (*args == ' ') {
                args++;
            }

            if (*args == '\0') {
                fprintf(output, "incorrect:'%.20s'\n", line);
                return;
            } else {
                insert_db(args, output);
            }
            
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

int main(void) {
    FILE* input = fopen("input.txt", "r");
    FILE* output = fopen("output.txt", "w");
    if (!input || !output) {
        return 1;
    }
    
    read_input(input, output);
    
    fclose(input);
    fclose(output);
    
    return 0;
}
