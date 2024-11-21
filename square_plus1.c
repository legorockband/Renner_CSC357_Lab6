#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(){

    int status1, status2;
    int pipe_p_c1[2], pipe_c1_c2[2], pipe_c2_p[2];
    pid_t child1, child2;
   
    // Create pipe from parent to child 1 
    if(pipe(pipe_p_c1) == - 1){
        perror("Parent to Child 1 pipe failed");
        exit(EXIT_FAILURE);
    }
    // Create pipe from child 1 to child 2
    if(pipe(pipe_c1_c2) == - 1){
        perror("Child 1 to Child 2 pipe failed");
        exit(EXIT_FAILURE);
    }
    // Create pipe from child 2 to parent
    if(pipe(pipe_c2_p) == - 1){
        perror("Child 2 to Parent pipe failed");
        exit(EXIT_FAILURE);
    }
   
    // Create child 1
    child1 = fork();
    if(child1 == -1){
        perror("Child1 fork failed");
        exit(EXIT_FAILURE);
    }
    // Child 1 Process
    if(child1 == 0){
        close(pipe_p_c1[1]);    // Close write for parent to child1
        close(pipe_c1_c2[0]);   // Close Read for child1 to 2

        //Close unused pips
        close(pipe_c2_p[0]);
        close(pipe_c2_p[1]);

        int buffer;        
        // Read the value from the parent into child1 
        if(read(pipe_p_c1[0], &buffer, sizeof(int)) == -1){
            perror("Child 1: Read failed");
            exit(EXIT_FAILURE);
        }
        // Square the val from the parent
        buffer *= buffer;
        
        // Write the value to child2
        if(write(pipe_c1_c2[1], &buffer, sizeof(int)) == -1){
            perror("Child 1: Write Failed");
            exit(EXIT_FAILURE);
        }
        close(pipe_p_c1[0]);       
        close(pipe_c1_c2[1]);
        exit(0);
    }
    // Back in Parent
    else{
        child2 = fork();
        if(child2 == -1){
            perror("Child2 fork failed");
            exit(EXIT_FAILURE);
        }
        // Child 2 process
        if(child2 == 0){
            
            close(pipe_c1_c2[1]);   // Close write from child1 to 2    
            close(pipe_c2_p[0]);    // Close read from child2 to parent

            //Not used pipes
            close(pipe_p_c1[0]);
            close(pipe_p_c1[1]);

            // Child 1 sends child 2 val 
            int buffer;        
            if(read(pipe_c1_c2[0], &buffer, sizeof(int)) == -1){
                perror("Child 2: Read failed");
                exit(EXIT_FAILURE);
            }
            // Add 1 to buffer
            buffer += 1;
        
            // Write value from child2 to parent
            if(write(pipe_c2_p[1], &buffer, sizeof(int)) == -1){
                perror("Child 2: Write Failed");
                exit(EXIT_FAILURE);
            }
            close(pipe_c1_c2[0]);
            close(pipe_c2_p[1]);      
            exit(0);
        }
    
        // Back in Parent
        else{
            close(pipe_p_c1[0]);    // Close the read end of parent to child1 pipe 
            close(pipe_c2_p[1]);    // Close write end of child2 to parent
            
            //Unused Pipes
            close(pipe_c1_c2[0]);
            close(pipe_c1_c2[1]);

            int val;
            while(1){
                printf("Enter an Integer (CRTL+D to exit): ");
                if(scanf("%d", &val) != 1){
                    printf("hello\n");
                    break;
                }
                // Write val to the child 1    
                if(write(pipe_p_c1[1], &val, sizeof(int)) == -1){
                    perror("Write Failed");
                    exit(EXIT_FAILURE);
                }
            
                // Read the final value 
                if(read(pipe_c2_p[0], &val, sizeof(int)) == -1){
                    perror("Child 2: Read failed");
                    exit(EXIT_FAILURE);
                }
            
                printf("Square + 1: %d\n", val);                      
            }
            
            // Close the read side
            close(pipe_p_c1[1]);
            close(pipe_c2_p[0]);
       
            // Wait for child 1 
            if(waitpid(child1, &status1, 0) == -1){
                perror("Child 1: Wait Failed");
                exit(EXIT_FAILURE);
            }
            // Wait for child 2 to finish to get new value
            if(waitpid(child2, &status2, 0) == -1){
                perror("Child 2: Wait Failed");
                exit(EXIT_FAILURE);
            }
        }
    
    }

    return 1;
}












