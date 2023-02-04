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

int dem(char *s, char t);
void sighandler(int signum);
void server_ls(char *response, char **current_path);
void server_cd(int socket, char *open_dir, char *response, char **current_path);
void server_download(int socket, char *target_file, char **current_path);
void command_sprocess(int socket, char *full_command, char **current_path);

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
  char command[MAX];
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
                    // strcpy(current_path, "~/");
                    // printf("\n%s\n", account->folder);
                    strcpy(current_path, account->folder);
                    printf("\n%s account is accessing in ~/%s\n", username, current_path);

                    // Sent current_path to client
                    if (send(cfd, current_path, MAX, 0) <= 0)
                    {
                      printf("\nERROR! Can't send current path ...\n");
                      exit(1);
                    }

                    while (TRUE)
                    {
                      if ((bytes_received = recv(cfd, command, MAX, 0)) <= 0)
                      {
                        printf("%s Terminated\n", username);
                        close(cfd);
                        break;
                      }
                      printf("%s commanded: %s\n", username, command);
                      command_sprocess(cfd, command, &current_path);
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

void sighandler(int signum)
{
  int stat;
  while (waitpid(-1, &stat, WNOHANG) > 0)
    ; ///* Wait the child process terminate */
  printf("\nChild matching this PID is teminated\n");
}

// int respond(int socket, char response[])
// {
//   if ((send(socket, response, strlen(response) + 1, 0)) == -1)
//   {
//     fprintf(stderr, "Can't send packet\n");
//     return errno;
//   }
//   return 0;
// }

void command_sprocess(int socket, char *full_command, char **current_path)
{
  // Prepare
  char *delim = " "; // dấu cách phân định command
  // strtok: Chia 1 chuỗi dài thành các chuỗi nhỏ được phân biệt riêng rẽ bởi các ký tự đặc biệt được chọn.
  char *command = strtok(full_command, delim); // lấy phần đầu của command (ls,cd,...)
  char *context = strtok(NULL, delim);         // phần sau của command
  char *response = malloc(sizeof(char) * 1024);

  if (strcmp(command, "ls") == 0)
  {
    server_ls(response, current_path);
    // respond(socket, response);
    send(socket, response, MAX, 0);
  }
  else if (strcmp(command, "cd") == 0)
  {
    server_cd(socket, context, response, current_path);
    send(socket, response, MAX, 0);
  }
  else if (strcmp(command, "cd") == 0)
  {
    server_cd(socket, context, response, current_path);
    send(socket, response, MAX, 0);
  }
  else if (strcmp(command, "download") == 0)
  {
    server_download(socket, context, current_path);
    // send(socket, response, MAX, 0);
  }
  free(response);
}

void server_ls(char *response, char **current_path)
{
  // *current_path = "test1/folder2";
  struct dirent *d;
  DIR *dh = opendir(*current_path);
  // printf("%s\n", *current_path);
  if (!dh)
  {
    if (errno = ENOENT)
    {
      // If the directory is not found
      perror("Directory doesn't exist");
    }
    else
    {
      // If the directory is not readable then throw error and exit
      perror("Unable to read directory");
    }
    exit(EXIT_FAILURE);
  }
  memset(response, '\0', sizeof(response));
  while ((d = readdir(dh)) != NULL)
  {
    strcat(response, d->d_name);
    strcat(response, "\n");
  }
  strcat(response, "\n");
  // printf("\n");
}

void server_cd(int socket, char *open_dir, char *response, char **current_path)
{
  // Handle empty arg and . and ..
  // xử lý đối số trống . và ..

  // neu khong co ten file (vd: cd ) => thong bao no directory given
  if (open_dir == NULL)
  {
    strcpy(response, "@no directory given");
    return;
    // neu "cd ."=> response = thu muc hien tai
  }
  else if (strcmp(open_dir, ".") == 0)
  {
    strcpy(response, *current_path);
    return;
    // "cd .."
  }
  else if (strcmp(open_dir, "..") == 0)
  {
    // Check Root
    // neu current_path = . => thong bao da den goc
    if (dem(*current_path, '/') == 0)
    {
      strcpy(response, "@already reached root");
      return;
    }
    // Truncate current path - cắt bớt đường dẫn hiện tại
    // failed
    char *trunct = strrchr(*current_path, '/');
    strcpy(trunct, "\0");
    strcpy(response, *current_path);
    return;
  }

  // Open Directory
  DIR *open_dir_fd;
  if ((open_dir_fd = opendir(*current_path)) == NULL)
  {
    strcpy(response, "@can't open");
    strcat(response, *current_path);
    fprintf(stderr, "(%d) Can't open %s", socket, *current_path);
    perror("");
    return;
  }

  // Check existance
  bool exist = false;
  struct dirent *dir_entry = NULL;
  // tìm từ đầu đến cuối thư mục hiện tại (current_path)
  while ((dir_entry = readdir(open_dir_fd)) != NULL && !exist)
  {
    if (dir_entry->d_type == DT_DIR && strcmp(dir_entry->d_name, open_dir) == 0)
    {
      // Build new path name
      char *new_path = malloc(strlen(*current_path) + strlen(dir_entry->d_name) + 2);
      strcpy(new_path, *current_path);
      strcat(new_path, "/");
      strcat(new_path, dir_entry->d_name);

      // Store current path ~ đổi current_path
      free(*current_path);
      *current_path = malloc(strlen(new_path));
      strcpy(*current_path, new_path);
      strcpy(response, *current_path);
      printf("========%s========\n", *current_path);
      free(new_path);
      exist = true;
    }
  }
  if (!exist)
  {
    strcpy(response, "@");
    strcat(response, *current_path);
    strcat(response, "/");
    strcat(response, open_dir);
    strcat(response, " does not exist");
  }

  // Close Directory
  if (closedir(open_dir_fd) < 0)
  {
    fprintf(stderr, "(%d) Directory Close Error", socket);
    perror("");
  }
}

int dem(char *s, char t)
{
  int dem = 0;
  for (int i = 0; i <= strlen(s); i++)
  {
    if (s[i] == t)
      dem = dem + 1;
  }
  return dem;
}

int respond(int recfd, char response[])
{
  if ((send(recfd, response, strlen(response) + 1, 0)) == -1)
  {
    fprintf(stderr, "Can't send packet\n");
    return errno;
  }
  return 0;
}

int is_regular_file(const char *path)
{
  struct stat path_stat;
  stat(path, &path_stat);
  return S_ISREG(path_stat.st_mode);
}

void server_download(int socket, char *target_file, char **current_path)
{

 // Build Path 
  char *full_path = malloc(strlen(*current_path) + strlen(target_file) + 2);
  strcpy(full_path, *current_path);
  strcat(full_path, "/");
  strcat(full_path, target_file);
  // char *full_path = "./hello/new/file.txt";

  if (is_regular_file(full_path) == 0)
  {

    respond(socket, "@Cannot download a folder or file not existed");
    fprintf(stderr, "%s is a folder or not existed\n", full_path);
    // fclose(fd);
    return;
  }
  // Initialize File Descriptor, Buffer
  FILE *fd;
  if ((fd = fopen(full_path, "rb")) == NULL)
  {
    respond(socket, "@file open error");
    fprintf(stderr, "Can't open %s", full_path);
    perror("");
    return;
  }

  char buffer[1024];
  ssize_t chunk_size;

  // Notify File Size
  fseek(fd, 0L, SEEK_END);           // 0L => 0 long means 0.000KB/Mb file, SEEK_END => end of file;
  sprintf(buffer, "%ld", ftell(fd)); // ftell(fd) current position
  ssize_t byte_sent = send(socket, buffer, strlen(buffer) + 1, 0);
  if (byte_sent == -1)
  {
    fprintf(stderr, "Can't send packet");
    perror("");
    fclose(fd);
    return;
  }
  fseek(fd, 0L, SEEK_SET);

  // Wait for client to be ready
  ssize_t byte_received = recv(socket, buffer, sizeof(buffer), 0);
  if (byte_received == -1)
  {
    fprintf(stderr, "Can't receive packet");
    perror("");
    fclose(fd);
    return;
  }

  // Start Transmission
  while ((chunk_size = fread(buffer, 1, sizeof(buffer), fd)) > 0)
  {
    ssize_t byte_sent = send(socket, buffer, chunk_size, 0);
    if (byte_sent == -1)
    {
      fprintf(stderr, "Can't send packet");
      perror("");
      fclose(fd);
      return;
    }
  }
  printf("Transmited: %s\n", target_file);
  fclose(fd);
}
