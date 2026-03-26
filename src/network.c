#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "network.h"

typedef struct {
    char* response_text;
    size_t size;
}network_buffer;

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* custom_data){
    size_t real_size = size * nmemb;

    network_buffer* buffer = (network_buffer*)custom_data;

    char* ptr = realloc(buffer->response_text, buffer->size + real_size + 1);

    if(ptr == NULL){
        printf("[ERROR] Not enough memory for network response!\n");
        return 0;
    }

    buffer->response_text = ptr;
    memcpy(&(buffer->response_text[buffer->size]), contents, real_size);
    buffer->size += real_size;
    buffer->response_text[buffer->size] = 0;

    return real_size;
}

char* make_http_request(const char* url){
    CURL* curl;
    CURLcode res;

    network_buffer chunk;
    chunk.response_text = malloc(1);
    chunk.size = 0;
    
    curl = curl_easy_init();

    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "JARVIS-Agent/1.0");

        res = curl_easy_perform(curl);

        if(res != CURLE_OK){
            fprintf(stderr, "[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.response_text);
        }

        curl_easy_cleanup(curl);
    }

    return chunk.response_text;
}

void test_internet_connection(){
    CURL* curl = curl_easy_init();

    if(curl){
        printf("[J.A.R.V.I.S. Network] Online module is active. libcurl started successfully!\n\n");

        curl_easy_cleanup(curl);
    } else{
        printf("[J.A.R.V.I.S. Network] CRITICAL ERROR: Could not start libcurl.\n\n");
    }
}