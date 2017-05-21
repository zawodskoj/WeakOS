#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

void accept_cmd(char *cmd, int cmd_size) {
    printf("# ");
    fgets(cmd, cmd_size - 1, stdin);
    cmd[cmd_size - 1] = 0;
}

char* get_token_and_advance(char ** cmd) {
    if (!*cmd || !**cmd) return 0;
    
    char *start = *cmd;
    while (**cmd && **cmd != '\n')
        if (**cmd == ' ') {
            **cmd = 0;
            (*cmd)++;
            if (strlen(start) == 0) return get_token_and_advance(cmd);
            return start;
        } else (*cmd)++;
    if (**cmd == '\n') **cmd = 0;
    if (strlen(start) == 0) return 0;
    return start;
}

int listdir(const char *path) {
  struct dirent *entry;
  DIR *dp;

  dp = opendir(path);
  if (dp == NULL) {
    perror("opendir");
    return -1;
  }

  while((entry = readdir(dp)))
    puts(entry->d_name);

  closedir(dp);
  return 0;
}

int main(int argc, char**argv) {
    printf("WeakOS Shell v0.1\n");

    const int cmd_size = 256;
    char *cmd = malloc(cmd_size);
    
    while (1) {
        
        accept_cmd(cmd, cmd_size);
        
        char *token;
        int n = 0;
        while ((token = get_token_and_advance(&cmd))) {
            if (!strcmp(token, "ls")) {
                listdir(".");
            }
        }
    }
}
