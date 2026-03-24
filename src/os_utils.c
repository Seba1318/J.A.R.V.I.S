#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_utils.h"

char* execute_command(const char* command){
    FILE* pipe = popen(command, "r");

    if(!pipe){
        return strdup("Error: The process couldn't be opened in terminal!\n");
    }

    char buffer[256];
    size_t current_size = 1; //Initially the buffer length would be 1, for the null terminator
    char* result = malloc(current_size);

    if(!result){
        pclose(pipe);
        return NULL;
    }

    result[0] = '\0';

    size_t total_read = 0;

    while(fgets(buffer, sizeof(buffer), pipe) != NULL){
        size_t buffer_length = strlen(buffer);

        if(total_read + buffer_length > MAX_OUTPUT_SIZE){
            const char* warning = "\n...[OUTPUT SHORTENED: Too long to save tokens]...\n";
            char* temp = realloc(result, current_size + strlen(warning));

            if(temp){
                result = temp;
                strcat(result, warning);
            } else{
                fprintf(stderr, "[DEBUG] Critical memory error during shortening!\n");
            }

            break;
        }

        char* temp = realloc(result, current_size + buffer_length);

        if(!temp){
            fprintf(stderr, "[ERROR] Memory allocation failed during command execution!\n");

            free(result);
            pclose(pipe);
            return NULL;
        }

        result = temp;

        strcat(result, buffer);

        current_size += buffer_length;
        total_read += buffer_length;
    }

    pclose(pipe);

    return result;
}