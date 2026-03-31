#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_utils.h"
#include "network.h"
#include "payload_handler.h"

int main(){
    const char* api_key = getenv("JARVIS_API_KEY");

    if(api_key == NULL){
        printf("[WARNING] No API Key found in environment variables. Some features may not work.\n\n");
        return 0;
    }

    const char* api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent";

    char user_command[512];

    printf("=========================================\n");
    printf("  J.A.R.V.I.S. System Initialized (v1.0)\n");
    printf("=========================================\n\n");

    // test_internet_connection();

    while(1){
        printf(">");

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

        printf("[SYSTEM] Transmitting request to AI core...\n\n");

        char* json_payload = build_ai_payload(user_command);

        if(json_payload == NULL){
            fprintf(stderr, "[ERROR] Failed to build AI payload.\n");
            continue;
        }

        char* raw_response = send_ai_payload(api_url, api_key, json_payload);
        free(json_payload);

        if(raw_response){
            char* jarvis_command = NULL;
            char* jarvis_message = NULL;

            parse_ai_response(raw_response, &jarvis_command, &jarvis_message);
            free(raw_response);

            if(jarvis_message){
                printf("J.A.R.V.I.S.> %s\n\n", jarvis_message);
                free(jarvis_message);
            }

            if(jarvis_command && strlen(jarvis_command) > 0){
                printf("[EXECUTING COMMAND] %s\n\n", jarvis_command);

                char* command_output = execute_command(jarvis_command);

                if (command_output != NULL) {
                    // command_output[strcspn(command_output, "\r\n")] = 0;

                    if (strlen(command_output) > 0) {
                        printf("%s\n", command_output);
                    }
                    free(command_output); 
                }

                free(jarvis_command);
            } else{
                free(jarvis_command);
            }
        }
    }

    printf("J.A.R.V.I.S.> System shutting down. Goodbye, Sir!\n");

    return 0;
}