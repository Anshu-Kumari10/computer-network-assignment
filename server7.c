// Roll: 24105142002 Service Type: C d = 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 30002
#define ROLL "24105142002"
#define SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server, client;
    socklen_t client_len;
    char buffer[SIZE], response[SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server,
             sizeof(server)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("Server running on port %d\n", PORT);

    while (1) {
        client_len = sizeof(client);
        memset(buffer, 0, sizeof(buffer));

        int n = recvfrom(sockfd, buffer, SIZE - 1, 0,
                         (struct sockaddr *)&client,
                         &client_len);

        if (n < 0) {
            perror("Receive failed");
            continue;
        }

        buffer[n] = '\0';

        printf("\nClient: %s:%d\n",
               inet_ntoa(client.sin_addr),
               ntohs(client.sin_port));

        char *separator = strchr(buffer, '#');

        if (separator == NULL) {
            strcpy(response, "ERROR: Invalid message format");
        } else {
            *separator = '\0';

            if (strcmp(buffer, ROLL) != 0) {
                strcpy(response, "ERROR: Unauthorized client");
            } else {
                char *text = separator + 1;

                printf("Received message: %s\n", text);

                if (strcmp(text, "EXIT") == 0) {
                    strcpy(response, "Server shutting down");
                    sendto(sockfd, response, strlen(response), 0,
                           (struct sockaddr *)&client, client_len);
                    printf("Exit command received.\n");
                    break;
                }

                int len = strlen(text);
                int vowels = 0, consonants = 0, digits = 0;

                for (int i = 0; text[i] != '\0'; i++) {
                    unsigned char ch = (unsigned char)text[i];

                    if (isalpha(ch)) {
                        ch = tolower(ch);

                        if (ch == 'a' || ch == 'e' ||
                            ch == 'i' || ch == 'o' ||
                            ch == 'u') {
                            vowels++;
                        } else {
                            consonants++;
                        }
                    } else if (isdigit(ch)) {
                        digits++;
                    }
                }

                snprintf(response, sizeof(response),
                         "LEN=%d;VOWELS=%d;CONSONANTS=%d;DIGITS=%d",
                         len, vowels, consonants, digits);
            }
        }

        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&client, client_len);

        printf("Sent response: %s\n", response);
    }

    close(sockfd);
    return 0;
}