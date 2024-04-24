#include "dz_communicator.h"
#include <iostream>


key_t key = 1234;
union d2c{
    /* data */
    double d[100];
    char c[100*sizeof(double)];
};


int main(int argc, char* argv[]) 
{
    d2c data;
    dz_communicator dz;
    dz.dz_communicator_init(isserver,"127.0.0.1",1998,
                            100,100,low_level,key);
    // 共享内存握手
    dz.SM_set_flag();
    dz.SM_read_flag();
    dz.share_mem_clean(low_level);
    std::cout<<"receive data"<<std::endl;
    dz.receive(data.c,sizeof(data.c));
    std::cout<<"write data"<<std::endl;
    dz.SM_write(data.d);
    for(int i=0;i<100;i++){
        std::cout<<data.d[i]<<std::endl;
    }
    dz.socketDisconnect();
    return 0;
}