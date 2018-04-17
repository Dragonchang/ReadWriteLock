#include<pthread.h>
#include<stdio.h>
#include <unistd.h>
#include <time.h>  
#include<sys/time.h>
class readwritelock {
    public:
        readwritelock()
        {
            stat = 0;
            if (pthread_mutex_init(&mtx, 0) != 0)
                printf("mutex init failed\n");

            if(pthread_cond_init( &cond, NULL)!= 0)
		printf("cond init failed\n");
        }
        ~readwritelock()
        {
   		pthread_mutex_destroy( &mtx ) ;
   		pthread_cond_destroy( &cond ) ;
        }
        void readLock()
        {
	    //printf("enter readLock-a stat: %d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            pthread_mutex_lock(&mtx);
  	    printf("enter readLock-b stat:%d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            while (stat < 0) {
	          printf("readLock wait for write lock stat%d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
 		  //当有线程在conditon上等待的时候互斥锁失效
                  pthread_cond_wait(&cond, &mtx );
	    }
            ++stat;
            printf("leave readLock stat: %d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            pthread_mutex_unlock(&mtx);
        }
      
        void readUnlock()
        {
	    //printf("enter readUnlock-a stat: %d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            pthread_mutex_lock(&mtx);
   	    printf("enter readUnlock-b stat: %d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            if (--stat == 0) {
                printf("wake up a write lock stat:%d pid:%d tid:%d\n",stat, getpid(),(unsigned)pthread_self());
                pthread_cond_signal( &cond );   // 叫醒一个等待的写操作
	    }
  	    printf("leave readUnlock stat%d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            pthread_mutex_unlock(&mtx);
        }
      
        void writeLock()
        {
       	    //printf("enter writeLock-a stat%d pid:%d tid:%d\n",stat, getpid(),(unsigned)pthread_self());
            pthread_mutex_lock(&mtx);
	    printf("enter writeLock-b stat%d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            while (stat != 0) {
                printf("writeLock wait for read or write lock stat%d pid:%d tid:%d\n",stat, getpid(),(unsigned)pthread_self());
		//当有线程在conditon上等待的时候互斥锁失效
                pthread_cond_wait(&cond, &mtx );
 	    }
            stat = -1;
	    printf("leave writeLock-b stat:%d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            pthread_mutex_unlock(&mtx);
	    //printf("leave writeLock-a stat:%d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
        }
      
        void writeUnlock()
        {
	    //printf("enter writeUnlock-a stat:%d pid:%d tid:%d\n",stat, getpid(),(unsigned)pthread_self());
            pthread_mutex_lock(&mtx);
	    printf("enter writeUnlock-b stat:%d pid:%d tid:%d\n",stat, getpid(),(unsigned)pthread_self());
            stat = 0;
            pthread_cond_broadcast( &cond ); // 叫醒所有等待的读和写操作
	    printf("leave writeUnlock-b stat:%d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
            pthread_mutex_unlock(&mtx);
	    //printf("leave writeUnlock-a stat:%d pid:%d tid:%d\n",stat,getpid(),(unsigned)pthread_self());
        }

    private:
        pthread_mutex_t mtx;
        pthread_cond_t cond;
        int stat; // == 0 无锁；> 0 已加读锁个数；< 0 已加写锁
};

readwritelock *mRW_Lock;


void read() {
    printf("read enter pid:%d tid:%d\n",getpid(),(unsigned)pthread_self());
    sleep(2);
    printf("read leave pid:%d tid:%d\n",getpid(),(unsigned)pthread_self());
}

void write() {
    printf("write pid:%d tid:%d\n",getpid(),(unsigned)pthread_self());
    sleep(1);
    printf("write leave pid:%d tid:%d\n",getpid(),(unsigned)pthread_self());
}

void * readrun(void *arg)
{
    printf("enter readrun pid:%d tid:%d\n",getpid(),(unsigned)pthread_self());
    mRW_Lock->readLock();
    read();
    mRW_Lock->readUnlock();
}

void * writerun(void *arg)
{
    printf("enter writerun pid:%d tid:%d\n",getpid(),(unsigned)pthread_self());
    mRW_Lock->writeLock();
    write();
    mRW_Lock->writeUnlock();
}

int main(void)
{
    mRW_Lock = new readwritelock();
    struct timeval startTime,endTime;  
    float Timeuse;
    //****begin***** case 1 有多个read没有write 
    printf("case1***begin:有多个read没有write\n");  
    gettimeofday(&startTime,NULL);
    pthread_t g_read_thread_case1[10];
    for (int i = 0; i< 10;i++) {
        pthread_create( &g_read_thread_case1[i], NULL, readrun, NULL) ;  
    }
    for ( int i = 0; i < 10; ++ i )   
    {  
      pthread_join(g_read_thread_case1[i], NULL ) ;  
    } 
    gettimeofday(&endTime,NULL);
    Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    Timeuse /= 1000000;
    printf("case1 Timeuse = %f\n",Timeuse);
    //****end***** case 1 有多个read没有write

    //****begin***** case 2 有多个write没有read 
    printf("case2***begin:有多个write没有read\n");
    gettimeofday(&startTime,NULL);
    pthread_t g_write_thread_case2[10];
    for (int i = 0; i< 10;i++) {
        pthread_create( &g_write_thread_case2[i], NULL, writerun, NULL) ;  
    }
    for ( int i = 0; i < 10; ++ i )   
    {  
      pthread_join(g_write_thread_case2[i], NULL ) ;  
    } 
    gettimeofday(&endTime,NULL);
    Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    Timeuse /= 1000000;
    printf("case2 Timeuse = %f\n",Timeuse);
    //****end***** case 2 有多个write没有read 

    //****begin***** case 3 有多个read有一个write 
    printf("case3***begin:有多个read有一个write\n");  
    gettimeofday(&startTime,NULL);
    pthread_t g_read_thread_case3[10];
    pthread_t g_write_thread_case3;
    pthread_create( &g_write_thread_case3, NULL, writerun, NULL) ;
    for (int i = 0; i< 10;i++) {
        pthread_create( &g_read_thread_case3[i], NULL, readrun, NULL) ;  
    }
    for ( int i = 0; i < 10; ++ i )   
    {  
      pthread_join(g_read_thread_case3[i], NULL ) ;  
    } 
    pthread_join(g_write_thread_case3, NULL ) ; 
    gettimeofday(&endTime,NULL);
    Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    Timeuse /= 1000000;
    printf("case3 Timeuse = %f\n",Timeuse);
    //****end***** case 3 有多个read有一个write

    //****begin***** case 4 有多个write有一个read 
    printf("case4***begin:有多个write有一个read\n");  
    gettimeofday(&startTime,NULL);
    pthread_t g_write_thread_case4[10];
    pthread_t g_read_thread_case4;
    pthread_create( &g_read_thread_case4, NULL, readrun, NULL) ;
    for (int i = 0; i< 10;i++) {
        pthread_create( &g_write_thread_case4[i], NULL, writerun, NULL) ;  
    }
    for ( int i = 0; i < 10; ++ i )   
    {  
      pthread_join(g_write_thread_case4[i], NULL ) ;  
    } 
    pthread_join(g_read_thread_case4, NULL ) ; 
    gettimeofday(&endTime,NULL);
    Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    Timeuse /= 1000000;
    printf("case4 Timeuse = %f\n",Timeuse);
    //****end***** case 4 有多个write有一个read

    //****begin***** case 5 有多个write有多个read 
    printf("case5***begin:有多个write有多个read\n");  
    gettimeofday(&startTime,NULL);
    pthread_t g_write_thread_case5[10];
    pthread_t g_read_thread_case5[10];
    for (int i = 0; i< 10;i++) {
        pthread_create( &g_write_thread_case5[i], NULL, writerun, NULL) ;  
    }
    for (int i = 0; i< 10;i++) {
        pthread_create( &g_read_thread_case5[i], NULL, readrun, NULL) ;  
    }
    for ( int i = 0; i < 10; ++ i )   
    {  
      pthread_join(g_write_thread_case5[i], NULL ) ;  
    } 
    for ( int i = 0; i < 10; ++ i )   
    {  
      pthread_join(g_read_thread_case5[i], NULL ) ;  
    }  
    gettimeofday(&endTime,NULL);
    Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    Timeuse /= 1000000;
    printf("case5 Timeuse = %f\n",Timeuse);
    //****end***** case 5 有多个write有多个read
}









































