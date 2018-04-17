# ReadWriteLock
(1) java中有wait()、notify()、notifyall()
Linux C中有一组函数和它们是一一对应的，实现完全相同的功能。
int  pthread_cond_wait(pthread_cond_t  *cond,pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t * cond);//随机唤醒一个等待的线程
int pthread_cond_broadcast(pthread_cond_t * cond);//唤醒所有等待的线程

(2)当有线程在conditon上等待的时候互斥锁失效

