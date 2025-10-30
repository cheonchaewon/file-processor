#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#define FIFO_CLNT_TO_SVR "fifo_clnt_to_svr"
#define FIFO_SVR_TO_CLNT "fifo_svr_to_clnt"
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <input_file> <mode>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    char *mode = argv[2];

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("파일 열기 실패");
        exit(1);
    }

    int fd_out, fd_in;
    char buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    int line_num = 0;
    struct timeval start, end;


    mkfifo(FIFO_CLNT_TO_SVR, 0666);
    mkfifo(FIFO_SVR_TO_CLNT, 0666);

    fd_out = open(FIFO_CLNT_TO_SVR, O_WRONLY);
    fd_in = open(FIFO_SVR_TO_CLNT, O_RDONLY);

    if (fd_out < 0 || fd_in < 0) {
        perror("FIFO open error");
        exit(1);
    }


    gettimeofday(&start, NULL);


    write(fd_out, mode, strlen(mode) + 1);

    while (fgets(buf, BUF_SIZE, fp)) {
        buf[strcspn(buf, "\n")] = '\0'; // 개행 제거
        line_num++;
        printf("%d번째 줄 전송...\n", line_num);
        write(fd_out, buf, strlen(buf) + 1);

        memset(recv_buf, 0, BUF_SIZE);
        read(fd_in, recv_buf, BUF_SIZE);
        printf("%d번째 줄 결과 수신: %s\n", line_num, recv_buf);
    }


    write(fd_out, "END", 4);

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    printf("\n=== 처리 통계 ===\n");
    printf("처리 모드: %s\n", mode);
    printf("처리한 줄 수: %d줄\n", line_num);
    printf("소요 시간: %.3f초\n", elapsed);

    close(fd_out);
    close(fd_in);
    fclose(fp);
    return 0;
}

