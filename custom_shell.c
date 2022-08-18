#include "custom_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
// Find out what other #include's you need! (E.g., see man pages.)

int interp(const struct cmd *c)
{
    static int list_fd;
    static bool list_fd_found;
    bool fd_found = false;
    int fd;
    if(list_fd_found == true){
        fd = list_fd;
    }
    if(c->redir_stdout != NULL){
        fd = open(c->redir_stdout, O_WRONLY | O_TRUNC | O_CREAT, 0666);
        if(fd < 0){
            return 1;
        }
        fd_found = true;
    }
    if(c->type == ECHO){
        echo_d echo = c->data.echo;
        if(fd_found == false && list_fd_found == false){
            fd = fileno(stdout);
        }
        int num_bytes = write(fd, echo.arg, strlen(echo.arg));
        if(num_bytes < 0){
            return 1;
        }
        return 0;
    }
    else if(c->type == FORX){
        forx_d forx = c->data.forx;
        int status;
        int fork_val = fork();
        if(fork_val == 0){
            if(fd_found == false && list_fd_found == false){
                fd = fileno(stdout);
            }
            dup2(fd, fileno(stdout));
            int exec_val = execvp(forx.pathname, forx.argv); // Need to add redirection to file in execvp
            if(exec_val < 0){
                perror("exec failed");
                exit(127);
            }
        }
        else{
            int wait_val = waitpid(fork_val, &status, 0);
            if(wait_val < 0){
                perror("wait failed");
                return 1;
            }
            if(WIFEXITED(status)){
                return WEXITSTATUS(status);
            }
            else if(WIFSIGNALED(status)){
                return WTERMSIG(status) + 128;
            }
            else{
                return 0;
            }
        }
    }
    else if(c->type == LIST){
        list_d list = c->data.list;
        int n = list.n;
        if(n==0){
            return 0;
        }
        if(c->redir_stdout != NULL){
            list_fd = open(c->redir_stdout, O_WRONLY | O_TRUNC | O_CREAT, 0666);
            if(list_fd < 0){
                return 1;
            }
            list_fd_found = true;
        }
        int ret_val;
        for(int i=0; i<n; i++){
            ret_val = interp(list.cmds+i);
            if(ret_val == 128+SIGINT){
                return 128+SIGINT;
            }
        }
        list_fd_found = false;
        list_fd = 1;
        return ret_val;
    }
    return 0;
}
