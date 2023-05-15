#include <iostream>
#include <fstream>
using namespace std;

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>

int main() {
    int cnt = 0;
    ofstream out("out.txt");
    for(int i = 0; i < 10000; i++) {
        int pid = fork();
        if(pid == 0) {
            execl("/home/admin/Code/MyThreadPool/main", "main", NULL);
        } else if(pid < 0) {
            cout << "pid < 0" << endl;
            return 0;
        } else {
            int status;
            usleep(10000);
            pid_t ret = waitpid(pid, &status, WNOHANG);
            if (ret == -1) {
                perror("waitpid");
                exit(1);
            } else if (ret == 0) {
                cnt++;
                printf("timeout\n");
                sleep(1);
                kill(pid, SIGKILL); // 使用SIGKILL信号杀死子进程
                wait(&status);      // 等待子进程结束
            } else {
                // if (WIFEXITED(status)) {
                //     // printf("child process exited with status %d\n", WEXITSTATUS(status));
                // }
            }
        }
    }
    cout << "timeout " << cnt << endl;
    return 0;
}