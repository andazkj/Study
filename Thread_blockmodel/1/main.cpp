#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>7

//由于要传两个参数，所以这里定义了一个结构体
typedef struct worker_ctx_s
{
        int 				shared_var;//需要保护的数据
        pthread_mutex_t 	lock;	//引入锁
}worker_ctx_t;
/**********************************************************
* 作者：andazkj
* 名称：thread_worker1
* 功能：线程1处理函数
* 入口参数：  void *args
* 返回值： void
***********************************************************/
void *thread_worker1(void *args)
{
    worker_ctx_t *ctx = (worker_ctx_t *)args;

    if (!args)
    {
        printf("%s() get invalid arguments\n", __FUNCTION__);
        pthread_exit(NULL);
    }

    printf("%s [%ld] start running...@%s-%s\n", __FUNCTION__, pthread_self(), __DATE__, __TIME__);

    /* 修改数据，加锁保护 */
    while (1)
    {
        /* 加阻塞锁 */
        pthread_mutex_lock(&ctx->lock);
        printf("+++: %s before shared_var++: %d\n", __FUNCTION__, ctx->shared_var);
        ctx->shared_var ++;
        sleep(2);
        printf("+++: %s after sleep shared_var: %d\n", __FUNCTION__, ctx->shared_var);
        /* 解开阻塞锁 */
        pthread_mutex_unlock(&ctx->lock);
        sleep(1);
    }

    printf("%s exit...@%s-%s\n", __FUNCTION__, __DATE__, __TIME__);

    return NULL;
}
/**********************************************************
* 作者：andazkj
* 名称：thread_worker2
* 功能：线程2处理函数
* 入口参数：  void *args
* 返回值： void
***********************************************************/
void *thread_worker2(void *args)
{
    worker_ctx_t *ctx = (worker_ctx_t *)args;

    if (!args)
    {
        printf("%s() get invalid arguments\n", __FUNCTION__);
        pthread_exit(NULL);
    }

    printf("%s [%ld] start running...@%s-%s\n", __FUNCTION__, pthread_self(), __DATE__, __TIME__);

    while(1)
    {
        /* 加阻塞锁 */
        if (0 != pthread_mutex_trylock(&ctx->lock))
        {
            continue;
        }

        printf("---: %s before shared_var++: %d\n", __FUNCTION__, ctx->shared_var);
        ctx->shared_var ++;
        sleep(2);
        printf("---: %s after sleep shared_var: %d\n", __FUNCTION__, ctx->shared_var);
        /* 解开阻塞锁 */
        pthread_mutex_unlock(&ctx->lock);
        sleep(1);
    }

     printf("%s exit...@%s-%s\n", __FUNCTION__, __DATE__, __TIME__);

    return NULL;
 }
int main(int argc, char **argv)
{
        worker_ctx_t 		worker_ctx;
        pthread_t			tid;
        pthread_attr_t 		thread_attr;

        /* 初始化互斥锁 */
        worker_ctx.shared_var = 1000;
        pthread_mutex_init(&worker_ctx.lock, NULL);

        /* 初始化属性 */
        if (pthread_attr_init(&thread_attr))
        {
            printf("pthread_attr_init() failure: %s\n", strerror(errno));
            return -1;
        }

        if (pthread_attr_setstacksize(&thread_attr, 120*1024))
        {
            printf("pthread_attr_setstacksize() failure: %s\n", strerror(errno));
            return -1;
        }

        if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED))
        {
            printf("pthread_attr_setdetachstate() failure: %s\n", strerror(errno));
            return -1;
        }

        pthread_create(&tid, &thread_attr, thread_worker1, &worker_ctx);
        printf("Thread worker1 tid[%ld] created ok\n", tid);

        pthread_create(&tid, &thread_attr, thread_worker2, &worker_ctx);
        printf("Thread worker2 tid[%ld] created ok\n", tid);

        while (1)
        {
            printf("Main/Control thread shared_var: %d\n", worker_ctx.shared_var);
            sleep(10);
        }

        pthread_mutex_destroy(&worker_ctx.lock);
}
