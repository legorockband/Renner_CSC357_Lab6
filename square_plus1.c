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
        // Child only wants to read from pipe, closes write side
        close(pipe_p_c1[1]);    

        // Parent sends child val 
        int buffer[10];        
        if(read(pipe_p_c1[0], buffer, sizeof(int)) == -1){
            perror("Child 1: Read failed");
            exit(EXIT_FAILURE);
        }
        // Close the read side
        close(pipe_p_c1[0]);       
        
        // Square the val
        buffer[0] *= buffer[0];
        
        // Write: child1 to child2
        close(pipe_c1_c2[0]);
        if(write(pipe_c1_c2[0], buffer, sizeof(int)) == -1){
            perror("Child 1: Write Failed");
            exit(EXIT_FAILURE);
        }
        close(pipe_c1_c2[1]);
        exit(9);
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
            
            // Child 2 only wants to read from pipe, closes write side
            close(pipe_c1_c2[1]);    

            // Child 1 sends child 2 val 
            int buffer[10];        
            if(read(pipe_c1_c2[0], buffer, sizeof(int)) == -1){
                perror("Child 2: Read failed");
                exit(EXIT_FAILURE);
            }
            // Close the read side
            close(pipe_c1_c2[0]);
            
            // Add 1 to buffer
            buffer[0] += 1;
        
            // Write: child2 to parent
            close(pipe_c2_p[0]);
            if(write(pipe_c2_p[0], buffer, sizeof(int)) == -1){
                perror("Child 2: Write Failed");
                exit(EXIT_FAILURE);
            }
            close(pipe_c2_p[1]);      
            exit(10);
        }
    
        // Back in Parent
        else{
            // Close the read end of parent to child1 pipe
            close(pipe_p_c1[0]);
            int val[10];
            printf("Enter Integer: ");
            scanf("%d", &val[0]);
            
            // Write val to the child 1    
            if(write(pipe_p_c1[1], val, sizeof(int)) == -1){
                perror("Write Failed");
                exit(EXIT_FAILURE);
            }
            close(pipe_p_c1[1]);
   
            // Wait for child 1 
            if(waitpid(child1, &status1, 0) == -1){
                perror("Child 1: Wait Failed");
                exit(EXIT_FAILURE);
            }
            printf("First child exited with status %d\n", WEXITSTATUS(status1));
            // Wait for child 2 to finish to get new value
            if(waitpid(child2, &status2, 0) == -1){
                perror("Child 2: Wait Failed");
                exit(EXIT_FAILURE);
            }
            printf("Second child exited with status %d\n", WEXITSTATUS(status2));
            
            // Close write end 
            close(pipe_c2_p[1]);
            
            int final_val[10];        
            if(read(pipe_c2_p[0], final_val, sizeof(int)) == -1){
                perror("Child 2: Read failed");
                exit(EXIT_FAILURE);
            }
            // Close the read side
            close(pipe_c2_p[0]);
            
            printf("Square + 1: %d\n", final_val[0]);                      

        }
    
    }


    return 1;
}












