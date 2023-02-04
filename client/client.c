#include <arpa/inet.h>  // htons(), inet_addr()
#include <errno.h>      // errno
#include <netinet/in.h> // inet_addr()
#include <stdbool.h>    // bool
#include <stdio.h>
#include <stdlib.h>     // strtol()
#include <string.h>     // bzero(), strlen(), strcmp(), strcpy(), strtok(), memcmp()
#include <sys/socket.h> // socket(), inet_addr(), connect(), recv(), send()
#include <sys/types.h>  // socket()
#include <unistd.h>     // close()
#include <sys/stat.h>

#define MAX 1024
#define TRUE 1
#define FALSE 0

typedef struct sockaddr_in SOCKADDR_IN;

void command_cprocess(int socket, char *command, char **path)
{
  char *full_command = malloc(strlen(command) + 1);
  strcpy(full_command, command);
  char *delim = " ";
  char *first_command = strtok(full_command, delim); // first token
  char *context = strtok(NULL, delim);               // second token, third token

  if (strcmp(first_command, "ls") == 0)
  {
    client_ls(socket, command);
  }
}

void client_ls(int socket, char *message)
{
  if (send(socket, message, MAX, 0) <= 0)
  {
    fprintf(stderr, "can't send packet");
    perror("");
    return;
  }
  char response[1024];
  if (recv(socket, response, sizeof(response), 0) == -1)
  {
    fprintf(stderr, "can't receive packet");
    perror("");
    return;
  }
  // if (begin_with(response, "@"))
  // {
  //   printf("Server Error: %s\n", &response[1]);
  // }
  else
  {
    printf("%s", response);
  }
}

int main(int argc, const char *argv[])
{
  if (argc < 3)
  {
    printf("Not Enough Parameters ...\n");
    exit(1);
  }

  char choice[10], username[MAX] = {0}, password[MAX] = {0},
                   notify[10], command[MAX];
  char *path;
  int bytes_received, bytes_sent;

  // Declare socket
  int sfd;
  SOCKADDR_IN saddr;
  // memset(&saddr, '0', sizeof(saddr));
  bzero(&saddr, sizeof(saddr)); // set size of server_addr to O
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(atoi(argv[2]));
  saddr.sin_addr.s_addr = inet_addr(argv[1]);

  if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("\nERROR! Failed to create The Socket ... \n");
    exit(1);
  }

  if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
  {
    perror("\nERROR! Failed to Connect to The Server ... \n");
    exit(1);
  }

  int option;
  while (TRUE)
  {
    printf("\n============MENU===========\n");
    printf("|1. Login                  |\n");
    printf("|2. Create account         |\n");
    printf("|3. Exit                   |\n");
    printf("===========================\n");
    printf("Enter your choice: ");
    scanf("%d", &option);
    switch (option)
    {
    case 1:
      // Login
      {
        strcpy(choice, "1");
        if (send(sfd, choice, 10, 0) == -1)
        {
          perror("\nERROR! Failed to send The client Choice ... \n");
          exit(1);
        }
        int done = 0;
        while (done == 0)
        {
          // memset(choice, '\0', sizeof(choice));
          // memset(username, '\0',MAX);
          printf("%s\n", username);
          puts("\nPlease enter your account");
          // Check username
          printf("Username: ");
          fgets(username, MAX, stdin);
          username[strcspn(username, "\n")] = '\0'; // the last character in this string to \0
          if (send(sfd, username, MAX, 0) <= 0)
          {
            printf("\nConnection ended\n");
            return 0;
          }
          if ((bytes_received = recv(sfd, notify, sizeof(notify), 0)) <= 0)
          {
            printf("\nERROR! Can't receive data\n");
            return 0;
          }
          notify[bytes_received] = '\0';
          if (strcmp(notify, "0") == 0)
          {
            printf("\nAccount not existed. Please try again\n");
            continue;
          }

          // Check password
          printf("Password: ");
          fgets(password, MAX, stdin);
          password[strcspn(password, "\n")] = '\0'; // the last character in this string to \0
          if (send(sfd, password, MAX, 0) <= 0)
          {
            printf("\nConnection ended\n");
            return 0;
          }

          memset(notify, '\0', sizeof(notify));
          if ((bytes_received = recv(sfd, notify, sizeof(notify), 0)) <= 0)
          {
            printf("\nERROR! Can't receive data\n");
            return 0;
          }
          notify[bytes_received] = '\0';
          if (strcmp(notify, "0") == 0)
          {
            printf("\nPassword is incorrect. Please try again\n");
            continue;
          }
          if (strcmp(notify, "1") == 0)
          {
            printf("\nWelcome!!!\n-----------------------------\n");
          }
          done = 1;
        }

        if ((bytes_received = recv(sfd, path, MAX, 0)) <= 0)
        {
          printf("\nERROR! Can't receive path\n");
          return 0;
        }
        path[bytes_received] = '\0';

        while (TRUE)
        {
          // memset(command, '\0', sizeof(command));
          printf("~/%s$ ", path);
          // scanf("%s", command);
          fgets(command, MAX, stdin);
          // printf("%s\n", command);
          if (strcmp(command, "\n") == 0)
          {
            continue;
          }
          if (strcmp(command, "exit\n") == 0)
          {
            break;
          }
          if ((strlen(command) > 0) && (command[strlen(command) - 1] == '\n'))
          {
            command[strlen(command) - 1] = '\0';
            send(sfd, command, MAX, 0);
          }
          command_cprocess(sfd, command, &path);
        }
      }
      break;

    default:
      printf("Choose again\n");
      break;
    }
  }
}
