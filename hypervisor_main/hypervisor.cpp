#pragma once
#include "process_data.h"
#include<map>
#include<string>
#include<queue>
#include<mutex>
#include "../db/db.h"
#include "hypervisor.h" 
#include "../instance_main/instance.h"
#include<thread>
#include<unistd.h>
#include<random>
#include<format>
#include<string>
#include <sstream>

int random_mod_m(int m){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, m-1);
    return dis(gen);
}


int hypervisor::login(std::string &username,std::string &password){
    int result = check_login_username_password(username,password);
    if(result == -1){
        std::cout<<"Error occured with db\n";
        exit(-1);
    }
    return result;
}
int hypervisor::request_resources(std::string username,int cpu,int ram,int storage,bool key_gen_req,bool insert_into_db){
    lock_resources.lock();
    bool can_allocate = false;
    if(insert_into_db){ 
        if(cpu <= available_cpu && ram <= available_ram && storage <= available_storage){
            available_cpu -= cpu;
            available_ram -= ram;
            available_storage -= storage;
            can_allocate = true;
            push_resource_allocation_to_table(username,cpu,ram,storage);
        }
    }
    else{
        can_allocate = true;
    }
    if(!can_allocate){
        lock_resources.unlock();
        return false;
    }
    std::vector<int> allocated_cpu_ids;
    for(int i=0;i<cpu;i++){
        allocated_cpu_ids.push_back(cpu_ids.back());
        cpu_ids.pop_back();
    }
    lock_resources.unlock();
    int port_running = 2222 + random_mod_m(1000);
    instance *new_instance = new instance(cpu,username,ram,storage,port_running,allocated_cpu_ids);
    if(key_gen_req){
        std::string key_generated = key_generation();
        push_key_username_pair(username,key_generated);
        std::cout<<"Generated key for user "<<username<<": "<<key_generated<<"\n";
    }
    else{
        std::cout<<"Key generation not requested for user "<<username<<"\n";
    }
    // std::thread t([=](){
    //     sched_setaffinity(0, allocated_cpu_ids.size()*sizeof(int), (cpu_set_t*)allocated_cpu_ids.data());
    //     new_instance->run();
    // });
    // t.detach();
    int t = fork();
    if(t==0){
        std::cout<<"running instance for user :"<<username<<" inside child process\n";
        sched_setaffinity(0, allocated_cpu_ids.size()*sizeof(int), (cpu_set_t*)allocated_cpu_ids.data());
        new_instance->run();
    }
    else{
        int child_pid = t;
        new_instance->add_pid(child_pid);
        int p = fork();
        if(p==0){
            std::stringstream ss;
            ss << "systemd-run --scope -p MemoryMax=" << new_instance->ram_size<< " -p CPUQuota=50--pid=" << child_pid;
            execvp(ss.str().c_str(),NULL);
        }
        else{
            std::cout<<"limited the memory of the instance successfully\n";
        }
    }
    return can_allocate;
}
std::string hypervisor::key_generation(){
    std::string key_generated = "";
    for(int i =0;i<key_length;i++){
        key_generated += random_mod_m(26)+'A';
    }
    return key_generated;
}
void hypervisor::run(){
    std::cout<<"Hypervisor is running and managing resources.\n";
    std::vector<resource_allocated_rows> current_allocations = get_all_resource_allocations();
    for(auto &row: current_allocations){
        available_cpu -= row.cpu;
        available_ram -= row.ram;
        available_storage -= row.storage;
        request_resources(row.username,row.cpu,row.ram,row.storage,false,false);
    }
    while(true){
        std::cout<<"allowed operations:1. Request Resources\n3. Exit\n";
        int choice;
        std::cin>>choice; 
        if(choice == 1){
            std::string username;
            std::cout<<"Enter username: ";
            std::cin>>username;
            std::cout<<"Enter password: ";
            std::string password;
            std::cin>>password;
            if(!login(username,password)){
                std::cout<<"Login failed for user "<<username<<"\n";
                continue;
            }
            int cpu,ram,storage;
            std::cout<<"Enter CPU, RAM and Storage request: ";
            std::cin>>cpu>>ram>>storage;
            if(request_resources(username,cpu,ram,storage,true,true)){
                std::cout<<"Resource allocation successful\n";
                std::cout<<"new instance is up and running for user "<<username<<"\n";
            }
            else{
                std::cout<<"Resource allocation failed\n";
            }
        }
        else if(choice == 3){
            std::cout<<"Exiting...\n";
            break;
        }
    }
}