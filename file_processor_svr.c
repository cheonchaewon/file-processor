#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_CLNT_TO_SVR "fifo_clnt_to_svr"
#define FIFO_SVR_TO_CLNT "fifo_svr_to_clnt"
#define BUF_SIZE 1024


int count_words(const char *line) {
    int count = 0;
    int in_word = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if (isspace(line[i])) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            count++;
        }
    }
    return count;
}


void str_reverse(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}

int main() {
    int fd_in, fd_out;
    char buf[BUF_SIZE];
    char mode[20];
    int line_num = 0;


    mkfifo(FIFO_CLNT_TO_SVR, 0666);
    mkfifo(FIFO_SVR_TO_CLNT, 0666);

    printf("서버 시작 대기 중...\n");

    fd_in = open(FIFO_CLNT_TO_SVR, O_RDONLY);
    fd_out = open(FIFO_SVR_TO_CLNT, O_WRONLY);

    if (fd_in < 0 || fd_out < 0) {
        perror("FIFO open error");
        exit(1);
    }


    read(fd_in, mode, sizeof(mode));
    printf("서버 모드 설정: %s\n", mode);

    while (1) {
        memset(buf, 0, BUF_SIZE);
        if (read(fd_in, buf, BUF_SIZE) <= 0)
            break;

        if (strcmp(buf, "END") == 0) {
            printf("서버 종료 신호 수신\n");
            break;
        }

        line_num++;
        printf("%d번째 줄 처리 중...\n", line_num);

        
        char result[BUF_SIZE];
        memset(result, 0, BUF_SIZE);

        if (strcmp(mode, "count") == 0) {
            int chars = strlen(buf);
            int words = count_words(buf);
            snprintf(result, BUF_SIZE, "Line %d: %d chars, %d words", line_num, chars, words);
        } else if (strcmp(mode, "upper") == 0) {
            for (int i = 0; buf[i]; i++)
                result[i] = toupper(buf[i]);
        } else if (strcmp(mode, "lower") == 0) {
            for (int i = 0; buf[i]; i++)
                result[i] = tolower(buf[i]);
        } else if (strcmp(mode, "reverse") == 0) {
            strcpy(result, buf);
            str_reverse(result);
        } else {
            snprintf(result, BUF_SIZE, "Invalid mode: %s", mode);
        }

        write(fd_out, result, strlen(result) + 1);
    }

    close(fd_in);
    close(fd_out);
    unlink(FIFO_CLNT_TO_SVR);
    unlink(FIFO_SVR_TO_CLNT);

    printf("서버 종료\n");
    return 0;
}

