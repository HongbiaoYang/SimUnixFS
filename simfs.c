/* 
 * File:   simfs.c
 * Author: Bill
 *
 * Created on April 3, 2014, 4:15 PM
 */

#include <stdio.h>
#include <stdlib.h>

#define LINE 128
/*
 * 
 */
int main(int argc, char** argv) {

    char path[LINE] = "home/";
    char command[LINE];
    
    printf("Welcome to simulated file system!\nType HELP for help\n");

    printf("$%s$>",path);        
    while (gets(command) != NULL)
    {
		printf("%s\n", command);
        printf("$%s$>",path);        
        
    }
    return (EXIT_SUCCESS);
}

