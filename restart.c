#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

static pid_t kill_child_pid = 0;
static char log_buffer[1024] = {0};

void print_log(char* log) {
    time_t rawtime;
    struct tm *info;
    char buffer[80] = {0};

    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);

    fprintf(stdout, "[%s]::%s\n", buffer, log);
    fflush(stdout);
}

void kill_child(int signo) {

    int kill_res = 0;

    sprintf(log_buffer, "捕获到信号:%d;", signo);
    print_log(log_buffer);
    memset(log_buffer, 0, sizeof log_buffer);

    if (kill_child_pid != 0) {
        sprintf(log_buffer, "关闭子进程:%d;", kill_child_pid);
        print_log(log_buffer);
        memset(log_buffer, 0, sizeof log_buffer);
        kill_res = kill(kill_child_pid, SIGQUIT);

        if ( kill_res != 0 ) {
            print_log("成功关闭子进程\n");
        } else {
            print_log("未能关闭子进程\n");

            sprintf(log_buffer, "失败原因%d\n", errno);
            print_log(log_buffer);
            memset(log_buffer, 0, sizeof log_buffer);
        }
    } else {
        print_log("未找到子进程\n");
    }

    exit(0);
}

int main(int argc, char *argv[]) {

    char* self_name = argv[0];

    if(argc < 5) {
        printf("参数太少！！！！！！！！！！！\n");
        printf("encoding:UTF-8;\n");
        printf("使用方法：%s -n xxxx -e /bin/xxxx -f xxx.log -p \"param1 param2\" -f \"\"\n", self_name);
        printf("-n 进程名\n");
        printf("-e 待加载ELF文件绝对路径\n");
        printf("-p 子进程参数(多参数空格，并加\"\"双引号)，可留空\n");
        printf("-f 子进程输出及错误重定向，可留空\n");
        return 1;
    }

    pid_t pid;
    pid_t child_pid;

    char* param[16] = {0};

    char* arg_e = "";
    char* arg_n = "";
    char* arg_p = "";
    char* arg_f = "";

    int opt;

    while ((opt = getopt(argc, argv, "e:n:p:f:")) != -1) {
        switch (opt) {
            case 'e':
                arg_e = optarg;
                break;
            case 'n':
                arg_n = optarg;
                break;
            case 'p':
                arg_p = optarg;
                break;
            case 'f':
                arg_f = optarg;
                break;
            case '?':
                printf("encoding:UTF-8;\n");
                printf("使用方法：%s -n xxxx -e /bin/xxxx -f xxx.log -p \"param1 param2\" -f \"\"\n", self_name);
                printf("-n 进程名\n");
                printf("-e 待加载ELF文件绝对路径\n");
                printf("-p 子进程参数(多参数空格，并加\"\"双引号)，可留空\n");
                printf("-f 子进程输出及错误重定向，可留空\n");
                break;
        }
    }

    int file_test = 0;
    //可执行文件权限判定
    file_test = access(arg_e, X_OK);
    if(file_test != 0) {
        printf("待执行文件不存在，请检查文件\n");
        return 2;
    }
    //日志文件权限判定
    if(strlen(arg_f) != 0) {
        file_test = access(arg_f, F_OK);
        if (file_test != 0) {
            FILE* file_fd = fopen(arg_f, "w+");
            if (file_fd == NULL) {
                printf("输出文件无法写入，请检查创建权限\n");
                return 2;
            }
            fclose(file_fd);
        }else{
            file_test = access(arg_f, W_OK);
            if (file_test != 0) {
                printf("输出文件无法写入，请检查写入权限\n");
                return 2;
            }
        }
    }
    //参数解析
    if(strlen(arg_p) != 0) {
        char* p_cut;
        char* delim = " ";
        int i = 0;

        //设置子进程进程名
        param[i] = arg_n;
        i++;

        p_cut = strtok(arg_p, delim);
        while (p_cut != NULL) {
            param[i] = p_cut;
            p_cut = strtok(NULL, delim);
            i++;
        }
        param[i] = NULL;
    }

    do {
        pid = fork();

        if(pid < 0) {
            print_log("创建子进程失败");
        } else if(pid != 0) {
            print_log("父进程等待子进程结束");

            kill_child_pid = pid;
            signal(SIGHUP,  SIG_IGN);
            signal(SIGINT,  kill_child);
            signal(SIGQUIT, kill_child);
            signal(SIGTERM, kill_child);

            child_pid = wait(NULL);

            if(child_pid == -1) {
                print_log("获取子进程进程号失败");
            } else {
                sprintf(log_buffer, "退出的子进程进程号是:%d;", child_pid);
                print_log(log_buffer);
                memset(log_buffer, 0, sizeof log_buffer);
            }
        } else {
            //child
            print_log("开始载入子进程ELF至内存，并重置输出和错误输出");
            if(strlen(arg_f) != 0) {
                freopen(arg_f, "a+", stdout);
                freopen(arg_f, "a+", stderr);
            }
            print_log("开始执行子进程");
            //加载其他ELF，余下代码不被执行
            execv(arg_e, param);
        }

        print_log("睡一秒，等会儿接着干活\n");
        sleep(1);

    } while (1);

    return 0;
}
