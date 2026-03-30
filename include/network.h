#ifndef NETWORK_H
#define NETWORK_H

void test_internet_connection();

char* send_ai_payload(const char* url, const char* api_key, char* json_payload);

#endif