#include "sched.h"
#include <linux/slab.h>

/*更新rq中的当前进程curr*/
static void update_curr_wrr(struct rq *rq)
{
    //  printk("I Am in update_curr_wrr\n");
    struct task_struct *curr = rq->curr;
    u64 delta_exec;
    //如果当前进程不是WRR则跳过
    if (curr->sched_class != &wrr_sched_class)
        return;
    //当前进程的单次执行时间
    delta_exec = rq->clock_task - curr->se.exec_start;
    if (unlikely((s64)delta_exec < 0))
        delta_exec = 0;
    //调度统计信息取单次最大时间
    schedstat_set(curr->se.statistics.exec_max,
              max(curr->se.statistics.exec_max, delta_exec));
    //更新当前队列的运行时间
    curr->se.sum_exec_runtime += delta_exec;
    account_group_exec_runtime(curr, delta_exec);
    //更新执行开始时间
    curr->se.exec_start = rq->clock_task;
    cpuacct_charge(curr, delta_exec);
    //  printk("I quit update_curr_wrr\n");

}
//调度实体加入对应的rq
static void enqueue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se, bool head)
{
    //  printk("I Am in enqueue_wrr_entity\n");
    struct list_head *queue = &(rq->wrr.queue);
    struct task_struct *p;
    p = container_of(wrr_se, struct task_struct, wrr);
    struct task_group *tg = task_group(p);
    char group_path[1024];
    if (!(autogroup_path(tg, group_path, 1024)))
    {
        if (!tg->css.cgroup) {
            group_path[0] = '\0';
        }
        cgroup_path(tg->css.cgroup, group_path, 1024);
    }
    //进程已经调入后台但是时间片分配仍未修改
    if(group_path[1]=='b'&&p->wrr.weight!=1)
    {
        p->wrr.time_slice = WRR_BG_TIMESLICE;
        p->wrr.weight = 1;
        printk("I Am in enqueue_wrr_entity Change timeslice to background\n");
    }
    //进程已经调入前台但是时间片分配仍未修改
    else if(group_path[1]!='b' && p->wrr.weight!=10)
    {
        p->wrr.time_slice = WRR_FG_TIMESLICE;
        p->wrr.weight = 10;
        printk("I Am in enqueue_wrr_entity Change timeslice to foreground\n");
    }

    if(head)
        list_add(&wrr_se->run_list, queue);
    else
        list_add_tail(&wrr_se->run_list, queue);
    
    rq->wrr.total_weight += wrr_se->weight;
    ++rq->wrr.wrr_nr_running;
   //  printk("I quit enqueue_wrr_entity\n");
}

//add a task_struct into runqueue
static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags){
    //printk("I Am in enqueue_task_wrr\n");
    struct sched_wrr_entity *wrr_se = &p->wrr;
    enqueue_wrr_entity(rq, wrr_se, flags & ENQUEUE_HEAD);
    //更新调度队列的信息
    inc_nr_running(rq);
    // printk("I quit enqueue_task_wrr\n");
}
static void dequeue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se)
{
    //printk("I Am in dequeue_wrr_entity\n");
    //删除链表中wrr_se所指向的调度实体
    list_del_init(&wrr_se->run_list);
    rq->wrr.total_weight -= wrr_se->weight;
    --rq->wrr.wrr_nr_running;
    //  printk("I quit dequeue_wrr_entity\n");
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    //  printk("I Am in dequeue_task_wrr\n");
    struct sched_wrr_entity *wrr_se = &p->wrr;
    update_curr_wrr(rq);
    dequeue_wrr_entity(rq, wrr_se);
    //更新调度队列的信息
    dec_nr_running(rq);
    // printk("I quit dequeue_task_wrr\n");
}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags){

}
void free_wrr_sched_group(struct task_group *tg)
{
}

int alloc_wrr_sched_group(struct task_group *tg, struct task_group *parent)
{
	return 1;
}

static void task_fork_wrr(struct task_struct* p)
{
    printk("YOU ARE IN: task_fork_wrr @ WRR\n");

    p->wrr.time_slice = 0;
}

void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq){
    INIT_LIST_HEAD(&wrr_rq->queue);
    wrr_rq->wrr_nr_running=0;
    wrr_rq->total_weight=0;
}
static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
   // printk("I Am in requeue_task_wrr\n");
    struct sched_wrr_entity *wrr_se = &p->wrr;
    struct list_head *queue = &(rq->wrr.queue);

    if (head)
        list_move(&wrr_se->run_list, queue);
    else
        list_move_tail(&wrr_se->run_list, queue);
   // printk("I quit requeue_task_wrr\n");
}
 //进程主动放弃cpu时调用该函数
static void yield_task_wrr(struct rq *rq)
{
   // printk("I Am in yield_task_wrr\n");
    requeue_task_wrr(rq, rq->curr, 0);
  //  printk("I quit yield_task_wrr\n");
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
   // printk("I Am in put_prev_task_wrr\n");
    update_curr_wrr(rq);
    prev->se.exec_start = 0;
  //  printk("I quit put_prev_task_wrr\n");
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
    printk("I Am in task_tick_wrr\n");
    struct sched_wrr_entity *wrr_se = &p->wrr;
    update_curr_wrr(rq);

    struct task_group *tg = p->sched_task_group;
    char group_path[1024];
    if (!(autogroup_path(tg, group_path, 1024)))
    {
        if (!tg->css.cgroup) {
            group_path[0] = '\0';
        }
        cgroup_path(tg->css.cgroup, group_path, 1024);
    }
    //若时间片未耗尽   
    printk("pid= %d  time_slice: %d\n",p->pid,p->wrr.time_slice);
    if (--p->wrr.time_slice){
        printk("I quit task_tick_wrr\n");
        return;
    }

    //若时间片耗尽
    //根据权重weight重新分配时间片
    if(p->wrr.weight == 1)
        p->wrr.time_slice = WRR_BG_TIMESLICE;
    else
        p->wrr.time_slice = WRR_FG_TIMESLICE;

    //如果wrr_se不是rq的唯一一个调度实体
    if (wrr_se->run_list.prev != wrr_se->run_list.next) {
        requeue_task_wrr(rq, p, 0);
        //改变p的TIF_NEED_RESCHED标志位
        resched_task(p);
    }

    printk("I quit task_tick_wrr\n");
}


//在更换调度策略后调用
static void set_curr_task_wrr(struct rq *rq)
{
  //  printk("I Am in set_curr_task_wrr\n");
    struct task_struct *p = rq->curr;
    p->se.exec_start = rq->clock_task;
    //printk("I quit set_curr_task_wrr\n");
}

static struct task_struct *pick_next_task_wrr(struct rq *rq){
    //printk("I Am in pick_next_task_wrr\n");
    struct sched_wrr_entity *result;
    struct task_struct *p;
    if(rq->wrr.wrr_nr_running==0){
      //  printk("I quit pick_next_task_wrr\n");
        return NULL;
    }

    //选择链表的第一个调度实体
    result = list_first_entry(&((rq->wrr)).queue, struct sched_wrr_entity, run_list);
    p=container_of(result, struct task_struct,wrr); 
    // printk("I quit pick_next_task_wrr\n");
    return p;
   
}

static void prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio){

}
static void switched_to_wrr(struct rq *rq, struct task_struct *p){
    printk(KERN_ALERT "I Am in switched_to_wrr\n");
    struct task_group *tg = p->sched_task_group;
    char group_path[1024];
    //若p在运行对列中并且当前执行的进程不为p时
    if (p->on_rq && rq->curr != p)
    {
         //请求调度rq
        resched_task(rq->curr);
        if (!(autogroup_path(tg, group_path, 1024)))
        {
            if (!tg->css.cgroup) {
                group_path[0] = '\0';
            }
            cgroup_path(tg->css.cgroup, group_path, 1024);
        }
        //根据前后台分配权重
        if(group_path[1]=='b')
            p->wrr.weight = 1;
        else
            p->wrr.weight = 10;
    }
    printk("pid: %d switch to wrr !\n",p->pid);
    printk(KERN_ALERT "I quit switched_to_wrr\n");
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *p)
{
    printk("I Am in get_rr_interval_wrr\n");
    struct task_group *tg = p->sched_task_group;
    char group_path[1024];
    cgroup_path(tg->css.cgroup, group_path, 1024);
    printk("I quit ger_rr_interval_wrr\n");
    if(p->policy==SCHED_WRR){
        if(group_path[1]=='\0')
           return WRR_FG_TIMESLICE;
        else
           return WRR_BG_TIMESLICE;
    }
}
const struct sched_class wrr_sched_class = {

    .next           = &fair_sched_class,        /*Required*/
    .enqueue_task       = enqueue_task_wrr,     /*Required*/
    .dequeue_task       = dequeue_task_wrr,     /*Required*/
    .yield_task     = yield_task_wrr,       /*Required*/ 
    .check_preempt_curr = check_preempt_curr_wrr,       /*Required*/
    .pick_next_task     = pick_next_task_wrr,       /*Required*/
    .put_prev_task      = put_prev_task_wrr,        /*Required*/
    .task_fork          = task_fork_wrr,
#ifdef CONFIG_SMP
    .select_task_rq     = select_task_rq_wrr,          /*Never need impl */
    .set_cpus_allowed       = set_cpus_allowed_wrr,        /*Never need impl */
    .rq_online              = rq_online_wrr,           /*Never need impl */
    .rq_offline             = rq_offline_wrr,          /*Never need impl */
    .pre_schedule       = pre_schedule_wrr,        /*Never need impl */
    .post_schedule      = post_schedule_wrr,           /*Never need impl */
    .task_woken     = task_woken_wrr,          /*Never need impl */
    .switched_from      = switched_from_wrr,           /*Never need impl */
#endif
    .set_curr_task          = set_curr_task_wrr,        /*Required*/
    .task_tick      = task_tick_wrr,        /*Required*/
  
   .get_rr_interval    = get_rr_interval_wrr,
  
    .prio_changed       = prio_changed_wrr,        /*Never need impl */
  
    .switched_to        = switched_to_wrr,         /*Never need impl */

};

 
