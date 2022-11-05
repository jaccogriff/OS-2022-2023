#include "parser/ast.h"
#include "shell.h"

void initialize(void)
{
    /* This code will be called once at startup */
    if (prompt)
        prompt = "vush$ ";
}

void run_command(node_t *node)
{
    /* Print parsed input for testing - comment this when running the tests! */
    //print_tree(node);

    int pid, child_status;

    switch (node->type)
    {
    case NODE_COMMAND:

        if (fork() == 0)
        {
            execvp(node->command.program, node->command.argv);
        } 
        else
        {
            wait(&child_status);
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
