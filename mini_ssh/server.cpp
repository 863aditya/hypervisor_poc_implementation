#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "../db/db.h"
#include <pty.h>
#include <utmp.h>
#include <termios.h>
#include <sys/ioctl.h>

// Simple mini_ssh server that listens on 127.0.0.1:2222 and accepts a single auth packet.
// On successful auth, replies 0x01 and spawns a /bin/sh with stdin/out/err attached to the socket.

// static const int SERVER_PORT = 2222;

static ssize_t read_n(int fd, void* buf, size_t n){
    size_t left = n;
    char* p = (char*)buf;
    while(left>0){
        ssize_t r = read(fd,p,left);
        if(r<=0) return r;
        left -= r;
        p += r;
    }
    return n;
}

static ssize_t write_n(int fd,const void* buf,size_t n){
    size_t left=n;
    const char* p=(const char*)buf;
    while(left>0){
        ssize_t w=write(fd,p,left);
        if(w<=0) return w;
        left-=w; p+=w;
    }
    return n;
}

int handle_client(int cfd){
    uint8_t version;
    if(read_n(cfd,&version,1)!=1) return -1;
    if(version!=0x01){
        std::cerr<<"Unsupported version: "<<int(version)<<"\n";
        return -1;
    }
    uint8_t auth_type;
    if(read_n(cfd,&auth_type,1)!=1) return -1;
    if(auth_type!=0x01){
        std::cerr<<"Unsupported auth type: "<<int(auth_type)<<"\n";
        return -1;
    }
    uint16_t ulen_n, klen_n;
    if(read_n(cfd,&ulen_n,2)!=2) return -1;
    if(read_n(cfd,&klen_n,2)!=2) return -1;
    uint16_t ulen = ntohs(ulen_n);
    uint16_t klen = ntohs(klen_n);
    if(ulen==0||ulen>65535||klen>65535){
        std::cerr<<"Invalid lengths\n";
        return -1;
    }
    std::string username(ulen, '\0');
    std::string key(klen, '\0');
    if(read_n(cfd,&username[0],ulen)!=(ssize_t)ulen) return -1;
    if(read_n(cfd,&key[0],klen)!=(ssize_t)klen) return -1;

    // validate using existing DB function
    int ok = check_key_username_pair(username,key);
    uint8_t reply = (ok==1?0x01:0x00);
    if(write(cfd,&reply,1)!=1) return -1;
    if(reply==0x00){
        std::cerr<<"Auth failed for "<<username<<"\n";
        return 0;
    }

    // Auth succeeded; spawn a shell connected to a pseudo-terminal (so the shell behaves
    // like a real terminal: line editing, prompts, job control, etc.). We'll use forkpty
    // to allocate a pty and then relay data between the socket and the pty master.
    int master_fd;
    pid_t shell_pid = forkpty(&master_fd, NULL, NULL, NULL);
    if(shell_pid < 0){
        perror("forkpty");
        return -1;
    }
    if(shell_pid == 0){
        // child: exec the shell
        execlp("/bin/sh","sh","-i",(char*)NULL);
        perror("execlp");
        _exit(127);
    }

    // parent: relay data between client socket (cfd) and pty master (master_fd)
    char buf[4096];
    while(1){
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(cfd,&rfds);
        FD_SET(master_fd,&rfds);
        int maxfd = cfd>master_fd?cfd:master_fd;
        int rv = select(maxfd+1,&rfds,NULL,NULL,NULL);
        if(rv<0){
            if(errno==EINTR) continue;
            break;
        }
        if(FD_ISSET(cfd,&rfds)){
            ssize_t r = read(cfd,buf,sizeof(buf));
            if(r<=0) break;
            if(write_n(master_fd,buf,r)<=0) break;
        }
        if(FD_ISSET(master_fd,&rfds)){
            ssize_t r = read(master_fd,buf,sizeof(buf));
            if(r<=0) break;
            if(write(cfd,buf,r)<=0) break;
        }
    }

    // Wait for the shell child to finish
    int status = 0;
    waitpid(shell_pid,&status,0);
    close(master_fd);
    return 0;
}

int server_run(int argc,char** argv,int SERVER_PORT = 2222){
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd<0){ perror("socket"); return 1; }
    int opt = 1;
    setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if(bind(lfd,(struct sockaddr*)&addr,sizeof(addr))<0){ perror("bind"); close(lfd); return 1; }
    if(listen(lfd,10)<0){ perror("listen"); close(lfd); return 1; }

    std::cout<<"mini_ssh server listening on 127.0.0.1:"<<SERVER_PORT<<"\n";
    while(1){
        struct sockaddr_in peer{};
        socklen_t plen = sizeof(peer);
        int cfd = accept(lfd,(struct sockaddr*)&peer,&plen);
        if(cfd<0){ perror("accept"); continue; }
        std::cout<<"connection accepted\n";
        pid_t pid = fork();
        if(pid<0){ perror("fork"); close(cfd); continue; }
        if(pid==0){
            // in child: handle client then exit
            close(lfd);
            int r = handle_client(cfd);
            close(cfd);
            _exit(r==0?0:1);
        }
        else{
            close(cfd);
            // reap children in a non-blocking way
            while(waitpid(-1,NULL,WNOHANG)>0){}
        }
    }
    close(lfd);
    return 0;
}
