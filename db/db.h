#pragma once
#include<iostream>
#include<vector>
struct resource_allocated_rows
{
    std::string username;
    int cpu;
    int ram;
    int storage;
};

void execute_query(const std::string &&query);
int check_key_username_pair(std::string &username,std::string &key);
int check_login_username_password(std::string &username,std::string &password);
int push_resource_allocation_to_table(std::string username,int cpu,int ram,int storage);
int push_key_username_pair(std::string &username,std::string &key);
std::vector<resource_allocated_rows> get_all_resource_allocations();