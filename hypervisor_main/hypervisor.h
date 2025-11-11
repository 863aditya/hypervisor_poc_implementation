#pragma once

#include <string>
#include <mutex>
#include <queue>
#include <utility>

class hypervisor{
    public:
    // resource lock for this hypervisor instance
    std::mutex lock_resources;

    // global resource counters (defined in .cpp)
    int available_cpu;
    int available_ram;
    int available_storage;

    // key length used by key_generation
    const int key_length = 25;

    // queue and lock used for cross-thread process/user coordination
    static std::mutex q_lock;
    static std::queue<std::pair<int,std::string>> q;
    std::vector<int> cpu_ids;
    // optional helper (kept from previous header)
    static void add_to_queue(int pid,std::string &username);

    // run the hypervisor main loop
    void run();

    // user related
    int login(std::string &username,std::string &password);
    int request_resources(std::string username,int cpu,int ram,int storage,bool key_gen_req,bool insert_into_db);
    // int get_id(std::string &username);
    std::string key_generation();

    // allocation related
    // int set_cpu_pid(int pid,int cpu_id);

    // // maintaining checks
    // int maintain_check();
    // int check_process_for_storage_breach(int id);
    // int check_process_for_ram_breach(int id);
};