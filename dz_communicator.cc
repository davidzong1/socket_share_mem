#include "dz_communicator.h"
#include <iostream>


/**************************************************************************************** 
**************************************************************************************** 
**************************************************************************************** 
                                套接字相关函数
**************************************************************************************** 
**************************************************************************************** 
*****************************************************************************************/

union c2uc {
    char c;
    unsigned char uc;
};

dzsocket::socketgo::socketgo(void) {}
dzsocket::socketgo::~socketgo(void) {}

//socket网络服务器端
int dzsocket::socketgo::socketConnectNetServer(int PORT)
{
#ifdef __linux__
    this->server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(this->server_sockfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1) {
        perror("bind");		return -1;
    }
    if (listen(this->server_sockfd, 5) == -1) {
        perror("listen");		return -1;
    }
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    this->sockServer = accept(this->server_sockfd, (struct sockaddr*)&client_addr, &length);
    if (this->sockServer < 0) {
        perror("connect");
        return -1;
    }
    else {
        return 0;
    }
#endif
#if defined(_WIN32) || defined(_WIN64)
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize winsock.\n");
        return -1;
    }
    this->server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->server_sockfd == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        WSACleanup();
        return -1;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);  // Choose a port number
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(this->server_sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Failed to bind socket.\n");
        //关闭监听套接字
        closesocket(this->server_sockfd);
        WSACleanup();
        return -1;
    }
    if (listen(this->server_sockfd, SOMAXCONN) == SOCKET_ERROR) {
        printf("Failed to listen on socket.\n");
        closesocket(this->server_sockfd);
        WSACleanup();
        return -1;
    }
    //数据传输套接字
    sockServer = accept(this->server_sockfd, NULL, NULL);
    if (sockServer == INVALID_SOCKET) {
        printf("Failed to accept client connection.\n");
        closesocket(sockServer);
        WSACleanup();
        return -1;
    }
    return 0;
#endif	
}
//socket网络客户端
int dzsocket::socketgo::socketConnectNetClient(int PORT, const char* IP)
{
#ifdef __linux__
    struct sockaddr_in    servaddr;
    if ((this->sockClient = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n", IP);
        return -1;
    }

    if (connect(this->sockClient, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        close(this->sockClient);
        return -1;
    }
    else{
        return 0;
    }

#endif
#if defined(_WIN32) || defined(_WIN64)
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize winsock.\n");
        return -1;
    }
    // Create a socket
    sockClient = socket(AF_INET, SOCK_STREAM, 0);
    if (sockClient == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        WSACleanup();
        return -1;
    }
    // Set up the server address
    std::cout << IP << std::endl;
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(serverAddr.sin_family, IP, &serverAddr.sin_addr) <= 0) {
        printf("Invalid IP address.\n");
        closesocket(sockClient);
        WSACleanup();
        return -1;
    }
    // Connect to the server
    if (connect(sockClient, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Failed to connect to server.\n");
        closesocket(sockClient);
        WSACleanup();
        return -1;
    }
    return 0;
#endif
}
bool dzsocket::socketgo::getIsServer()
{
    return isServer;
}
//socket网络连接
/*
    * 函数功能：socket网络连接
    * 函数参数：PORT:端口号，IP:IP地址，isServer:是否为服务器
                本地时使用本机IP地址
    * 函数返回：正常返回0，异常返回-1

*/

int dzsocket::socketgo::socketConnectNet(bool isServer, const char* IP, int PORT)
{
    this->isServer = isServer;
    if (this->isServer == isserver)
        return this->socketConnectNetServer(PORT);
    else
        return this->socketConnectNetClient(PORT, IP);

}
/*************************************
    * 函数功能：关闭socket连接
    * 函数参数：socket_num:socket连接号
    * 函数返回：无
**************************************/
void dzsocket::socketgo::socketDisconnect(socket_num socket_num )
{
#ifdef __linux__
    close(socket_num);
#endif
#if defined(_WIN32) || defined(_WIN64)
    closesocket(socket_num);
    WSACleanup();
#endif
}

/*************************************
    * 函数功能：发送数据
    * 函数参数：Data:发送数据的指针，data_size:发送数据的大小，sockConn:socket连接号
    * 函数返回：正常返回0，异常返回-1
**************************************/
int dzsocket::socketgo::transmit(char* Data, int data_size, socket_num sockConn)
{
    // 单次发送最大数据量
    const int max_size = 1024;
    char* data = (char*)malloc(data_size);
    memcpy(data, Data, data_size);
    // 分包记数
    int sent_byte_cnt = 0;
    while (sent_byte_cnt < data_size) {
        int size_to_send = data_size - sent_byte_cnt;
        if (size_to_send > max_size) {
            size_to_send = max_size;
        }

        int result = send(sockConn, data + sent_byte_cnt, size_to_send, 0);
        if (result < 0) {
            printf("Send Data Failed! : %s\n",strerror(errno));
            free(data);
            return -1;
        }

        sent_byte_cnt += result;
    }

    switch (waitack(sockConn)) {
    case 0:
        free(data);
        return 0;
    case -1:
        free(data);
        printf("Stop!\n");
        return -1;
    default:
        free(data);
        printf("ACK format Error! \n");
        return -1;
    }
}
/*************************************
    * 函数功能：接收数据
    * 函数参数：Data:接收数据的指针，data_size:接收数据的大小，sockConn:socket连接号
    * 函数返回：正常返回0，异常返回-1
**************************************/
int dzsocket::socketgo::receive(char* Data, int data_size, socket_num sockConn)
{
    // 单次接受最大数据量
    const int max_size = 1024;
    char* data = (char*)malloc(data_size);
    if (data == NULL) {
        printf("Receive data size configuration error!\n");
        printf("Memory allocation failed!\n");
        return -1;
    }
    int returnflag = 0; // 用于指示接收是否成功的标志
    memset(data, 0, data_size); // 初始化一个名为data的结构体，假设是用于存储接收数据的全局或类成员变量
    int rec_byte_cnt = 0;
    int rec_byte_cnt_cache = 0;
    while (rec_byte_cnt < data_size) {
        rec_byte_cnt_cache = data_size - rec_byte_cnt;
        if(rec_byte_cnt_cache > max_size){
            rec_byte_cnt_cache = max_size;
        }
        returnflag = recv(sockConn, (char*)(data + rec_byte_cnt), 
                        data_size - rec_byte_cnt, 0); // 接收数据
        if (returnflag <= 0) {
            printf("Recieve Data Failed!\n");
            skerror(sockConn);
            free(data);
            return -1;
        }
        rec_byte_cnt += returnflag;
    }
    skack(sockConn);
    memcpy(Data, data, data_size);
    free(data);
    return 0;
}

void dzsocket::socketgo::skack(socket_num sockConn)
{
    char ack[3] = { 'A','C','K' };
    if (send(sockConn, (char*)(ack), 3 * sizeof(char), 0) < 0) {
        printf("Server Send Ack Failed!\n");
    }
}

void dzsocket::socketgo::skerror(socket_num sockConn)
{
    char ack[3] = { 'E','R','R' };
    if (send(sockConn, (char*)(ack), 3 * sizeof(char), 0) < 0) {
        printf("Server Send Ack Failed!\n");
    }
}

int dzsocket::socketgo::waitack(socket_num sockConn)
{
    char ack[3];
    if (recv(sockConn, (char*)(ack), 3 * sizeof(char), 0) < 0) {
        printf("Wait ack fail.\n");
        return -1;
    }
    if (strcmp(ack, "ACK") == 0)
        return 0;
    else
        return 1;
}


/**************************************************************************************** 
**************************************************************************************** 
**************************************************************************************** 
                                共享内存相关函数
**************************************************************************************** 
**************************************************************************************** 
*****************************************************************************************/
#ifdef __linux__
int share_mem_create(key_t key,bool *creator_flag)
{
    // 创建IPC键值
    // 创建共享内存
    int shmid = shmget(key, sizeof(Share_mem::share_zone), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return -1;
    }
    *creator_flag=true;  //设置共享内存创建者标志,共享内存为该进程创建
    return shmid;
}
// 获取共享内存ID
int shid_require(key_t key,bool *creator_flag)
{
    if ((shmget(key, 0, 0)) == -1) {
    // 如果返回-1，则表示错误发生，可能是该键值对应的共享内存不存在,则创建共享内存
        return share_mem_create(key,creator_flag); 
    }else { 
    // 如果返回的不是-1，则表示共享内存存在，直接返回共享内存ID
        return shmget(key, 0, 0666);
    }
}
// 绝对时间更新
void time_update(Share_mem::SMP share_mem)
{
    clock_gettime(CLOCK_REALTIME, &(share_mem->ts)); // 获取当前时间
#if (select_msec_or_sec==0)
    share_mem->ts.tv_sec += block_tim; 
   // printf("block_tim is sec\n");
#elif (block_tim>=1000)&&(select_msec_or_sec==1)
    share_mem->ts.tv_sec += block_tim/1000; 
   // printf("block_tim is sec2\n");
#else
    share_mem->ts.tv_nsec += block_tim*1000000;
   // printf("block_tim is msec\n");
#endif
}
Share_mem::SMP Share_mem::share_mem_init(int H2L_data_size ,int L2H_data_size,bool H_or_L,key_t key)
{
    bool  creator_flag=false;      //判断是否为共享内存创建者
    SMP share_mem_param;
    // 初始化共享内存参数结构体
    share_mem_param=(SMP)malloc(sizeof(smp));
    time_update(share_mem_param);
    //flag时间戳阻塞时间(ms)
    share_mem_param->flag_ts.tv_nsec = flag_tim_block_unit*1000000;
    share_mem_param->HL_ID=H_or_L;
    // 初始化共享内存结构体属性
    int cd_cache=shid_require(key,&creator_flag);
    share_mem_param->ID=cd_cache;
    printf("ssm_ID=%d\n",share_mem_param->ID);
    if ((share_mem_param->share_mem_ptr = (SZ)shmat(share_mem_param->ID, NULL, 0)) == (SZ) -1) {
        perror("shmat failed");
        fprintf(stderr, "Error code: %d\n", errno);
        exit(EXIT_FAILURE); // 或者进行适当的错误处理
    }
    // 本地参数结构体指向共享内存指针
    share_mem_param->H2L_data_size=H2L_data_size;
    share_mem_param->L2H_data_size=L2H_data_size;
    //如果为共享内存创建者则初始化标志位
    if(creator_flag){           
        share_mem_param->share_mem_ptr->H2L_flag=false;
        share_mem_param->share_mem_ptr->L2H_flag=false;
    }
    // 初始化信号量
#if H2L_EN
    // 创建信号量
    share_mem_param->H2L = sem_open("/hight_to_low", O_CREAT, 0644, 0);
    // 全局信号量初始化
    //sem_init(share_mem_param->H2L, 1, 0);
#endif
#if L2H_EN
    share_mem_param->L2H = sem_open("/low_to_high ", O_CREAT, 0644, 0);
    // 全局信号量初始化
    //sem_init(share_mem_param->H2L, 1, 0);
#endif
    // 初始化共享内存读写标志
    share_mem_param->wr_operate = share_mem_write;
    // 初始化互斥量属性
    pthread_mutexattr_t attr;
    // 初始化互斥量属性
    pthread_mutexattr_init(&attr);
    // 设置互斥量为进程间共享
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    // 初始化互斥量
    pthread_mutex_init(&(share_mem_param->share_mem_ptr->shared_mutex), &attr);
    // 连接共享内存到当前进程地址空间，并分配用于数据的空间
    return share_mem_param;
}




int Share_mem::share_mem_destroy(Share_mem::SMP share_mem_param)
{
    // 删除共享内存
    int ret = shmctl(share_mem_param->ID, IPC_RMID, NULL);
    if (ret == -1) {
        perror("shmctl");
        return -1;
    }
    // 释放内存
    return 0;
}

// 高层读写共享内存
int H_share_mem_operater(Mem_t *cache,Share_mem::SMP share_mem_param)
{
    // 操作共享内存
    if(share_mem_param->wr_operate==share_mem_write){
        // 加锁,若共享内存正在被操作，则等待
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        // 写操作
        for(int i=0;i<share_mem_param->H2L_data_size;i++){
            share_mem_param->share_mem_ptr->H2L_cache[i]=cache[i];
        }
#if H2L_EN
        sem_post(share_mem_param->H2L);
#endif
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
    }
    else{
        // 读操作
#if L2H_EN
        // 等待低层写操作完成
        time_update(share_mem_param);
        if(sem_timedwait(share_mem_param->L2H,&(share_mem_param->ts))==-1){
            return 0;
        }
#endif
        // 加锁,若共享内存正在被操作，则等待
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        for(int i=0;i<share_mem_param->L2H_data_size;i++){
            cache[i]=share_mem_param->share_mem_ptr->L2H_cache[i];
        }
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
    }
    return 1; // 程序执行成功
}

// 低层读写共享内存
int L_share_mem_operater(Mem_t *cache,Share_mem::SMP share_mem_param)
{
    // 操作共享内存
    //写操作
    if(share_mem_param->wr_operate==share_mem_write){
        // 加锁,若共享内存正在被操作，则等待
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        // 写操作
        for(int i=0;i<share_mem_param->L2H_data_size;i++){
            share_mem_param->share_mem_ptr->L2H_cache[i]=cache[i];
        }
#if L2H_EN
        sem_post(share_mem_param->L2H);
#endif
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
    }
    else{ // 读操作
#if H2L_EN
        // 等待高层写操作完成
        time_update(share_mem_param);
        if(sem_timedwait(share_mem_param->H2L,&(share_mem_param->ts))==-1){
            return 0;
        }
#endif
        // 加锁,若共享内存正在被操作，则等待
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        for(int i=0;i<share_mem_param->H2L_data_size;i++){
            cache[i]=share_mem_param->share_mem_ptr->H2L_cache[i];
            
        }
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
    }
    return 1; // 成功
}

void Share_mem::share_mem_clean(bool H_or_L,Share_mem::SMP share_mem_param)
{
    if(H_or_L==H_clean){
        // 加锁,若共享内存正在被操作，则等待
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        // 写操作
        for(int i=0;i<share_mem_param->H2L_data_size;i++){
            share_mem_param->share_mem_ptr->H2L_cache[i]=0;
        }
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
    }else{
        // 加锁,若共享内存正在被操作，则等待
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        // 写操作
        for(int i=0;i<share_mem_param->L2H_data_size;i++){
            share_mem_param->share_mem_ptr->L2H_cache[i]=0;
        }
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
    }
    
}


void Share_mem::SM_set_flag(Share_mem::SMP share_mem_param)
{
    if(share_mem_param->HL_ID==high_level){
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        // 对高对低标识符进行置1
        share_mem_param->share_mem_ptr->H2L_flag=1;
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
    }else
    {
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        // 对高对低标识符进行置1
        share_mem_param->share_mem_ptr->L2H_flag=1;
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
    }
}

bool Share_mem::SM_read_flag(Share_mem::SMP share_mem_param)
{

    if(share_mem_param->HL_ID==high_level){
#if L2H_flag_block      // 高对低阻塞读取
        for(int i=0;i < flag_block_tim*1000/flag_tim_block_unit;i++){
            pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
            bool flag=share_mem_param->share_mem_ptr->L2H_flag;
            if(flag){
                // 标识符清0
                share_mem_param->share_mem_ptr->L2H_flag=false;
                pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
                return true;
            }else{
                pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
            }
            nanosleep(&(share_mem_param->flag_ts), NULL);// 休眠flag_tim_block_unit ms
        }
        printf("H_read_flag timeout\n");
        return false;
#else
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        bool flag=share_mem_param->share_mem_ptr->L2H_flag;
        share_mem_param->share_mem_ptr->L2H_flag=false;
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
        return flag;
#endif
    }else{
#if H2L_flag_block      // 低对高阻塞读取
        for(int i=0;i < flag_block_tim*1000/flag_tim_block_unit;i++){
            pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
            // 对高对低标识符进行置1
            bool flag=share_mem_param->share_mem_ptr->H2L_flag;
            if(flag){
                // 标识符清0
                share_mem_param->share_mem_ptr->H2L_flag=false;
                pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
                return true;
            }else{
                pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
            }
            nanosleep(&(share_mem_param->flag_ts), NULL);// 休眠flag_tim_block_unit ms
        }
        printf("L_read_flag timeout\n");
        return false;
#else
        pthread_mutex_lock(&(share_mem_param->share_mem_ptr->shared_mutex));
        // 对高对低标识符进行置1
        bool flag=share_mem_param->share_mem_ptr->H2L_flag;
        // 标识符清0
        share_mem_param->share_mem_ptr->H2L_flag=false;
        // 解锁
        pthread_mutex_unlock(&(share_mem_param->share_mem_ptr->shared_mutex));
        return flag;
#endif
    }
}



void Share_mem::SM_write(Mem_t *cache,Share_mem::SMP share_mem_param)
{
    share_mem_param->wr_operate=share_mem_write;
    if(share_mem_param->HL_ID==high_level){
        H_share_mem_operater(cache,share_mem_param);
        // printf("write success");
    }else
    {
        L_share_mem_operater(cache,share_mem_param);
    }
}

int Share_mem::SM_read(Mem_t *cache,Share_mem::SMP share_mem_param)
{
    share_mem_param->wr_operate=share_mem_read;
    if(share_mem_param->HL_ID==high_level){
        return H_share_mem_operater(cache,share_mem_param);
    }else
    {
        return L_share_mem_operater(cache,share_mem_param);
    }
}
#endif
/**************************************************************************************** 
**************************************************************************************** 
**************************************************************************************** 
                                外部接口相关函数
**************************************************************************************** 
**************************************************************************************** 
*****************************************************************************************/
void dz_communicator::dz_communicator_init(bool isServer,const char* IP, int PORT)
{
    mysocket = new dzsocket::socketgo;
    do{
        if(mysocket->socketConnectNet(isServer,IP,PORT)){
            std::cout << "connect failed" << std::endl;
            sleep(1);
        }
        else{
            std::cout << "connect success" << std::endl;
            socket_enable = true;
        }
        socket_enable = true;
    }while(!socket_enable);
}
#ifdef __linux__
void dz_communicator::dz_communicator_init(bool isServer,const char* IP, int PORT,
                                            int H2L_data_size ,int L2H_data_size,bool H_or_L,key_t key)
{
    mysocket = new dzsocket::socketgo;
    do{
        if(mysocket->socketConnectNet(isServer,IP,PORT)){
            std::cout << "connect failed" << std::endl;
            sleep(1);
        }
        else{
            std::cout << "connect success" << std::endl;
            socket_enable = true;
        }
        socket_enable = true;
    }while(!socket_enable);
    // 初始化共享内存
    my_share_mem = Share_mem::share_mem_init(H2L_data_size,L2H_data_size,H_or_L,key);
    share_mem_enable = true;
}

void dz_communicator::dz_communicator_init(int H2L_data_size ,int L2H_data_size,bool H_or_L,key_t key)
{
    // 初始化共享内存
    my_share_mem = Share_mem::share_mem_init(H2L_data_size,L2H_data_size,H_or_L,key);
    share_mem_enable = true;
}

////////////////////////////////////////////////////////////////////////////////////
                                //共享内存
///////////////////////////////////////////////////////////////////////////////////
void dz_communicator::SM_write(Mem_t *cache)
{
    if(!share_mem_enable){
        std::cout << "share memory is not enable !" <<std::endl;
        return;
    }
    Share_mem::SM_write(cache,my_share_mem);
}

int dz_communicator::SM_read(Mem_t *cache)
{
    if(!share_mem_enable){
        std::cout << "share memory is not enable !" <<std::endl;
        return -1;
    }
    return Share_mem::SM_read(cache,my_share_mem);
}

void dz_communicator::share_mem_clean(bool H_or_L)
{
    if(!share_mem_enable){
        std::cout << "share memory is not enable !" <<std::endl;
        return;
    }
    Share_mem::share_mem_clean(H_or_L,my_share_mem);
}
int dz_communicator::share_mem_destroy()
{
    if(!share_mem_enable){
        std::cout << "share memory is not enable !" <<std::endl;
        return -1;
    }
    return Share_mem::share_mem_destroy(my_share_mem);
}

void dz_communicator::SM_set_flag()
{
    if(!share_mem_enable){
        std::cout << "share memory is not enable !" <<std::endl;
        return;
    }
    Share_mem::SM_set_flag(my_share_mem);
}

bool dz_communicator::SM_read_flag()
{
    if(!share_mem_enable){
        std::cout << "share memory is not enable !" <<std::endl;
        return false;
    }
    return Share_mem::SM_read_flag(my_share_mem);
}
#endif
////////////////////////////////////////////////////////////////////////////////////
                                //套接字
///////////////////////////////////////////////////////////////////////////////////
int dz_communicator::transmit(char* Data,int data_size)
{
    if(!socket_enable){
        std::cout << "socket is not enable !" <<std::endl;
        return -1;
    }
    if (mysocket->getIsServer() ==isserver)
        return mysocket->transmit(Data,data_size,mysocket->sockServer);
    else
        return mysocket->transmit(Data,data_size,mysocket->sockClient);
}

int dz_communicator::receive(char* Data,int data_size)
{
    if(!socket_enable){
        std::cout << "socket is not enable !" <<std::endl;
        return -1;
    }
    if (mysocket->getIsServer() == isserver)
        return mysocket->receive(Data,data_size,mysocket->sockServer);
    else
        return mysocket->receive(Data,data_size,mysocket->sockClient);
}

void dz_communicator::socketDisconnect()
{
    if(!socket_enable){
        std::cout << "socket is not enable !" <<std::endl;
        return;
    }
    if (mysocket->getIsServer() == isserver)
        mysocket->socketDisconnect(mysocket->sockServer);
    else
        mysocket->socketDisconnect(mysocket->sockClient);
}



