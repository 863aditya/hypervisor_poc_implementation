#include<pqxx/pqxx>
#include<iostream>
#include<vector>
#include "db.h"
pqxx::connection C("dbname=hypervisor_db user=hypervisor_user password=hypervisor_pass hostaddr=127.0.0.1 port=5432");

void execute_query(const std::string &&query){
    try{
        pqxx::transaction N(C);
        pqxx::result R = N.exec(query);
        std::cout<<"Query Successfull\n";
        N.commit();
    } catch(const std::exception &e){
        std::cout<<e.what()<<"\n";
    }
}
int check_key_username_pair(std::string &username,std::string &key){
    try{
        pqxx::nontransaction N(C);
        std::string query = "select key_generated from key_username where username = $1";
        pqxx::result R =  N.exec_params(query,username);
        bool found = false;
        for(pqxx::row const &row:R){
            if(row["key_generated"].as<std::string>()==key){
                found = true;
            }
        }
        return found;
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
    return -1;
}

int check_login_username_password(std::string &username,std::string &password){
    try{
        pqxx::nontransaction N(C);
        std::string query = "select password from login_credentials where username = $1";
        pqxx::result R =  N.exec_params(query,username);
        bool found = false;
        for(pqxx::row const &row:R){
            if(row["password"].as<std::string>()==password){
                found = true;
            }
        }
        return found;
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
    return -1;
}

int push_resource_allocation_to_table(std::string username,int cpu,int ram,int storage){
    try
    {
        pqxx::work N(C);
        N.exec_params(
            "insert into resource_allocated values ($1,$2,$3,$4);",
            username,
            cpu,
            ram,
            storage
        );
        N.commit();
        std::cout<<"Records inserted successfully\n";
        return 1;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return -1;
}
int push_key_username_pair(std::string &username,std::string &key){
    try
    {
        pqxx::work N(C);
        N.exec_params(
            "insert into key_username values ($1,$2);",
            username,
            key
        );
        N.commit();
        std::cout<<"Key-Username pair inserted successfully\n";
        return 1;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return -1;
}

std::vector<resource_allocated_rows> get_all_resource_allocations(){
    std::vector<resource_allocated_rows> result_vector;
    try{
        pqxx::nontransaction N(C);
        std::string query = "select * from resource_allocated";
        pqxx::result R =  N.exec(query);
        for(pqxx::row const &row:R){
            std::cout<<"Fetched row - Username: "<<row["username"].as<std::string>()
                     <<", CPU: "<<row["cpu"].as<int>()
                     <<", RAM: "<<row["ram"].as<int>()
                     <<", Storage: "<<row["storage"].as<int>()<<"\n";
            resource_allocated_rows rar;
            rar.username = row["username"].as<std::string>();
            rar.cpu = row["cpu"].as<int>();
            rar.ram = row["ram"].as<int>();
            rar.storage = row["storage"].as<int>();
            result_vector.push_back(rar);
        }
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
    return result_vector;
}