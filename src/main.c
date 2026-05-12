#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <pthread.h>
#include <unistd.h>

#include "os_utils.h"
#include "network.h"
#include "payload_handler.h"

volatile int ai_is_thinking = 0;

void* loading_animation(void* arg) {
    const char* messages[] = {
        "J.A.R.V.I.S. is thinking...",
        "J.A.R.V.I.S. is processing...",
        "J.A.R.V.I.S. is analyzing...",
        "J.A.R.V.I.S. is formulating..."
    };
    int num_messages = 4;
    int msg_idx = 0;

    while (ai_is_thinking) {
        const char* current_msg = messages[msg_idx];
        int len = strlen(current_msg);

        printf("\r                                          \r");
        fflush(stdout);

        for (int i = 0; i < len && ai_is_thinking; i++) {
            printf("%c", current_msg[i]);
            fflush(stdout);
            usleep(50000); 
        }

        for (int p = 0; p < 20 && ai_is_thinking; p++) {
            usleep(50000); 
        }

        msg_idx = (msg_idx + 1) % num_messages; 
    }

    printf("\r                                          \r");
    fflush(stdout);
    
    return NULL;
}

int main(){
    const char* api_key = getenv("JARVIS_API_KEY");

    if(api_key == NULL){
        printf("[WARNING] No API Key found in environment variables. Some features may not work.\n\n");
        return 0;
    }

    const char* api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.1-pro-preview:generateContent";

    char user_command[MAX_COMMAND_SIZE - 16];

    printf("=========================================\n");
    printf("  J.A.R.V.I.S. System Initialized (v1.0)\n");
    printf("=========================================\n\n");

    curl_global_init(CURL_GLOBAL_DEFAULT);

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

        char current_ai_prompt[MAX_COMMAND_SIZE + MAX_OUTPUT_SIZE + 512];
        strcpy(current_ai_prompt, user_command);

        int ai_task_finished = 0;

        while(!ai_task_finished){

            char* json_payload = build_ai_payload(current_ai_prompt);

            if(json_payload == NULL){
                fprintf(stderr, "[ERROR] Failed to build AI payload.\n");
                break;
            }

            ai_is_thinking = 1;
            pthread_t loading_thread;
            pthread_create(&loading_thread, NULL, loading_animation, NULL);

            char* raw_response = send_ai_payload(api_url, api_key, json_payload);

            ai_is_thinking = 0;
            pthread_join(loading_thread, NULL);
            
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
                    char user_confirmation;
                    printf("The next command will be executed: %s\n", jarvis_command);
                    printf("Do you want to proceed? (y/n): ");
                    scanf(" %c", &user_confirmation);

                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);

                    if(user_confirmation != 'y' && user_confirmation != 'Y'){
                        printf("Command execution cancelled by user.\n\n");
                        free(jarvis_command);
                        ai_task_finished = 1;
                        continue;
                    }

                    printf("[EXECUTING COMMAND]...\n");

                    char* command_output = execute_command(jarvis_command);

                    if (command_output != NULL) {

                        if (strlen(command_output) > 0) {
                            printf("\n[TERMINAL OUTPUT]:\n%s\n", command_output);
                        } else {
                            char* empty_msg = "(Command executed successfully, but returned no output.)";
                            printf("\n[TERMINAL OUTPUT]:\n%s\n\n", empty_msg);
                            
                            command_output = realloc(command_output, strlen(empty_msg) + 1);
                            strcpy(command_output, empty_msg);
                        }

                        snprintf(current_ai_prompt, sizeof(current_ai_prompt), 
                            "[SYSTEM AUTOMATED FEEDBACK]\nThe command you just ran outputted this data:\n%s\n\n"
                            "Based on this output, generate the next command to fulfill the user's original request: '%s'. "
                            "If the task is fully complete, leave the 'command' field COMPLETELY EMPTY.", 
                            command_output, user_command);

                        free(command_output); 
                    }
                    free(jarvis_command);
                } else{
                    free(jarvis_command);
                    ai_task_finished = 1;
                }
            } else{
                ai_task_finished = 1;
            }
        }

    }

    printf("J.A.R.V.I.S.> System shutting down. Goodbye, Sir!\n");

    curl_global_cleanup();

    return 0;
}