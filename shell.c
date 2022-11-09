#include "parser/ast.h"
#include "shell.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define CD "cd"
#define EXIT "exit"
#define PIPE_RD 0
#define PIPE_WR 1
#define STDIN 0
#define STDOUT 1


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

void handlePipe(node_t *node){
    
    int fd[2];
    int cat_id, sort_id;
    pipe(fd);
    cat_id = fork();
    if ( cat_id == 0)
    {
        close(fd[PIPE_RD]);
        close(STDOUT);
        dup(fd[PIPE_WR]);

        node_t *pipe_node = node->pipe.parts[0];
        
        execvp(pipe_node->command.program, pipe_node->command.argv);
    } 
    sort_id = fork();
    if ( sort_id == 0)
    {
        close(fd[PIPE_WR]);
        close(STDIN);
        dup(fd[PIPE_RD]);
        node_t *pipe_node = node->pipe.parts[1];
        execvp(pipe_node->command.program, pipe_node->command.argv);
    } 

    close(fd[PIPE_RD]);
    close(fd[PIPE_WR]);

    waitpid(cat_id, NULL, 0);
    waitpid(sort_id, NULL, 0);

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
        handlePipe(node);
        
        break;
    default:
        break;
    }

    if (prompt)
        prompt = "vush$ ";


}
