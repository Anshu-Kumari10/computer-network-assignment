#define PORT 8080
#define BUFFER_SIZE 4096
void trim_string(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || str[len - 1] == ' ' || str[len - 1] == '\t')) {
        str[len - 1] = '\0';
        len--;
    }
    while (*str == ' ' || *str == '\t') {
        memmove(str, str + 1, strlen(str));
    }
}
void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        close(client_sock);
        exit(0);
    }
    char *student_id = strtok(buffer, "\n");
    char *file_data = strtok(NULL, "");

    if (!student_id || !file_data) {
        char *msg = "Record Not Found";
        write(client_sock, msg, strlen(msg));
        close(client_sock);
        exit(0);
    }

    trim_string(student_id);
    char line_copy[BUFFER_SIZE];
    strcpy(line_copy, file_data);

    char *line = strtok(line_copy, "\n");
    int found = 0;
    char found_record[256];

    while (line != NULL) {
        if (strlen(line) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }

        char temp_line[256];
        strcpy(temp_line, line);
        char *id_token = strtok(temp_line, "|");
        
        if (id_token != NULL) {
            trim_string(id_token);

            if (strcmp(id_token, student_id) == 0) {
                found = 1;
                trim_string(line);
                snprintf(found_record, sizeof(found_record), "Record Found: %s", line);
                break;
            }
        }
        line = strtok(NULL, "\n");
    }
    if (found) {
        write(client_sock, found_record, strlen(found_record));
    } else {
        char *msg = "Record Not Found";
        write(client_sock, msg, strlen(msg));
    }

    close(client_sock);
    exit(0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client_sock);
            continue;
        }

        if (pid == 0) {
            close(server_sock); 
            handle_client(client_sock);
        } else {
        
            close(client_sock);
            while (waitpid(-1, NULL, WNOHANG) > 0);
        }
    }

    close(server_sock);
    return 0;
}