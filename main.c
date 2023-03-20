#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<fcntl.h>
#include<stdbool.h>

extern char **environ;
char *outFile = NULL;
char *commands[110][110];
int commands_nr;

int read_line(char *input) {
    char *buffer = readline("\n[>] ");
    if (strlen(buffer) > 0) {
        add_history(buffer);
        strcpy(input, buffer);
        free(buffer);
        return 0;
    } else {
        free(buffer);
        return 1;
    }
}


int wc_command(char *argv[]) {

    int nrOfFiles = 1;
    bool c = 0, w = 0, l = 0, L = 0;
    FILE *file;
    char buffer[4096];
    int total_words = 0;
    int total_lines = 0;
    int total_bytes = 0;
    int total_max = 0;
    int max1 = 0;
    int j = 1;
    char completed[50];
    memset(completed, 0, 50);

    while (argv[j] != NULL) {

        if (argv[j + 1] == NULL && strstr("-c-w-l-L", argv[j]) != NULL && !strcmp(argv[j - 1], "wc")) {
            fprintf(stderr,"Error: At least one file is required.");
            return -1;
        }

        if (!strstr("-c-w-l-L", argv[j]) && argv[j][0] == '-') {
            fprintf(stderr,"Error: Invalid argument. Use -c -w -l -L");
            return -1;
        }

        if (strcmp(argv[j], "-c") == 0) {
            c = 1;
            j++;
            continue;
        }

        if (strcmp(argv[j], "-w") == 0) {
            w = 1;
            j++;
            continue;
        }

        if (strcmp(argv[j], "-l") == 0) {
            l = 1;
            j++;
            continue;
        }

        if (strcmp(argv[j], "-L") == 0) {
            L = 1;
            j++;
            continue;
        }

        file = fopen(argv[j], "r");
        if (!file) {
            fprintf(stderr,"Error opening file");
            return -1;
        }
        int bytes = 0;
        int word_count = 0;
        int line_count = 0;
        int line_length = 0;
        int max = 0;

        while (!feof(file)) {
            bytes = fread(buffer, 1, (sizeof buffer) - 1, file);
            for (int i = 0; i < bytes; i++) {
                line_length++;
                if ((isspace(buffer[i]) && !isspace(buffer[i + 1])) ||
                    (!isspace(buffer[i]) && (i + 1) == bytes)) {
                    word_count++;
                }
                if (buffer[i] == '\n') {
                    line_count++;
                    if (line_length > max)
                        max = line_length - 1;
                    line_length = 0;
                }
            }
        }


        fclose(file);


        if (l) {
            sprintf(completed + strlen(completed), "%2d ", line_count);
        }

        if (w) {
            sprintf(completed + strlen(completed), "%2d ", word_count);
        }

        if (c) {
            sprintf(completed + strlen(completed), "%2d ", bytes);
        }

        if (L) {
            sprintf(completed + strlen(completed), "%2d ", max);
        }

        if (!c && !w && !l && !L) {
            sprintf(completed + strlen(completed), "%2d %2d %2d ", line_count, word_count, bytes);
        }

        sprintf(completed + strlen(completed), "%s\n", argv[j]);

        total_words += word_count;
        total_lines += line_count;
        total_bytes += bytes;


        if (nrOfFiles == 1) {
            max1 = max;
        }

        if (argv[j + 1] != NULL && nrOfFiles == 1) {
            nrOfFiles += 1;
            j++;
            continue;
        }

        if (max < max1) {
            total_max = max1;
        } else {
            total_max = max;
        }

        if (nrOfFiles == 2) {

            if (l) {
                sprintf(completed + strlen(completed), "%2d ", total_lines);
            }

            if (w) {
                sprintf(completed + strlen(completed), "%2d ", total_words);
            }

            if (c) {
                sprintf(completed + strlen(completed), "%2d ", total_bytes);
            }

            if (L) {
                sprintf(completed + strlen(completed), "%2d ", total_max);
            }

            if (!c && !w && !l && !L) {
                sprintf(completed + strlen(completed), "%2d %2d %2d ", total_lines, total_words, total_bytes);
            }
            sprintf(completed + strlen(completed), "total");

        }
        j++;
        printf("%s", completed);
    }
    return 1;
}

int expand_command(char *argv[]) {

    FILE *file;
    int t_number = 0;
    bool t = 0, i = 0;
    int j = 1;
    int cbc;
    char buffer[4096];

    char completed[500];
    memset(completed, 0, 500);

    while (argv[j] != NULL) {

        if ((argv[j + 1] == NULL && strstr("-t-i", argv[j]) != NULL) ||
            (!strcmp(argv[j], "expand") && argv[j + 1] == NULL)) {
            fprintf(stderr,"Error: A file is required.\n");
            return -1;
        }

        if (strstr("-t-i", argv[j]) == 0 && argv[j][0] == '-') {
            fprintf(stderr,"Error: Unknown argument. Use one of the following arguments: -t -i");
            return -1;
        }

        if (strcmp(argv[j], "-t") == 0) {
            t = 1;
            t_number = atoi(argv[j + 1]);
            if (t_number == 0) {
                fprintf(stderr,"Error: Value cannot be 0");
                return -1;
            }
            j += 2;
            continue;
        }

        if (strcmp(argv[j], "-i") == 0) {
            i = 1;
            j++;
            continue;
        }

        file = fopen(argv[j], "r");
        if (!file) {
            fprintf(stderr,"Error opening file");
            return -1;
        }

        while (!feof(file)) {
            cbc = fread(buffer, 1, (sizeof buffer) - 1, file);
            int used = 0;
            int sw = 0;
            for (int m = 0; m < cbc; m++) {
                if (buffer[m] == '\t' && sw == 0) {
                    if (t_number == 0) {
                        t_number = 8;
                    }
                    for (int k = 1; k <= (t_number - used); k++) {
                        sprintf(completed + strlen(completed), "%s", " ");
                    }
                    used = 0;
                } else {
                    sprintf(completed + strlen(completed), "%c", buffer[m]);
                    if (used == 8 || buffer[m] == '\n') {
                        used = 0;
                    } else {
                        used++;
                    }
                    if (i) {
                        if (buffer[m] != ' ') {
                            if (buffer[m] != '\n') {
                                sw = 1;
                            } else {
                                sw = 0;
                            }
                        }
                    }
                }
            }
        }

        fclose(file);

        j++;
        printf("%s", completed);

    }

    return 1;
}


int env_command(char *argv[]) {
    int len;
    if (argv[2])
        len = strlen(argv[2]);

    if (argv[1] != NULL) {
        if (!strcmp(argv[1], "-u")) {
            if (argv[2] == NULL) {
                fprintf(stderr,"Error: Parameter expected after -u.");
                return -1;
            }
        }
    }


    if (argv[1] != NULL && strcmp(argv[1], "-u") != 0) {
        fprintf(stderr, "Error: Use one of the following arguments: -u");
        return -1;
    }

    for (int i = 0; environ[i] != NULL; i++) {
        char *current_env_var = environ[i];
        if (current_env_var[0] == '\0') {
            continue;
        }
        if (argv[2] != NULL && strncmp(current_env_var, argv[2], strlen(argv[2])) == 0) {
            continue;
        }
        printf("%s\n", current_env_var);
    }
    return 1;

}

void help_command() {
    printf("\nThe following commands are implemented:\n\n"
           "Command       Optional parameters\n\n"
           "wc            -c -w -l -L\n"
           "expand        -t -i\n"
           "env           -u\n\n");
    printf("Additionally, you can use any command available in Linux.\n"
           "Type \'version\' for more information.\n");
}

void version_command() {
    printf("Version 1.3.1\n"
           "Author: Denis Filea\n"
           "E-mail: denis.filea02@e-uvt.ro\n"
           "West University of Timisoara\n");
}

int o_commands(int c) {
    int args_sw = 0;
    char *command_names[6] = {"exit", "wc", "expand", "env", "help", "version"};

    for (int i = 0; i < 6; i++) {
        if (!strcmp(commands[c][0], command_names[i])) {
            args_sw = i + 1;
            break;
        }
    }

    switch (args_sw) {
        case 1:
            printf("\nExiting...\n");
            _exit(0);
        case 2:
            return wc_command(commands[c]);
        case 3:
            return expand_command(commands[c]);
        case 4:
            return env_command(commands[c]);
        case 5:
            help_command();
            return 1;
        case 6:
            version_command();
            return 1;
        default:
            break;
    }

    return 0;
}

void redirection() {
    int originalout = dup(1);
    int currentout;

    pid_t pid;

    if (outFile) {
        currentout = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (currentout == -1) {
            fprintf(stderr,"Error opening file.");
        }

    } else {
        currentout = dup(originalout);
    }
    dup2(currentout, STDOUT_FILENO);
    int response = o_commands(0);
    if (!response) {
        pid = fork();
        if(pid == 0) {
            execvp(commands[0][0], commands[0]);
            _exit(1);
        }
        else
            waitpid(pid, NULL, 0);
    }
    close(currentout);

    dup2(originalout, STDOUT_FILENO);
    close(originalout);
}


void space_parser(char *str, int c) {
    int i = 0;
    char *word = strtok(str, " ");
    while (word != NULL && i < 15) {
        if (strlen(word) > 0) {
            commands[c][i] = word;
            i++;
        }
        word = strtok(NULL, " ");
    }

    while (i < 15) {
        commands[c][i] = NULL;
        i++;
    }
}


void string_parser(char *input) {
    char *cmds[15];
    int num_cmds = 0;
    char *out_file = NULL;

    char *cmd = strtok(input, "|");
    while (cmd != NULL && num_cmds < 15) {
        cmds[num_cmds] = cmd;
        num_cmds++;
        cmd = strtok(NULL, "|");
    }

    if (num_cmds == 0) {
        cmds[num_cmds] = input;
        num_cmds++;
    }


    cmd = cmds[num_cmds - 1];
    char *out_pos = strchr(cmd, '>');
    if (out_pos != NULL) {
        *out_pos = '\0';

        char *str = out_pos + 1;
        char *end;

        while (isspace((unsigned char) *str)) {
            str++;
        }

        end = str + strlen(str) - 1;
        while (end > str && isspace((unsigned char) *end)) {
            end--;
        }
        *(end + 1) = '\0';
        out_file = str;

    }
    
    for (int i = 0; i < num_cmds; i++) {
        char *str = cmds[i];
        char *end;

        while (isspace((unsigned char) *str)) {
            str++;
        }

        end = str + strlen(str) - 1;
        while (end > str && isspace((unsigned char) *end)) {
            end--;
        }
        *(end + 1) = '\0';


        space_parser(str, i);
    }

    commands_nr = num_cmds;
    outFile = out_file;
}


int main() {
    char input[500];
    while (1) {
        outFile = NULL;
        if (read_line(input))
            continue;
        string_parser(input);
        redirection();

    }
}