#include <stdio.h>
#include <stdlib.h>

#include "os_utils.h"

int main(){
    printf("Testing execution in the operating system...\n\n");

    char* output = execute_command("ls -la");

    if(output){
        printf("--- TERMINAL RESPONSE ---\n");
        printf("%s\n", output);
        printf("-------------------------\n");

        free(output);
    } else{
        printf("An error has occurred during execution!\n");
    }

    return 0;
}