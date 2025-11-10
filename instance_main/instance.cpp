#include <iostream>
#include <set>
#include <vector>
#include<thread>
#include "../mini_ssh/mini_ssh.h"
#include "instance.h"


void instance::add_pid(int pid){
    pids.insert(pid);
}
void instance::remove_pid(int pid){
    pids.erase(pid);
}

 void instance::run(){
    std::cout<<"Instance for user "<<username<<" is running with "<<num_of_cpu<<" CPUs, "<<ram_size<<" MB RAM and "<<persistent_storage<<" MB storage.\n";
    std::thread t([&](){
        sched_setaffinity(0, cpu_ids.size()*sizeof(int), (cpu_set_t*)cpu_ids.data());
        int argc = 3;
        char* argv[3];
        argv[0] = (char*)"mini_ssh";
        argv[1] = (char*)username.c_str();
        argv[2] = (char*)"dummy_key";
        server_run(argc,argv,default_mini_ssh_port);
    });
    t.detach();
}
