#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_utils.h"
#include "network.h"
#include "payload_handler.h"

int main(){
    char user_command[256];

    printf("--- J.A.R.V.I.S OS Tester ---\n");

    test_internet_connection();

    printf("Fetching data from the Matrix...\n");
    char* api_response = make_http_request("https://jsonplaceholder.typicode.com/todos/1");

    if(api_response){
        printf("--- API RESPONSE ---\n");
        printf("%s", api_response);
        printf("\n--------------------\n\n");

        free(api_response);
    } else{
        printf("[ERROR] Failed to fetch data from API.\n");
    }

    printf("Type OS commands to test the capture. Type 'exit' to quit. \n\n");

    while(1){
        printf("J.A.R.V.I.S> ");

        if(fgets(user_command, sizeof(user_command), stdin) == NULL){
            break;
        } //in case of Ctrl+D (EOF)

        user_command[strcspn(user_command, "\n")] = 0;

        if(strlen(user_command) == 0){
            continue;
        }

        if(strcmp(user_command, "exit") == 0){
            break;
        }

        if (strcmp(user_command, "clear") == 0) {
            system("clear");
            printf("--- EXECUTION RESULT ---\n");
            printf("[Terminal cleared]\n\n");

            continue; 
        }

        char* output =  execute_command(user_command);

        if(output){
            printf("\n--- EXECUTION RESULT ---\n\n");
            printf("%s\n", output);

            if(strlen(output) > 0 && output[strlen(output) - 1] != '\n'){
                printf("\n");
            }

            free(output);
        } else{
            printf("[ERROR] The command did not return a valid result.\n");
        }

        printf("--- J.A.R.V.I.S. JSON Payload Test ---\n\n");

        const char* command = "Hello J.A.R.V.I.S.! Create a folder named secret_project.";
        char* json_package = build_ai_payload(command);
        
        if (json_package != NULL) {
            printf("Payload ready to be sent over the network:\n");
            printf("%s\n\n", json_package);
            
            free(json_package);
            json_package = NULL;
        } else {
            printf("[ERROR] Failed to generate JSON payload!\n");
        }
    }

    return 0;
}