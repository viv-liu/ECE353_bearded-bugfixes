/*
 * main.c: A simple shell
 * Shen Wang
 * Vivian Liu
 * Team Octobear
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#define BUF_SIZE 512
#define MAX_VALUE 4
#define PROMPT "ece353sh$"

/*A linked list node
the path is stored as a string
a pointer to the next node is stored*/
struct pathNode{
    char* path;
    struct pathNode* next;
};
typedef struct pathNode PATHNODE;

struct histNode{
    char* past_input;
    struct histNode* next;
    struct histNode* prev;
};
typedef struct histNode HISTNODE;

int history_count;
HISTNODE *history_head;
HISTNODE *history_tail;
PATHNODE *head; //Define a global variable head as the head of the path variable

int parseinput(char*); //Function Prototype Declaration

void pop_hist_tail()
{
    HISTNODE* new_node;

    if((new_node = (HISTNODE *) malloc(sizeof(HISTNODE))) == NULL)
        {
            // Memory allocation failed
            printf("error: %s", strerror(errno));
            return;
        }
    new_node = history_tail;
    history_tail = history_tail->prev;
    free(new_node);
    history_count--;
    return;
}

void pop_hist_head()
{
    HISTNODE* new_node;

    if((new_node = (HISTNODE *) malloc(sizeof(HISTNODE))) == NULL)
        {
            // Memory allocation failed
            printf("error: %s", strerror(errno));
            return;
        }
    new_node = history_head;
    history_head = history_head->next;
    free(new_node);
    history_count--;
    return;
}

void push_hist(char* input)
{
    HISTNODE* new_node;
    char* tmpInput;
    if((new_node = (HISTNODE *) malloc(sizeof(HISTNODE))) == NULL)
        {
            // Memory allocation failed
            printf("error: %s", strerror(errno));
            return;
        }
    if((tmpInput = (char *) malloc(sizeof(input))) == NULL)
    {
        // Memory allocation failed 
        printf("error: %s", strerror(errno));
        free(new_node);
        return;
    }
    strcpy(tmpInput, input);
    //Initialize new_node
    new_node->past_input = tmpInput;
    new_node->prev = NULL;

    if(history_head == NULL) 
    { // Initialize head
        new_node->next = NULL;
        history_head = new_node;
        history_tail = new_node;
    }
    else
    {
        new_node->next = history_head;
        history_head->prev = new_node;
        history_head = new_node;
    }

    if(history_count >= MAX_VALUE)
    {
        pop_hist_tail();
    }
    history_count++;
    return;
}

/* Prints list of history aka previous inputs ie. [1] path */
void print_hist()
{
    HISTNODE *cur = history_tail;
    int i = 1;
    /* Iterate to end of PATH linked list*/
    while(cur != NULL) 
    {
        printf("[%d] %s", i, cur->past_input);
        cur = cur->prev;
        i++;
    }
    return;
}

/*Executes the nth history command*/
void call_hist(int n)
{
    int i;
    char*tmpInput;
    HISTNODE *cur = history_tail;
    //Given int n, find history, then send that entire history string to parseinput
    for (i = 1; i < (n - 1); i++)
    {
        cur = cur->prev;
    }

    if((tmpInput = (char *) malloc(sizeof(cur->past_input))) == NULL)
    {
        // Memory allocation failed
        printf("error: %s", strerror(errno));
        return;
    }
    strcpy(tmpInput, cur->past_input);

    //Found the nth history node
    pop_hist_head(); //Pop the !n command that has already executed (must be before parseinput)

    parseinput(tmpInput);

    return;
}

/*Adding a node with path to the end of the linked list PATH */
void addPath(char* path) 
{
    /* Check if provided path exists by attempting to open a directory */
    DIR* dir = opendir(path);
    if(dir) 
    {   
        closedir(dir); //Close the directory that was opened
        PATHNODE *new_node, *cur;
        /*Checks if malloc is valid*/
        if((new_node = (PATHNODE *) malloc(sizeof(PATHNODE))) == NULL)
        {
            // Memory allocation failed
            printf("error: %s", strerror(errno));
            return;
        }
        char* tmpPath;
        if((tmpPath = (char *) malloc(sizeof(path))) == NULL)
        {
            // Memory allocation failed
            printf("error: %s", strerror(errno));
            free(new_node);
            return;
        }
        strcpy(tmpPath, path);
        //Initialize new_node
        new_node->path = tmpPath;
        new_node->next = NULL;

        if(head == NULL) 
        { // Initialize head
            head = new_node;
            return;
        }
        else
        {

        cur = head; //assign cur to point to head

        /* Iterate to end of PATH linked list */
        while(cur->next != NULL) 
        {
            cur = cur->next;
        }

        /*Now cur->next == NULL 
        append the new node*/
        cur->next = new_node;
        //TEST: printf ("The path %s was added to PATH.\n",path); 
        }
    }
    /*error caused the problem with open directory*/
    else 
    {
        printf("error: %s", strerror(errno));
    }

    return;
}

/*Removing a node that is to path from PATH */
void removePath(char* path) 
{
    PATHNODE *cur = head;
    PATHNODE *tmp = head;

    if(head == NULL) 
    {
        //TEST: printf ("removePath(): head is empty\n"); 
        return;
    }
    if(strcmp(head->path, path) == 0) //If looking for the head's path
    {
        
        if (head->next != NULL) 
        { //For 2 or more elements in linked list
            tmp->next = head->next;
            //TEST: printf("removePath(): found match for %s, at head\n", path);
            free(head);
            head = tmp->next;
            //TEST: printf("the new head is: %s\n", head->path);
        }

        else 
        {
            free(head);
            head = NULL;
        }

        return;
    }
    
    /* Iterate till we find matching path */
    while(cur->next != NULL) 
    {

        if(strcmp(cur->path, path) == 0) 
        { // Found a match, remove this node
            //TEST: printf ("removePath(): found a match for %s, removing node...\n",path); 
            tmp->next = cur->next;
            free(cur);
            return;
        }

        else 
        {  // Nope, keep going
            tmp = cur;
            cur = cur->next;
        }
    }
    
    printf ("error: removePath() didn't find a match for: %s\n",path); 
    return;
}

/*Print entire PATH linked list, separating nodes with colons*/
void printPath() 
{
    /*When the linked list has no values then print a new line*/
     if(head == NULL) 
     {
        printf("\n");
        return;
     }

    PATHNODE *cur = head;
    /* Iterate to end of PATH linked list*/
    while(cur != NULL) 
    {
        printf("%s:", cur->path);
        cur = cur->next;
    }

    printf("\b \n"); //Replaces the last colon with a space and prints a new line afterwards
    return;
}

/*Current Working Directory Function:
Serves as a test function*/
char* cwd()
{
    /* Checks the Current Working Directory (Test Code) */
    char* cwd;
    size_t allocSize = sizeof(char)*1024;
    if((cwd = (char*) malloc(allocSize)) == NULL)
    {
        // Memory allocation failed
        printf("error: %s", strerror(errno));
        return NULL;
    }
    if (getcwd(cwd, allocSize) != NULL)
        return cwd;
    else
        // getcwd() failed
        printf("error: %s", strerror(errno));
    return NULL;
}

/*Change directory Function*/
int cd(char **cd_path) 
{
    char* thecwd;
    /*If there is a NULL at first then no arguments thus assume return to intial directory*/
    if (cd_path[1] == NULL) 
    {
        /*If cd has no arguments, no change in directory*/
        //TEST: printf ("The path to go to is %s\n",cd_path[1]);
        //TEST: thecwd = cwd();//Test: Checks the Current Working Directory
        //TEST: printf(thecwd);
        //TEST: printf("\n");
        return 0;
    }

    // /*If there are more than 1 argument then 2nd item in list is not NULL, exit to shell*/
    // if (cd_path[2] != NULL)
    // {
    //     printf ("Too many Arguments, cd only takes 1 argument.\n");
    //     return -1;
    // }

    /*Attempts to change directory through system call, if it fails then print the strerror*/
    if(chdir(cd_path[1]) == -1) //changes directory to the cd_path directory
    {
        printf ("error: %s", strerror(errno));
        return -1;
    }
    //TEST: thecwd = cwd(); 
    //TEST: printf(thecwd); //Checks the current working Directory
    //TEST: printf("\n");
    return 0;
}

/*Opens a function given the command and arguments
Returns errors if error occurs in executing program or waiting for the child program*/
void open(char *cmd,char **arguments)
{
    pid_t child_pid, w;
    int status;
    char* thecwd;
    thecwd = cwd();

    PATHNODE *cur = head;

    child_pid = fork();

    //Current Working Directory Search
    if (child_pid == 0)
    {
        if(execv(cmd, arguments) == -1)
        {
            // Don't check execv() error, only need cmd not found
            exit(3);
        }
    }
    else 
    {
        do {
            w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
            if (w == -1) 
                printf ("error: %s", strerror(errno));
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        if(status == 0)
        {
            //TODO: printf("%s Finished Executing, Child pid is %d\n", cmd, child_pid);
            return;
        }
    }

    //Go through path's search
    while(cur != NULL)
    {   
        /*Attempts to change directory through system call, if it fails then print the strerror*/
        if(chdir(cur->path) == -1) //changes directory to the cd_path directory
        {
            // chdir() error
            printf ("error: %s", strerror(errno));
            return;
        }

        child_pid = fork();

        if (child_pid == 0)
        {
            if(execv(cmd, arguments) == -1)
            {
                // Don't check execv() error, only need cmd not found
                exit(3);
            }
        }
        else 
        {
            do {
                w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
                if (w == -1) 
                    printf ("error: %s", strerror(errno));
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        if(status == 0)
        {
            //TODO: printf("%s Finished Executing, Child pid is %d\n", cmd, child_pid);

            if(chdir(thecwd) == -1) //changes directory to the cd_path directory
            {
                // Failed to change back to old directory: chdir error
                printf ("error: %s", strerror(errno));
                return;
            }
            return; //Success, opened a Program properly
        }

        /*At end of while loop change directory back to original current working directory*/
        if(chdir(thecwd) == -1) //changes directory to the cd_path directory
        {
            // Failed to change back to old directory: chdir error
            printf ("error: %s", strerror(errno));
            return;
        }

        cur = cur->next;
    }

    printf("Command not found.\n");
    if(chdir(thecwd) == -1) //changes directory to the cd_path directory
    {
        // Failed to change back to old directory: chdir error
        printf ("error: %s", strerror(errno));
        return;
    }
    return;
}

int n_convert(char* cmd)
{
    int ia = cmd[1] - '0';
    int ib = cmd[2] - '0';
    int ic = cmd[3] - '0';
    int n = 0;

    //Checks for whether the string is actual integers or nulls/spaces
    if ((ia > 10) | (ia < 0) | (ib > 10) | ((ib < 0) && (ib != -48)) | (ic > 10) | ((ic < 0) && (ic != -48)))
    {
        printf("Command not found.\n");
        return n;
    }

    //If given just !h then convert first value to ones before processing
    if ((ib == -48) && (ic == -48))
    {
        ic = ia;
        ib = 0;
        ia = 0;
    }
    //If given just !ht then convert first value to tens and second to ones before processing
    else if ((ic == -48))
    {
        ic = ib;
        ib = ia;
        ia = 0;
    }

    //Convert to one single integer
    ia = ia*100;
    ib = ib*10;
    n = ia + ib + ic;

    //TEST: printf("n is: %d\n", n);

    //Check integer's bounds
    if ((n < 1)|(n > MAX_VALUE))
    {
        printf("Command not found.\n");
        n = 0;
    }

    return n;
}

/*Gets input buffer to the parser*/
char *getinput(char *buf, int len) 
{
    printf("%s ", PROMPT);
    return fgets(buf, len, stdin);
}

/*Shell Input Parser, gets instructions
Returns: 0 if exiting the shell, 1 if returning to shell */
int parseinput(char *buf) { //used to be a Constant
    if (strcmp(buf, "exit\n") == 0)
        return 0;
    else
      {
        push_hist(buf);
        char *cmd;
        char *arguments[22]; //Creates the argv array
        arguments[21] = NULL; //Initializes the 21st argument to be NULL for error checking
        char *pch; //temporary holder TODO: free these unused variables

        int i = 1;
        cmd = strtok (buf," \n");
        arguments[0] = cmd;
        /*Build the arguments array*/
        do {
            pch = strtok (NULL, " \n");
            arguments[i] = pch; 
            i++;
        } while ((pch != NULL) && (i < 22));


        /*If there are over 20 arguments, then 21st argument is not NULL, throw error and return to shell*/
        if (arguments[21] != NULL)
            {
                printf("error: Too many arguments.\n");
                return 1;
            }

        /*Determine which instruction was called and call appropriate function*/
        if (strcmp(cmd, "path") == 0)
            {   
                /*When there isn't any arguments then print the PATH*/
                if (arguments[1] == NULL)
                {
                    printPath();
                }
                else if (arguments[2] == NULL) //To handle case of null argument after path +
                {
                    printf("error: No path input.\n");
                }
                else if (strcmp(arguments[1], "+") == 0)
                {
                    addPath(arguments[2]);
                }
                else if (strcmp(arguments[1], "-") == 0)
                {
                    removePath(arguments[2]);
                }
                /*When there's any arguments other than + and -, print the PATH*/
                else
                {
                    printPath();
                }
            }
        else if (strcmp(cmd, "cd") == 0)
            {
                cd(arguments);
            }
        else if (strcmp(cmd, "history") == 0)
            {
                print_hist();
            }
        else if (strncmp(cmd, "!", 1) == 0)
            {
                if ((strlen(cmd) > 4)|(strlen(cmd) < 1))
                {
                    printf("Command not found.\n");
                }
                else
                {
                    int n = n_convert(cmd);
                    if (n != 0)
                    {
                        call_hist(n);
                    }
                    else
                    {
                        printf("Command not found.\n");
                    }
                }
            }
        else
            {
                open (cmd, arguments);
            }
      }
    return 1;
}

/*Main Shell, calls parseinput until notified to exit*/
int main(int argc, char *argv[]) {
    char buf[BUF_SIZE]; //TODO: confirm each argument with 512 character limit or total 512
    head = NULL;
    history_count = 0;
    history_head = NULL;
    history_tail = NULL;
    do {
        getinput(buf, sizeof(buf));
    } while (parseinput(buf) == 1);
    return 0;
}
