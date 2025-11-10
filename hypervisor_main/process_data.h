class process_data{
    public:
    void add_pid(int pid);
    void remove_pid(int pid);
    int get_ram_size();
    int get_persistent_storage();
    int get_num_of_cpu();
    int get_id();
};