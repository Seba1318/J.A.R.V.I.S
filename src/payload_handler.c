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

    char full_command[16384];
    snprintf(full_command, sizeof(full_command), 
        "You are J.A.R.V.I.S., a polite and highly precise Linux terminal AI assistant. "
        "Current directory: '%s'. "
        "User request: '%s'. "
        "### ABSOLUTE BASH AND JSON PROTOCOL ###\n"
        "1. To create or overwrite files, ALWAYS use Heredoc syntax EXACTLY like this:\n"
        "   cat << 'EOF' > /absolute/path/to/filename.c\\n[YOUR_CODE_HERE]\\nEOF\\n\n"
        "2. CRITICAL 'EOF' RULE: The closing 'EOF' MUST be preceded by exactly one '\\n' AND followed by exactly one '\\n'. This guarantees the shell closes the file cleanly without writing EOF inside it.\n"
        "3. NEVER use the word 'EOF' inside the actual code block.\n"
        "4. SEARCH RULE: When finding a folder/file dynamically, search from '~' but EXCLUDE hidden, snap, and trash paths using grep. Example: DIR=$(find ~ -type d -name 'Target' 2>/dev/null | grep -Fv '.local' | grep -Fv 'snap' | grep -Fv 'Trash' | head -n 1)\n"
        "5. CONVERSATIONAL RULE: If the user just greets you or NO terminal action is required, leave the 'command' value COMPLETELY EMPTY (\"\"). Do NOT output 'true', 'echo', or any placeholder.\n"
        "6. ANTI-HALLUCINATION RULE: NEVER guess, invent, or predict the output of a command (like file or folder names) in your 'message'. If you are executing a command to find information, your message must only state your intent (e.g., 'Searching your system now, Sir.'). Let the terminal output provide the actual data.\n"
        "7. Respond STRICTLY with a valid raw JSON object on a SINGLE physical line. Escape all internal double quotes as \\\" and all internal newlines as \\n.\n"
        "JSON format: {\"command\": \"...\", \"message\": \"...\"} (No Markdown, no ```json blocks). (In your message, you will always address the user as 'Sir').", cwd, user_command);
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