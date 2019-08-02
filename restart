#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int child_pid;
    int ret;
    int file_exist;
    char* elf_path = argv[1];
    char* elf_proc_name = argv[2];
    char* elf_param = argv[3];

    if(argc != 4) {
        printf("参数不是4个，请使用单引号处理shell参数;\n");
        printf("使用方法，注意shell参数引号，使用绝对路径：本程序名 待加载ELF文件绝对路径 进程名 参数(可加空格)'\n");
        printf("./restart '/bin/xxxx' 'xxxx' '/xx/xx/param2 param1'\n");
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
            printf("创建子进程失败\n");
            sleep(1);
        } else if(pid != 0) {
            printf("父进程等待子进程结束\n");
            child_pid = wait(NULL);
            if(ret == -1) {
                printf("获取子进程进程号失败\n");
            } else {
                printf("退出的子进程进程号是:%d;\n", child_pid);
            }
        } else {
            //child
            printf("子进程开始执行任务\n");
            break;
        }

        printf("睡一秒，等会儿接着干活\n\n");
        sleep(1);
    } while (1);

    //加载其他ELF，余下代码不被执行
    execl(elf_path, elf_proc_name, elf_param, NULL);

    return 0;
}
