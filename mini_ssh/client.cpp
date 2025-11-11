#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>


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

// set terminal raw mode
static struct termios orig_term;
static void enable_raw(){
    struct termios t;
    tcgetattr(STDIN_FILENO,&orig_term);
    t = orig_term;
    cfmakeraw(&t);
    tcsetattr(STDIN_FILENO,TCSANOW,&t);
}
static void disable_raw(){
    tcsetattr(STDIN_FILENO,TCSANOW,&orig_term);
}

int interactive_loop(int sock){
    enable_raw();
    atexit(disable_raw);
    char buf[4096];
    while(1){
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO,&rfds);
        FD_SET(sock,&rfds);
        int maxfd = sock>STDIN_FILENO?sock:STDIN_FILENO;
        int rv = select(maxfd+1,&rfds,NULL,NULL,NULL);
        if(rv<0) break;
        if(FD_ISSET(STDIN_FILENO,&rfds)){
            ssize_t r = read(STDIN_FILENO,buf,sizeof(buf));
            if(r<=0) break;
            if(write_n(sock,buf,r)<=0) break;
        }
        if(FD_ISSET(sock,&rfds)){
            ssize_t r = read(sock,buf,sizeof(buf));
            if(r<=0) break;
            if(write_n(STDOUT_FILENO,buf,r)<=0) break;
        }
    }
    return 0;
}

int client_run(std::string username,std::string key,int SERVER_PORT = 2222){

    // std::string key;
    // if(argc>=3){ username = argv[1]; key = argv[2]; }
    // else{
    //     std::cerr<<"Usage: client <username> <key>\n";
    //     return 1;
    // }

    int s = socket(AF_INET,SOCK_STREAM,0);
    if(s<0){ perror("socket"); return 1; }
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);
    if(connect(s,(struct sockaddr*)&addr,sizeof(addr))<0){ perror("connect"); close(s); return 1; }

    // build auth packet
    uint8_t version = 0x01;
    uint8_t auth_type = 0x01;
    uint16_t ulen = htons((uint16_t)username.size());
    uint16_t klen = htons((uint16_t)key.size());
    if(write_n(s,&version,1)!=1) return 1;
    if(write_n(s,&auth_type,1)!=1) return 1;
    if(write_n(s,&ulen,2)!=2) return 1;
    if(write_n(s,&klen,2)!=2) return 1;
    if(write_n(s,username.data(),username.size())!=(ssize_t)username.size()) return 1;
    if(write_n(s,key.data(),key.size())!=(ssize_t)key.size()) return 1;

    uint8_t reply;
    ssize_t rr = read(s,&reply,1);
    if(rr!=1){ std::cerr<<"No reply from server\n"; close(s); return 1; }
    if(reply==0x00){ std::cerr<<"Auth failed\n"; close(s); return 1; }
    if(reply==0x01){ std::cerr<<"Auth success — entering interactive shell\n"; interactive_loop(s); }

    close(s);
    return 0;
}
int main(int argc, char** argv){
    if(argc < 4){
        std::cerr << "Usage: " << (argc>0 ? argv[0] : "client") << " <username> <key> <port>\n";
        return 1;
    }
    std::string username = argv[1];
    std::string key = argv[2];
    int port = 0;
    try{
        port = std::stoi(argv[3]);
    }catch(...){
        std::cerr << "Invalid port: " << argv[3] << "\n";
        return 1;
    }

    return client_run(username, key, port);
}