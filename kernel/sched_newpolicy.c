#include <linux/random.h>

#ifndef NEWPOLICY_CLASS
#define NEWPOLICY_CLASS
const struct sched_class newpolicy_sched_class;
#endif
void init_newpolicy_rq(struct NEWPOLICY_rq *newpolicy_rq)
{
	INIT_LIST_HEAD(&newpolicy_rq->NEWPOLICY_list_head);
	atomic_set(&newpolicy_rq->nr_running,0);
}

static void enqueue_task_newpolicy(struct rq *rq, struct task_struct *p, int wakeup, bool head)
{
	if(p){
		struct NEWPOLICY_rq *newNode;
		p->rt.time_slice = DEF_TIMESLICE;  // reset
		// #define DEF_TIMESLICE		(100 * HZ / 1000)
		newNode = (struct NEWPOLICY_rq *) kzalloc (sizeof(struct NEWPOLICY_rq), GFP_KERNEL);
		if (newNode == NULL) {
			printk(KERN_INFO "Error in enqueue_task_newpolicy: kzalloc\n");
			return;
		}
		newNode->task = p;

		list_add (&(newNode->NEWPOLICY_list_head), &(rq->NEWPOLICY_rq.NEWPOLICY_list_head));
		atomic_inc(&(rq->NEWPOLICY_rq.nr_running));
	}
}

static void dequeue_task_newpolicy(struct rq *rq, struct task_struct *p, int sleep)
{
	if(rq && p) {
		struct NEWPOLICY_rq *tempNode, *next;
		list_for_each_entry_safe (tempNode, next, &(rq->NEWPOLICY_rq.NEWPOLICY_list_head), NEWPOLICY_list_head) {
			if (tempNode && tempNode->task == p) {
				list_del(&tempNode->NEWPOLICY_list_head);
				kfree(tempNode);
				atomic_dec(&(rq->NEWPOLICY_rq.nr_running));
				return;
			}
		}
	}
}


static struct task_struct *pick_next_task_newpolicy(struct rq *rq)
{
	struct NEWPOLICY_rq *t=NULL;
	struct NEWPOLICY_rq *tempNode, *next;
	unsigned long long totalTickets = 0;
	unsigned long long runningTotal = 0;
	unsigned int *randomNumber;
	if (rq->NEWPOLICY_rq.nr_running.counter == 0)
        	return NULL;

	list_for_each_entry_safe (tempNode, next, &(rq->NEWPOLICY_rq.NEWPOLICY_list_head), NEWPOLICY_list_head) {
		if (tempNode && tempNode->task) {
			totalTickets += tempNode->task->numTickets;
		}
	}
	if (totalTickets == 0) {
		return NULL;
	}

	// generate random number from 1-totalTickets
	randomNumber = (int*) kzalloc(sizeof(int), GFP_KERNEL);

	if (randomNumber == NULL) {
		printk(KERN_INFO "Error in pick_next_task_newpolicy: kzalloc\n");
		return NULL;
	}

	*randomNumber = 0;
	get_random_bytes(randomNumber, sizeof(unsigned int));
	*randomNumber = (*randomNumber)%totalTickets +1;
	// while (*randomNumber < 1 && *randomNumber > totalTickets)
		// get_random_bytes(randomNumber, sizeof(unsigned int));

	list_for_each_entry_safe (tempNode, next, &(rq->NEWPOLICY_rq.NEWPOLICY_list_head), NEWPOLICY_list_head) {
		if (tempNode && tempNode->task) {
			runningTotal += tempNode->task->numTickets;  // would be atleast 1
			if (runningTotal >= *randomNumber) {
				t = tempNode;
			}
		}
	}	
	kfree(randomNumber);

	if(t && t->task){
		//dequeue_task_newpolicy(rq, t->task, 0);
		return t->task;
	}
	return NULL;
}

static void check_preempt_curr_newpolicy(struct rq *rq, struct task_struct *p, int flags)
{
	// This function checks if a task that entered the runnable state should
	// preempt the currently running task.
		//if (p->sched_class != &idle_sched_class && p->sched_class != &fair_sched_class )
		if (rq->curr->policy!=SCHED_NEWPOLICY) {
			resched_task(rq->curr); 
		}
		else if (p->policy!=SCHED_IDLE && p->policy!=SCHED_BATCH) {
			resched_task(rq->curr); 
		}	 
		//else if (p->policy==SCHED_NEWPOLICY) { 
			// preempt without checking prio because it doesnt matter
        	//	resched_task(rq->curr);
		//} 
		else if (rt_prio(p->prio)) {
			resched_task(rq->curr);
		} 
//else if (p->sched_class != &idle_sched_class) {
		//	resched_task(rq->curr);
		//}
}


static void put_prev_task_newpolicy(struct rq *rq, struct task_struct *p)
{
	if (p->state & TASK_DEAD)
		return;
	if (p->state != TASK_RUNNING)
		return;

	if(rq && p) {
		struct NEWPOLICY_rq *tempNode, *next;
		p->rt.time_slice = DEF_TIMESLICE;  // reset timeslice 
		//#define DEF_TIMESLICE		(100 * HZ / 1000)
		
		list_for_each_entry_safe (tempNode, next, &(rq->NEWPOLICY_rq.NEWPOLICY_list_head), NEWPOLICY_list_head) {
			if (tempNode && tempNode->task == p) {
				// already exists
				return;
			}
		}
	}
	enqueue_task_newpolicy(rq, p, 0, false);
}

static void set_curr_task_newpolicy(struct rq *rq)
{
/* Account for a task changing its policy or group.
 *
 * This routine is mostly called to set cfs_rq->curr field when a task
 * migrates between groups/classes.
 */

	rq->curr->numTickets = MAX_TICKETS - rq->curr->prio;
}


static void task_tick_newpolicy(struct rq *rq, struct task_struct *p, int queued)
{
	if (--p->rt.time_slice)  // decrement timeslice and if not 0 no need to do anything
		return;

	p->rt.time_slice = DEF_TIMESLICE;  // reset
	// #define DEF_TIMESLICE		(100 * HZ / 1000)

	if (rq->NEWPOLICY_rq.nr_running.counter > 1)
		resched_task(p);
	
	//atomic_t i = 1;
	//if (queued && rq->NEWPOLICY_rq.nr_running.counter > 1) {  // dont know what queued variable is
	//	resched_task(rq->curr);
		//resched_task(p);
	//} //else {
	//	if (rq->nr_running > 1)
        //		check_preempt_tick(cfs_rq, curr);
	//}
}




static void yield_task_newpolicy(struct rq *rq)
{
	//resched_task(rq->curr);
	// dont need to do anything

	// This function is basically just a dequeue followed by an enqueue, unless the
	// compat_yield sysctl is turned on; in that case, it places the scheduling
	// entity at the right-most end of the red-black tree.
}

/*
 * When switching a task to RT, we may overload the runqueue
 * with RT tasks. In this case we try to push them off to
 * other runqueues.
 */
static void switched_to_newpolicy(struct rq *rq, struct task_struct *p,
                           int running)
{
	p->numTickets = MAX_TICKETS - p->prio;
	//if (running)
	//	resched_task(rq->curr);
        
}



static void prio_changed_newpolicy(struct rq *rq, struct task_struct *p,
			    int oldprio, int running)
{
	p->numTickets = MAX_TICKETS - p->prio;
}

static int select_task_rq_newpolicy(struct rq *rq, struct task_struct *p, int sd_flag, int flags) 
{ 
 
//  struct rq *rq = task_rq(p); 
 
	if (sd_flag != SD_BALANCE_WAKE) 
		return smp_processor_id(); 
 
	return task_cpu(p); 
} 


const struct sched_class newpolicy_sched_class = {
	.next 			= &idle_sched_class,
	.enqueue_task		= enqueue_task_newpolicy,
	.dequeue_task		= dequeue_task_newpolicy,

	.check_preempt_curr	= check_preempt_curr_newpolicy,

	.pick_next_task		= pick_next_task_newpolicy,
	.put_prev_task		= put_prev_task_newpolicy,

	.set_curr_task          = set_curr_task_newpolicy,
	.task_tick		= task_tick_newpolicy,
	.yield_task		= yield_task_newpolicy,

	// added later
	.switched_to		= switched_to_newpolicy,
	.prio_changed		= prio_changed_newpolicy,


//	.get_rr_interval	= get_rr_interval_rt,

#ifdef CONFIG_SMP 
	.select_task_rq		= select_task_rq_newpolicy,

#endif
};
