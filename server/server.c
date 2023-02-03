#include <arpa/inet.h>  // htons(), inet_addr()
#include <dirent.h>     // opendir(), readdir(), closedir()
#include <errno.h>      // errno
#include <netinet/in.h> // inet_addr(), bind()
#include <signal.h>     // signal()
#include <stdbool.h>    // bool
#include <stdio.h>
#include <stdlib.h>     // strtol()
#include <string.h>     // bzero(), strlen(), strcmp(), strcpy(), strtok(), strrchr(), memcmp()
#include <sys/socket.h> // socket(), inet_addr(), bind(), listen(), accept(), recv(), send()
#include <sys/types.h>  // socket()
#include <unistd.h>     // close()
#include <ctype.h>
#include <sys/stat.h> //mkdir()
#include <sys/wait.h> //waitpid();
#include <libgen.h>

#include "account.h"

#define MaxClient 20
#define MAX 1024
#define TRUE 1
#define FALSE 0
#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void sighandler(int signum)
{
  int stat;
  while (waitpid(-1, &stat, WNOHANG) > 0)
    ; ///* Wait the child process terminate */
  printf("\nChild matching this PID is teminated\n");
}

int main(int argc, const char *argv[])
{
  if (argc < 2)
  {
    printf("\nNot Enough Parameters ...\n");
    exit(1);
  }

  char filename[] = "account.txt";

  // Declare a socket
  int sfd;
  SOCKADDR_IN saddr, caddr;
  int clen = sizeof(caddr);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(atoi(argv[1]));
  saddr.sin_addr.s_addr = INADDR_ANY;
  pid_t pid;

  char client_choice[10];
  int bytes_received, bytes_sent;
  char username[MAX] = {0}, password[MAX] = {0};
  char *notify;

  node_a *account;
  node_a *account_list = getListNode(filename);

  int choice;
  while (TRUE)
  {
    printf("\n============MENU===========\n");
    printf("|1. Start server           |\n");
    printf("|2. Update client          |\n");
    printf("|3. Delete client          |\n");
    printf("|4. Exit                   |\n");
    printf("===========================\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
      // Start server
      {
        // Creating socket
        sfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sfd == -1)
        {
          perror("\nERROR! Create socket failed ...\n");
          return FALSE;
        }
        // Binding socket
        int error = bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));
        if (error == -1)
        {
          perror("\nERROR! Binding failed ...\n");
          return FALSE;
        }
        // Listening
        if (listen(sfd, MaxClient) != -1)
        {
          signal(SIGCHLD, sighandler);
          printf("Server is listening...\n");
          while (TRUE)
          {
            int cfd;
            cfd = accept(sfd, (SOCKADDR *)&caddr, &clen);
            if (cfd != -1)
            {
              /* fork() is called in child process */
              pid = fork();
              if (pid == 0)
              {
                while (TRUE)
                {
                  
                  if ((bytes_received = recv(cfd, client_choice, sizeof(client_choice), 0)) == -1)
                  {
                    perror("\nERROR! Can't receive client's choice ...\n");
                    exit(1);
                  }
                  // client_choice[bytes_received] = '\0';
                  switch (atoi(client_choice))
                  {
                  case 1:
                  {
                    char *message = malloc(sizeof(char) * MAX);
                    char *current_path = malloc(sizeof(char) * MAX);
                    int done = 0, check = 0;
                    while (done == 0)
                    {
                      memset(username, '\0', MAX);
                      if ((bytes_received = recv(cfd, username, MAX, 0)) <= 0)
                      {
                        printf("\nConnection ennded\n");
                        break;
                      }
                      username[bytes_received] = '\0';
                      // Check username
                      if (account = findNode(account_list, username))
                      {
                        notify = "1";
                        if ((bytes_sent = send(cfd, notify, sizeof(notify), 0)) <= 0)
                        {
                          printf("\nConnection ended\n");
                          break;
                        }
                      }
                      else
                      {
                        notify = "0";
                        if ((bytes_sent = send(cfd, notify, sizeof(notify), 0)) <= 0)
                        {
                          printf("\nConnection ended\n");
                          break;
                        }
                        continue;
                      }

                      // Check pass
                      while (check == 0)
                      {
                        memset(password, '\0', MAX);
                        if ((bytes_received = recv(cfd, password, MAX, 0)) <= 0)
                        {
                          printf("\nConnection ended\n");
                          break;
                        }
                        password[bytes_received] = '\0';
                        if (strcmp(account->pass, password) == 0)
                        {
                          notify = "1";
                          done = 1;
                          check = 1;
                        }
                        else
                          notify = "0";
                        if ((bytes_sent = send(cfd, notify, sizeof(notify), 0)) <= 0)
                        {
                          printf("%s is try to logged in\n", username);
                          break;
                        }
                        break;
                      }
                    }
                    strcpy(current_path, "~/");
                    // printf("\n%s\n", account->folder);
                    strcat(current_path, account->folder);
                    printf("\n%s account is accessing in %s\n", username, current_path);

                    // Sent current_path to client
                    if (send(cfd, current_path, MAX, 0) <= 0)
                    {
                      printf("\nERROR! Can't send current path ...\n");
                      exit(1);
                    }
                  }
                  break;

                  default:
                    break;
                  }
                }
              }
              if (pid < 0)
              {
                perror("\nERROR! ");
                return 1;
              }
              close(cfd);
            }
            else
            {
              perror("\nERROR! Can't accept ...");
              continue;
            }
          }
        }
        else
        {
          perror("\nERROR! Can't listen ...");
          return 0;
        }
      }
      break;

    default:
      break;
    }
  }
}