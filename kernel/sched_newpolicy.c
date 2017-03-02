#include <linux/random.h>

void init_newpolicy_rq(struct NEWPOLICY_rq *newpolicy_rq)
{
	INIT_LIST_HEAD(&newpolicy_rq->NEWPOLICY_list_head);
	atomic_set(&newpolicy_rq->nr_running,0);
}

static void enqueue_task_newpolicy(struct rq *rq, struct task_struct *p, int wakeup, bool head)
{
	if(p){
		struct NEWPOLICY_rq *newNode;
		newNode = (struct NEWPOLICY_rq *) kzalloc (sizeof(struct NEWPOLICY_rq), GFP_ATOMIC);
		if (newNode == NULL) {
			printk(KERN_INFO "Error in enqueue_task_newpolicy: kzalloc\n");
			return;
		}
		newNode->task = p;

		list_add (&(newNode->NEWPOLICY_list_head), &(rq->NEWPOLICY_rq.NEWPOLICY_list_head));
		atomic_inc(&rq->NEWPOLICY_rq.nr_running);
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
				atomic_dec(&rq->NEWPOLICY_rq.nr_running);
				return;
			}
		}
	}
}

// todo
static struct task_struct *pick_next_task_newpolicy(struct rq *rq)
{
	struct NEWPOLICY_rq *t=NULL;
	struct NEWPOLICY_rq *tempNode, *next;
	unsigned long long totalTickets = 0;
	unsigned long long runningTotal = 0;
	unsigned int *randomNumber;
	list_for_each_entry_safe (tempNode, next, &(rq->NEWPOLICY_rq.NEWPOLICY_list_head), NEWPOLICY_list_head) {
		if (tempNode) {
			totalTickets += tempNode->task->numTickets;
		}
	}
	if (totalTickets == 0) {
		return NULL;
	}

	// generate random number from 1-totalTickets
	randomNumber = (int*) kzalloc(sizeof(int), GFP_ATOMIC);

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
		if (tempNode) {
			runningTotal += tempNode->task->numTickets;  // would be atleast 1
			if (runningTotal >= *randomNumber) {
				t = tempNode;
			}
		}
	}	
	kfree(randomNumber);

	if(t){
		return t->task;
	}
	return NULL;
}

static void check_preempt_curr_newpolicy(struct rq *rq, struct task_struct *p, int flags)
{
	// do nothing...we only reschedule on timer tick
}


static void put_prev_task_newpolicy(struct rq *rq, struct task_struct *p)
{
	enqueue_task_newpolicy(rq, p, 0, false);
}

static void set_curr_task_newpolicy(struct rq *rq)
{
	// empty
}


static void task_tick_newpolicy(struct rq *rq, struct task_struct *p, int queued)
{
	if (queued) {
		// resched_task(rq->curr);
		resched_task(p);
	}
}




static void yield_task_newpolicy(struct rq *rq)
{
	// resched_task(rq->curr);
	// dont need to do anything
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
}



static void prio_changed_newpolicy(struct rq *rq, struct task_struct *p,
			    int oldprio, int running)
{
	p->numTickets = MAX_TICKETS - p->prio;
}



static const struct sched_class newpolicy_sched_class = {
	.next 			= &fair_sched_class,
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

};
