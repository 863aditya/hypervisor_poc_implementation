#include "db/db.h"
#include<iostream>
#include "mini_ssh/mini_ssh.h"
#include "hypervisor_main/hypervisor.h"
int main(){
    hypervisor hv;
    hv.available_cpu = 4;
    hv.available_ram = 8192; // in MB
    hv.available_storage = 100000; // in MB
    hv.cpu_ids = {0,1,2,3};
    hv.run();
    return 0;
}