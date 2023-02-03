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

int main(int argc, const char *argv[])
{
  if (argc < 3)
  {
    printf("Not Enough Parameters ...\n");
    exit(1);
  }

  char choice[10], username[MAX], password[MAX],
      notify[10], path[MAX], command[MAX];
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
            printf("\nWelcome!!!\n");
          }
          done = 1;
        }

        if ((bytes_received = recv(sfd, path, MAX, 0)) <= 0)
        {
          printf("\nERROR! Can't receive path\n");
          return 0;
        }
        path[bytes_received] = '\0';
        printf("\n%s$ \n", path);
        printf("\n DO st: ");
        fgets(command, MAX, stdin);
      }
      break;

    default:
      printf("Choose again\n");
      break;
    }
  }
}
