// Universidade Federal de São Carlos
// Departamento de Computação

// Criação de um programa shell simplificado
// Disciplina: Sistemas Operacionais 2
// Doscente: Prof. Dr. Helio Crestana Guardia

// Alunos:
// Matheus de Brito Soares Porto (744348)
// Vítor Hugo Guilherme (744359)

// Compilar: $ gcc shell.c -o shell
// Executar: $ ./shell

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>


#define BUFFER_SIZE 999999

#define FORK_ERROR -1
#define FORK_CHILD 0

#define BACKGROUND 0
#define FOREGROUND 1

#define DEF_STDIN 0
#define DEF_STDOUT 1

#define ARGS_LIST_SIZE 20

typedef struct process {
  int stdin, stdout, stderr;  /* standard i/o channels */
  int place; // 0 -> background, 1-> foreground
  char *arguments[ARGS_LIST_SIZE]; // args list
  int argumentsIndex;
  int jobNumber;
} process;

const char delim[] = " ";  // Command delimiter

const char spipe = '|';  
const char sbg = '&';  
const char sout = '>'; 
const char empty = '\n';

void readCommand(char*);
int runCommand(char*,int);
process createProcess(int, int,int);
int launchProcess(process);
void draw();

int jobs = 0;

int main(int argc, char const *argv[]) {
    char commandBuffer[BUFFER_SIZE];

    draw();
    
    while (1){
        
        readCommand(commandBuffer);
        runCommand(commandBuffer, DEF_STDIN);

        // Clear buffer
        commandBuffer[0] = '\0';
    }
    return 0;
}

/* Reads a command from the terminal */
void readCommand(char *buffer) {
    printf("$ ");
    fgets(buffer, BUFFER_SIZE, stdin);
}

/* Run a command
   Returns 0 - success / 1- error 
*/
int runCommand(char* commands, int stdin) {

    int isPipe =  0;
    int pipefd[2];

    char *ptr = strtok(commands, delim);

    if(*ptr == empty) 
        return 0;

    process p = createProcess(stdin, DEF_STDOUT, FOREGROUND);

    // Loop through next commands/args 
    while(ptr != NULL) { 
        if (*ptr == spipe) {// | - pipe
            // Create pipe, change job's stdout to pipe and check the next program
            if (pipe(pipefd)==-1) {
                perror("Failed to create Pipe");
                exit(0);

            }
            p.stdout = pipefd[1];
            
           	isPipe = 1;
           	break;
         
        } else if (*ptr == sbg) {// & - background
            p.place = BACKGROUND;

        } else if (*ptr == sout) {// > stdout
             ptr = strtok(NULL, delim);
             ptr[strcspn(ptr, "\n")] = 0;
             p.stdout = open(ptr, O_RDWR);
             
        } else {// arg or program 
            ptr[strcspn(ptr, "\n")] = 0; // removing \n
            p.arguments[p.argumentsIndex] = ptr;
            p.argumentsIndex++;

        }

        ptr = strtok(NULL, delim);  // Next command/arg
    }

    // Close args list and launch the process
    p.arguments[p.argumentsIndex] = NULL;
    p.argumentsIndex++;
    launchProcess(p);
    
    if ( isPipe ){
    	ptr = strtok(NULL, delim);
        runCommand(ptr, pipefd[0]);
    }

    return 0;
}

process createProcess(int stdin, int stdout, int place) {
    process p;
    p.stdin = stdin;
    p.stdout = stdout;
    p.stderr = 2;
    p.place = place;
    p.argumentsIndex = 0;
    p.jobNumber = jobs;
    jobs++;
    return p;
}

int launchProcess(process p){
    pid_t pid_filho;

    pid_filho = fork();

    if (pid_filho == FORK_ERROR){ 
	  	perror("Erro na execucao do programa");
		exit(0);
	} 
     
    if (pid_filho == FORK_CHILD) {    
        // Clean errors
        errno = 0;

        if (p.stdout != DEF_STDOUT){
            dup2(p.stdout, STDOUT_FILENO);
            close(p.stdout);
        }

        if (p.stdin != DEF_STDIN){
            dup2(p.stdin, STDIN_FILENO);
            close(p.stdin); 
        }
        
        // Execute the program  
        execvp(p.arguments[0], p.arguments);

        printf("%s \n", strerror(errno));
		exit(1);

	} else {  // Parent process
      
        if (p.place == BACKGROUND){ // Send child process to background
            printf("[%d] %d\n", p.jobNumber, pid_filho);
            kill(pid_filho,SIGTTIN);
            kill(pid_filho, SIGCONT);

        } else {  // Wait child process return
            waitpid(pid_filho, NULL, 0);
        }

	}

	return 0;
    
}

void draw() {
    printf(" _____                             _____  _            _  _ \n");
    printf("|  __ \\                           /  ___|| |          | || |\n");
    printf("| |  \\/  __ _  _ __   ___   ___   \\ `--. | |__    ___ | || |\n");
    printf("| | __  / _` || '_ \\ / __| / _ \\   `--. \\| '_ \\  / _ \\| || |\n");
    printf("| |_\\ \\| (_| || | | |\\__ \\| (_) | /\\__/ /| | | ||  __/| || |\n");
    printf("\\_____/ \\__,_||_| |_||___/ \\___/  \\____/ |_| |_| \\___||_||_|\n");

    printf("                                 ___ \n");
    printf("                                /   >\n");
    printf("                               /   /\n");
    printf("                         _____/   /\n");
    printf("                        <        /\n");
    printf("                         \\_    _/ \n");
    printf("                           |   |\n");
    printf("                           |   |\n");
    printf("                           ^   ^\n");
}

