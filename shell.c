#include "parser/ast.h"
#include "shell.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

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
    wait(&child_status);
    
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

void handleStopSignal(){
    return;
}

void executeCommandOnPipe(node_t *pipe_node){
    switch (pipe_node->type)
    {  
    case NODE_COMMAND: 
        execvp(pipe_node->command.program, pipe_node->command.argv);
        break;
    case NODE_SEQUENCE:
        executeCommandOnPipe(pipe_node->sequence.first);
        executeCommandOnPipe(pipe_node->sequence.second);
        break;
    default:
        break;
    }
}


int executeToPipe( node_t *pipe_node, int newPipe[]){
    int pid = fork();
    if ( pid == 0)
    {
        close(newPipe[PIPE_RD]);
        dup2(newPipe[PIPE_WR], STDOUT);
        executeCommandOnPipe(pipe_node);
    } 
    close(newPipe[PIPE_WR]); // close pipe WR end when finished writing to it (start closing pipe)
    return pid;
}

int executeFromPipeToPipe(node_t *pipe_node, int oldPipe[], int newPipe[]){
    int pid = fork();
    if ( pid == 0)
    {
        close(oldPipe[PIPE_WR]); // close unused pipe ends on child
        close(newPipe[PIPE_RD]);

        dup2(oldPipe[PIPE_RD], STDIN);
        dup2(newPipe[PIPE_WR], STDOUT);
        executeCommandOnPipe(pipe_node);
    } 
    close(oldPipe[PIPE_RD]); // finish closing old pipe
    close(newPipe[PIPE_WR]); // close WR end of new pipe (start closing pipe)
    return pid;
}

int executeFromPipeToSTDOUT(node_t *pipe_node, int pipeFDs[]){
    int pid = fork();
    if ( pid == 0)
    {
        close(pipeFDs[PIPE_WR]);
        dup2(pipeFDs[PIPE_RD], STDIN);
        executeCommandOnPipe(pipe_node);
    } 
    close(pipeFDs[PIPE_RD]); // close pipe RD end when finished reading from it (finish closing pipe)
    return pid;

}



void handlePipe(node_t *node){
    int pids[node->pipe.n_parts];
    int pipes[node->pipe.n_parts-1][2];
    for (size_t i = 0; i < node->pipe.n_parts - 1; i++)
    {
        pipe(pipes[i]);
    }

    pids[0] = executeToPipe(node->pipe.parts[0], pipes[0]);
    
    
    for (size_t i = 1; i < node->pipe.n_parts - 1; i++)
    {
        pids[i] = executeFromPipeToPipe(node->pipe.parts[i], pipes[i-1], pipes[i]);
    }
    
    int lastCommandIndx = node->pipe.n_parts - 1;
    pids[lastCommandIndx] = executeFromPipeToSTDOUT(node->pipe.parts[lastCommandIndx], pipes[lastCommandIndx-1]);
    
    for (size_t i = 0; i < node->pipe.n_parts ; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
    


    /*
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
    waitpid(sort_id, NULL, 0);*/

}


void run_command(node_t *node)
{
    /* Print parsed input for testing - comment this when running the tests! */
    //print_tree(node);

    signal(SIGINT, &handleStopSignal);

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
