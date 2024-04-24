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
    d2c data,data2;
    for(int i=0;i<100;i++){
        data.d[i] = i;
    }
    dz_communicator dz;
    dz.dz_communicator_init(isclient,"127.0.0.1",1998,
                            100,100,high_level,key);
    // 共享内存握手
    dz.SM_read_flag();
    dz.SM_set_flag();
    dz.share_mem_clean(high_level);
    std::cout<<"transmit data"<<std::endl;
    dz.transmit(data.c,sizeof(data.c));
    std::cout<<"read data"<<std::endl;
    dz.SM_read(data2.d);

    for(int i=0;i<100;i++){
        std::cout<<data2.d[i]<<std::endl;
    }
    dz.socketDisconnect();
    dz.share_mem_destroy();
    return 0;
}