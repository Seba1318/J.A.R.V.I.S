#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Required for getcwd() (letting J.A.R.V.I.S. know where he is, in order to give better responses)
#include <cjson/cJSON.h>

#include "payload_handler.h"

char* build_ai_payload(const char* user_command){
    cJSON* root = cJSON_CreateObject();
    cJSON* contents_array = cJSON_CreateArray();
    cJSON* content_obj = cJSON_CreateObject();
    cJSON* parts_array = cJSON_CreateArray();
    cJSON* part_obj = cJSON_CreateObject();

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "unknown location");
    }

    char full_command[4096];
    snprintf(full_command, sizeof(full_command), 
        "You are J.A.R.V.I.S., a polite, advanced, native Linux terminal AI assistant. "
        "Your current working directory is: '%s'. "
        "Execute this user request: '%s'. "
        "CRITICAL RULES FOR BASH COMMANDS:\n"
        "1. To write multi-line code or text to files, ALWAYS use Heredoc syntax: 'cat << 'EOF' > filename ... EOF'. NEVER use echo for multi-line code.\n"
        "2. Do not attempt to create directories if the user explicitly states they already exist.\n"
        "Respond STRICTLY with a valid raw JSON object containing exactly two keys: "
        "'command' (the Linux bash command to run, or empty string if none) and 'message' (your verbal reply, where you always address the user as Sir). "
        "Do NOT wrap the JSON in markdown blocks like ```json.", cwd, user_command);

    cJSON_AddStringToObject(part_obj, "text", full_command);
    cJSON_AddItemToArray(parts_array, part_obj);
    cJSON_AddItemToObject(content_obj, "parts", parts_array);
    cJSON_AddItemToArray(contents_array, content_obj);
    cJSON_AddItemToObject(root, "contents", contents_array);

    char* json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_string;
}

void parse_ai_response(const char* json_response, char** out_command, char** out_message){
    *out_command = NULL;
    *out_message = NULL;

    if(!json_response){
        return;
    }

    cJSON* root = cJSON_Parse(json_response);

    if(!root){
        fprintf(stderr, "[API ERROR] Failed to parse the raw API JSON response.\n");
        return;
    }

    cJSON* error_obj = cJSON_GetObjectItemCaseSensitive(root, "error");

    if(error_obj){
        cJSON* error_message = cJSON_GetObjectItemCaseSensitive(error_obj, "message");

        fprintf(stderr, "\n[API FATAL ERROR] %s\n", error_message ? error_message->valuestring : "Unknown API error");

        cJSON_Delete(root);

        return;
    }

    cJSON* candidates = cJSON_GetObjectItemCaseSensitive(root, "candidates");

    if(!candidates || !cJSON_IsArray(candidates)){
        fprintf(stderr, "[API ERROR] Unrecognized format. 'candidates' array is missing.\n");
        cJSON_Delete(root);
        return;
    }

    cJSON* first_candidate = cJSON_GetArrayItem(candidates, 0);
    cJSON* content = cJSON_GetObjectItemCaseSensitive(first_candidate, "content");
    cJSON* parts = cJSON_GetObjectItemCaseSensitive(content, "parts");
    cJSON* first_part = cJSON_GetArrayItem(parts, 0);
    cJSON* text_node = cJSON_GetObjectItemCaseSensitive(first_part, "text");

    if(text_node && cJSON_IsString(text_node) && text_node->valuestring != NULL){
        cJSON* jarvis_json = cJSON_Parse(text_node->valuestring);

        if(jarvis_json){
            cJSON* cmd_item = cJSON_GetObjectItemCaseSensitive(jarvis_json, "command");
            cJSON* msg_item = cJSON_GetObjectItemCaseSensitive(jarvis_json, "message");

            if(cJSON_IsString(cmd_item) && cmd_item->valuestring != NULL){
                *out_command = strdup(cmd_item->valuestring);
            }

            if(cJSON_IsString(msg_item) && msg_item->valuestring != NULL){
                *out_message = strdup(msg_item->valuestring);
            }

            cJSON_Delete(jarvis_json);
        } else{
            fprintf(stderr, "[ERROR] J.A.R.V.I.S. returned invalid JSON format.\n[DUMP]:\n%s\n", text_node->valuestring);
        }
    } else{
        fprintf(stderr, "[ERROR] J.A.R.V.I.S. response is missing the expected 'text' field.\n");   
    }

    cJSON_Delete(root);
}