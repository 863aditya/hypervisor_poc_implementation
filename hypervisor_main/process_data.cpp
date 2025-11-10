#include<set>
class process_data{
    public:
    int num_of_cpu;
    int id;
    int ram_size;
    int persistent_storage;
    std::set<int>pids;
    process_data(int num_of_cpu,int id,int ram_size,int persistent_storage):num_of_cpu(num_of_cpu),id(id),ram_size(ram_size),persistent_storage(persistent_storage){}
    void add_pid(int pid);
    void remove_pid(int pid);
    int get_ram_size();
    int get_persistent_storage();
    int get_num_of_cpu();
    int get_id();
};

void process_data::add_pid(int pid){
    pids.insert(pid);
}
void process_data::remove_pid(int pid){
    if(pids.find(pid)!=pids.end()){
        pids.erase(pid);
    }
}
int process_data::get_ram_size(){
    return ram_size;
}
int process_data::get_persistent_storage(){
    return persistent_storage;
}
int process_data::get_num_of_cpu(){
    return num_of_cpu;
}
int process_data::get_id(){
    return id;
}