#ifndef __DZ_COMMUNICATOR_H__
#define __DZ_COMMUNICATOR_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef __linux__
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h> // 引入信号量头文件
#include <stdbool.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <errno.h>
#include <time.h> 
#include <unistd.h> // 包含 usleep() 函数所在头文件
#define SOCK_FILE	"/tmp/sock"//设置临时文件路径
typedef int socket_num;
#endif
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_num;
#endif

#define isserver true
#define isclient false

#ifdef __linux__
////////////////////////////////////////
//          数据结构，后期修改
////////////////////////////////////////
// 共享内存数据类型
#define Mem_t double
// 共享内存长度
#define share_mem_size 250

////////////////////////////////////////
//          此处配置共享内存
////////////////////////////////////////
#define H2L_EN 0    // 高层对低层信号量使能，默认使能，使能后低层阻塞接收，使用前配置
#define L2H_EN 1    // 低层对高层信号量使能,默认不使能，使能后高层阻塞接收，使用前配置
#define H2L_flag_block 1    // 高层对低层flag读阻塞，默认阻塞，使用前配置
#define L2H_flag_block 1    // 低层对高层flag读阻塞，默认阻塞，使用前配置
#define select_msec_or_sec 0 // 选择阻塞时间单位，0为秒，1为毫秒
#define block_tim 60 // 阻塞时间,注意，毫秒等待不能超过1000，若超过1000，会自动转换为秒，使用前配置
#define flag_block_tim 6000 // 高层对低层flag读阻塞时间,单位为秒，使用前配置
#define flag_tim_block_unit 10    //flag时间戳阻塞时间单位(ms)(默认就行)



////////////////////////////////////////
//          此处无需配置
////////////////////////////////////////
#define WR_suc 1    // 写操作成功标识符，不用动
#define WR_fail 0   // 写操作失败标识符，不用动

#define WR_COMP 1   // 写操作完成标识符，不用动
#define RD_COMP 0   // 读操作完成标识符，不用动
#define share_mem_write true//，不用动
#define share_mem_read false//，不用动

#define high_level true    // 高层，不用动
#define low_level false     // 低层，不用动
#define H_clean 1   // 高层清空标识符，不用动
#define L_clean 0   // 低层清空标识符，不用动

// 如果使能，上层读取数据时若低层没更新数据，则会阻塞，直到低层更新数据
#endif



///////////////////////////////////////////////////////////////
//                      套接字通信模块
///////////////////////////////////////////////////////////////
namespace dzsocket{
	class socketgo{
	public:
		socketgo(void);
		~socketgo(void);
		//网络客户描述符
		socket_num sockClient;
		//网络服务端描述符
		socket_num sockServer;
	private:
		void skerror(socket_num sockConn);
		void skack(socket_num sockConn);
		int waitack(socket_num sockConn);
        int socketConnectNetClient (int PORT,const char *IP);
		int socketConnectNetServer (int PORT);
        //监听描述符(服务端临时创建用于监听的)
		socket_num server_sockfd;
        bool isServer;
	public:
		// 打开socket连接
		// params :	PORT	传输端口
		// return : -1		连接失败
		//			0		连接成功
        bool getIsServer();
		int socketConnectNet(bool isServer,const char* IP, int PORT);
		// return : -1		接收失败
		//			0		接收成功
		int transmit(char* Data,int data_size, socket_num sockConn);
		int receive(char* Data,int data_size ,socket_num sockConn);
		// 断开socket连接
		void socketDisconnect(socket_num sockConn);
	};
}

#ifdef __linux__
/////////////////////////////////
//  共享内存结构体大小不大于4K
/////////////////////////////////
namespace Share_mem{

    typedef struct {
        bool L2H_flag;              // 低层写高层标识符
        bool H2L_flag;              // 高层写低层标识符
        // 互斥锁
        pthread_mutex_t shared_mutex;
        // 数据缓存
        Mem_t H2L_cache[share_mem_size];
        Mem_t L2H_cache[share_mem_size];

    }share_zone,*SZ;


    typedef struct {
        // 共享身份
        bool HL_ID;
        // 共享内存ID
        int ID;
        // 共享内存大小
        int H2L_data_size;
        int L2H_data_size;
        // 共享内存指针
        SZ share_mem_ptr;
        // 读写操作完成标识符
        bool wr_operate;
        //  读写时间戳
        struct timespec ts;
        //  flag时间戳
        struct timespec flag_ts;
        //  共享内存信号量，H2L为高层对低层信号量，L2H为低层对高层信号量
    #if H2L_EN
        sem_t *H2L;
    #endif
    #if L2H_EN
        sem_t *L2H;
    #endif
    }smp,*SMP;


    SMP share_mem_init(int H2L_data_size ,int L2H_data_size,bool H_or_L,key_t key);
    int share_mem_destroy(SMP share_mem_param);
    void share_mem_clean(bool H_or_L,SMP share_mem_param);
    void SM_write(Mem_t *cache,SMP share_mem_param);
    int SM_read(Mem_t *cache,SMP share_mem_param);
    void SM_set_flag(SMP share_mem_param);
    bool SM_read_flag(SMP share_mem_param);
}

#endif



/////////////////////////////////
typedef class dz_communicator{
    public:
    dz_communicator(){};
    ~dz_communicator(){};
    void dz_communicator_init(  bool isServer,const char* IP, int PORT);
#ifdef __linux__
    void dz_communicator_init(  bool isServer,const char* IP, int PORT,
                                int H2L_data_size ,int L2H_data_size,bool H_or_L,key_t key);
    void dz_communicator_init(  int H2L_data_size ,int L2H_data_size,bool H_or_L,key_t key);
    // 共享内存函数接口
    void SM_write(Mem_t *cache);
    int SM_read(Mem_t *cache);
    void share_mem_clean(bool H_or_L);
    int share_mem_destroy();
    void SM_set_flag();
    bool SM_read_flag();
#endif
    // 套接字函数接口
    int transmit(char* Data,int data_size);
    int receive(char* Data,int data_size);
    void socketDisconnect();

    private:
#ifdef __linux__
    bool share_mem_enable = false;
    Share_mem::smp *my_share_mem; // Add class qualifier before Share_mem::smp
#endif
    bool socket_enable = false;
    dzsocket::socketgo *mysocket;
}dzc;




#endif
