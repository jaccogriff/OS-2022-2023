#include "parser/ast.h"
#include "shell.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define CD "cd"
#define EXIT "exit"

void initialize(void)
{
    /* This code will be called once at startup */
    if (prompt)
        prompt = "vush$ ";
}

void printIfError(int resultOfOperation)
{
    if(resultOfOperation == -1){
            perror("Error: ");
    }
}

void handleExternal(node_t *node)
{
    int child_status;
    if (fork() == 0)
    {
        printIfError(
            execvp(node->command.program, node->command.argv)
        );
    } 
    else
    {
        wait(&child_status);
    }
}

void handleExit(node_t *node)
{
    int exitCode = atoi(node->command.argv[1]);
    exit(exitCode);
}

void handleCd(node_t *node)
{
    printIfError(
        chdir(node->command.argv[1])
    );
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
            handleCd(node);
        }
        else if(strcmp(node->command.program, EXIT) == 0)
        {
            handleExit(node);
        }
        else
        {
            handleExternal(node);
        }
        
        break;
    case NODE_SEQUENCE:
        run_command(node->sequence.first);
        run_command(node->sequence.second);
        break;
    case NODE_PIPE:;
        size_t i;
        for (i = 0; i < node->pipe.n_parts; ++i)
            run_command(node->pipe.parts[i]);
        
        
        break;
    default:
        break;
    }

    if (prompt)
        prompt = "vush$ ";


}
