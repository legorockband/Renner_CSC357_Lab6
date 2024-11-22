#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 40271
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

void run_server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    //char *message = "Hello from server";

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept a connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    while(1){
        // Reset buffer for new data
        memset(buffer, 0, BUFFER_SIZE);
        int buff_read = read(new_socket, buffer, BUFFER_SIZE);
        
        // If read worked
        if(buff_read > 0){
            // Send message back to client
            send(new_socket, buffer, BUFFER_SIZE, 0);
            printf("Message sent to client\n");
        }
        else if(buff_read == 0){
            printf("Server: Client Requested Exit\n");
            break;
        }

    }
    // Close sockets
    close(new_socket);
    close(server_fd);
}

void run_client() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};
    char message[1024];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    while(1){
        printf("Enter Input (CTRL+D for exit): ");
        // Read Input from the user
        if(!fgets(message, sizeof(message), stdin)){
            // Set message to null
            printf("\nClient: Exit Detected\n");
            // Send exit request to server
            send(sock, "", 0,0);
            break;
        }
        message[strcspn(message, "\n")] = '\0';
        // Send data to server
        send(sock, message, strlen(message), 0);
        printf("Message sent to server\n");
        
        // Clear Buffer
        memset(buffer, 0, BUFFER_SIZE);

        // Read data from server
        if(read(sock, buffer, BUFFER_SIZE) > 0){
            printf("Server: %s\n", buffer);
        }
    }

    // Close socket
    close(sock);
}

int main() {
    pid_t childpid = fork();

    if (childpid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } 
    // Child process
    else if (childpid == 0) {
        sleep(1); // Have the server start first
        run_client();
    } 
    else {
        // Parent process: Run server
        run_server();

        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}


