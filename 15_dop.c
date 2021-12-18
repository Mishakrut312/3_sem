#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/signalfd.h>

void proc_info(void) {
    printf("PID %d, PPID %d, PGID %d, SID %d, UID %d, GID %d\n", getpid(), getppid(), getpgrp(), getsid(0), getuid(), getgid());
}

int main(void) {
    
    proc_info();
    
    sigset_t mask;
    // Хотим блокировать ВСЕ сигналы
    sigfillset(&mask);

    // Блокируем сигналы
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
       perror("sigprocmask");
       return 1;
    }

    int sfd = signalfd(-1, &mask, 0);
    if (sfd == -1) {
       perror("signalfd");
       return 1;
    }
    
    struct signalfd_siginfo fdsi;
    ssize_t size_of_siginfo = sizeof(struct signalfd_siginfo);
    
    //выгребаем информацию про сигналы
    while(1) {
        ssize_t size_read = read(sfd, &fdsi, (size_t)size_of_siginfo);
        if (size_read != size_of_siginfo) {
            //закончили выгребать
            break;
        }
        printf("%s\n", strsignal((int)fdsi.ssi_signo));
    }
    
    if(close(sfd) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}

