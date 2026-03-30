#ifndef PAYLOAD_HANDLER_H
#define PAYLOAD_HANDLER_H

char* build_ai_payload(const char* user_input);

void parse_ai_response(const char* json_response, char** out_command, char** out_message);

#endif