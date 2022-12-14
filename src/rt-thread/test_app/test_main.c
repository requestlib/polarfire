#include<rtthread.h>
#include<board.h>
#include<rthw.h>
#include<test_tool.h>

#define THREAD_PRIORITY 5
#define THREAD_STACK_SIZE 1024
#define THREAD_TIMESLICE 5
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;
static rt_thread_t tid_monitor = RT_NULL;
static int para1 = 0;
static int para2 = 2;

/* 线程1 入口函数*/
static void thread_entry(void *parameter)
{
    int cal_num = 80000;
    while(1){
        int wait_start = rt_cpu_self()->tick;
        int delay_time = 1000*(*(int *)parameter);
        if(delay_time){
            srand(wait_start);
            rt_thread_delay(rand()%delay_time);
        }
        primary_cal_test(cal_num);
        int end = rt_cpu_self()->tick;
        switch((end-wait_start)%3){
            case 0:
                rgb_all_off();
                led_b_on();
                break;
            case 1:
                rgb_all_off();
                led_r_on();
                break;
            case 2:
                rgb_all_off();
                led_g_on();
                break;
            default:
                break;
        }
    }


}

static void moniter_thread_entry(void *parameter){
    int count=0;
    while(1){
        rt_thread_delay(1000);
        for(int i=0;i<RT_CPUS_NR;i++){
            rt_kprintf("cpu[%d] usage:",i);
            rt_kprintf(" %d%%\n", (int)get_cpu_usage_float(i));
            rt_kprintf("total_tick:%d, idle_tick:%d\n",rt_cpu_index(i)->recent_total_ticks,rt_cpu_index(i)->idle_ticks);
        }
        list_thread();
    }
}

void test1(){
    // 测试1 存在空闲核
    // 核0持续处理任务 核1空闲
    int core = rt_hw_cpu_id();
    rt_kprintf("Core %d Hello world \n", core);
}

int main()
{
   test1(); //测试1 存在空闲核
   return 0;
}
