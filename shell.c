#include "parser/ast.h"
#include "shell.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define CD "cd"
#define EXIT "cd"

void initialize(void)
{
    /* This code will be called once at startup */
    if (prompt)
        prompt = "vush$ ";
}

void handleExternal(node_t *node)
{
    int child_status;
    if (fork() == 0)
    {
        if(execvp(node->command.program, node->command.argv) == -1){
            perror("Error: ");
        }
    } 
    else
    {
        wait(&child_status);
    }
}




void run_command(node_t *node)
{
    /* Print parsed input for testing - comment this when running the tests! */
    //print_tree(node);


    switch (node->type)
    {
    case NODE_COMMAND:

        if (strcmp(node->command.program, CD) == 0)
        {
            //handleCd();
        }
        else if(strcmp(node->command.program, CD) == 0)
        {
            //handleExit();
        }
        else
        {
            handleExternal(node);
        }
        
        break;
    case NODE_PIPE:
        printf("this is a pipe");
        break;
    default:
        break;
    }

    if (prompt)
        prompt = "vush$ ";


}
