#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

void print_log(char* log) {
    time_t rawtime;
    struct tm *info;
    char buffer[80];

    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);

    fprintf(stdout, "[%s]::%s\n", buffer, log);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    pid_t pid;
    pid_t child_pid;
    int ret, file_exist, i, j;
    char* elf_path = argv[1];
    char* param[16];
    char child_process[1024];
    
    //忽略本进程名及待加载ELF路径
    for(i = 2;i < argc;i++) {
        j = i - 2;
        param[j] = argv[i];
    }
    j++;
    param[j] = NULL;

    if(argc <= 2) {
        printf("参数数量错误\n");
        printf("使用方法：本程序名 待加载ELF文件绝对路径 进程名 参数(可加空格)'\n");
        printf("./restart /bin/xxxx xxxx /xx/xx/param1 param2");
        return 1;
    }
    file_exist = access(argv[1], X_OK);
    if(file_exist != 0) {
        printf("待执行文件不存在，请检查文件\n");
        return 2;
    }
    do {
        pid = fork();

        if(pid < 0) {
            print_log("创建子进程失败");
            sleep(1);
        } else if(pid != 0) {
            print_log("父进程等待子进程结束");
            child_pid = wait(NULL);
            if(child_pid == -1) {
                print_log("获取子进程进程号失败");
            } else {
                sprintf(child_process, "退出的子进程进程号是:%d;", child_pid);
                print_log(child_process);
            }
        } else {
            //child
            print_log("子进程开始执行任务");
            //加载其他ELF，余下代码不被执行
            execv(elf_path, param);
        }

        print_log("睡一秒，等会儿接着干活\n");
        sleep(1);
    } while (1);

    return 0;
}
