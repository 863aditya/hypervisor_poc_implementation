#pragma once

#include <string>
#include <set>
#include <vector>

class instance{
public:
    instance() = default;
    ~instance() = default;

    // full constructor matching the implementation in instance.cpp
    instance(int num_of_cpu, std::string &username, int ram_size, int persistent_storage, int default_mini_ssh_port, std::vector<int> cpu_ids = {})
        : num_of_cpu(num_of_cpu), username(username), ram_size(ram_size), persistent_storage(persistent_storage),
          default_mini_ssh_port(default_mini_ssh_port), cpu_ids(cpu_ids) {};

    // basic properties
    int num_of_cpu{};
    std::string username;
    int ram_size{};              // MB
    int persistent_storage{};    // MB
    std::set<int> pids;
    std::vector<int> allocated_ports;
    int default_mini_ssh_port{};
    std::vector<int> cpu_ids;
    // manage pids
    void add_pid(int pid);
    void remove_pid(int pid);

    // run the instance (spawns a mini_ssh server thread)
    void run();
};