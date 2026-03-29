#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

#include "payload_handler.h"

char* build_ai_payload(const char* user_input){
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"model", "gpt-4o-mini");

    cJSON* messages = cJSON_AddArrayToObject(root, "messages");

    cJSON* system_message = cJSON_CreateObject();
    cJSON_AddStringToObject(system_message, "role", "system");
    cJSON_AddStringToObject(system_message, "content", 
        "You are J.A.R.V.I.S., a brilliant and polite AI assistant with access to a Linux terminal. "
        "You MUST ALWAYS respond in valid JSON format containing exactly two keys: "
        "1. 'command': The raw Ubuntu Linux command to execute (leave empty string if no action is needed). "
        "2. 'message': Your conversational reply to the user, acting as a helpful and witty assistant. "
        "Do NOT wrap the JSON in markdown blocks. Output raw JSON only."    
    );
    cJSON_AddItemToArray(messages, system_message);

    cJSON* user_message = cJSON_CreateObject();
    cJSON_AddStringToObject(user_message, "role", "user");
    cJSON_AddStringToObject(user_message, "content", user_input);
    cJSON_AddItemToArray(messages, user_message);

    char* payload = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    return payload;
}