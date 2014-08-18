#include "task.h"

#define TASK_MAX_NUMS		32
#define TASK_SIZE			sizeof(s_Task)
#define TASK_BUF_SIZE		(TASK_MAX_NUMS * TASK_SIZE)
#define TASK_ITEM(NUM)		(&TaskBuf[NUM])

#define PARAM_BUF_ALIGN		(1<<2) //4
#define PARAM_BUF_SIZE		256
#define PARAM_INVALID(ADDRESS) \
	(((unsigned char*)(ADDRESS) < ParamBuf + sizeof(s_Param)) || \
	((unsigned char*)(ADDRESS) > ParamBuf + PARAM_BUF_SIZE - PARAM_BUF_ALIGN))

enum E_PARAM_FLAG {
	PF_FREE,
	PF_USED
};

typedef struct S_PARAM {
#if PARAM_BUF_ALIGN > 2
	unsigned short	Size;
	unsigned short	Flag;
#else
	unsigned char	Size;
	unsigned char	Flag;
#endif
} s_Param;

static s_Task TaskBuf[TASK_MAX_NUMS];
static unsigned char ParamBuf[PARAM_BUF_SIZE];

void task_buf_init(void)
{
	unsigned char *p = (unsigned char*)TaskBuf;
	for(; p < (unsigned char*)TaskBuf + TASK_BUF_SIZE; p++)
	{
		*p = 0;
	}

	((s_Param*)ParamBuf)->Size = PARAM_BUF_SIZE - sizeof(s_Param);
	((s_Param*)ParamBuf)->Flag = PF_FREE;
}

void task_list_init(s_TaskList *tasks)
{
	tasks->First = 0;
	tasks->Current = 0;
	tasks->Last = 0;
	tasks->Processing = 0;
	tasks->Changed = 0;
}

void *task_param_alloc(unsigned int size)
{
	void *ret = 0;
	s_Param *pp, *p = (s_Param*)ParamBuf;
	size += PARAM_BUF_ALIGN - 1;
	size &= ~(unsigned int)(PARAM_BUF_ALIGN - 1);

	do
	{
		if(p->Flag == PF_FREE && p->Size >= size)
		{
			pp = p;
			ret = (unsigned char*)pp + sizeof(s_Param);

			if((pp->Size - size) >= PARAM_BUF_ALIGN)
			{
				p = (void*)((unsigned char*)p + size + sizeof(s_Param));
				p->Size = pp->Size - size - sizeof(s_Param);
				p->Flag = PF_FREE;
				pp->Size = size;
			}
			pp->Flag = PF_USED;
			break;
		}
		p = (void*)((unsigned char*)p + p->Size + sizeof(s_Param));
	} while(ParamBuf + PARAM_BUF_SIZE - (unsigned char*)p >= sizeof(s_Param) + PARAM_BUF_ALIGN);

	return ret;
}

void task_param_free(void *param)
{
	s_Param *p, *pp;

	if(PARAM_INVALID(param))
	{
		return;
	}

	param = (unsigned char*)param - sizeof(s_Param);
	p = (s_Param*)ParamBuf;
	pp = (s_Param*)ParamBuf;

	do
	{
		if(p == param)
		{
			p->Flag = PF_FREE;

			if(pp!= p && pp->Flag == PF_FREE)
			{
				pp->Size += p->Size + sizeof(s_Param);
				param = pp;
				p = pp;
			}
		}
		else if(pp == param)
		{
			if(p->Flag == PF_FREE)
			{
				pp->Size += p->Size + sizeof(s_Param);
			}
			break;
		}
		pp = p;
		p = (void*)((unsigned char*)p + p->Size + sizeof(s_Param));
	} while(ParamBuf + PARAM_BUF_SIZE - (unsigned char*)p >= sizeof(s_Param) + PARAM_BUF_ALIGN);
}

signed char task_exists(s_TaskList *tasks, void *hook)
{
	s_Task *task;

	for(task = tasks->First; task; task = task->Next)
	{
		if(task->Hook == hook)
		{
			return 1;
		}
	}
	return 0;
}

signed char task_add(s_TaskList *tasks, void *hook, void *param, signed short interval)
{
	unsigned char *h = (unsigned char*)&TaskBuf[0].Hook;
	unsigned char i;
	
	for(i = 0; i < TASK_MAX_NUMS; i++)
	{
		if(!*(f_TaskHook*)h)
		{
			if(PARAM_INVALID(param))
			{
				param = 0;
			}
			
			if(tasks->Last)
			{
				tasks->Last->Next = &TaskBuf[i];
			}
			else
			{
				tasks->First = &TaskBuf[i];
			}
			tasks->Last = &TaskBuf[i];
			TaskBuf[i].Hook = (f_TaskHook)hook;
			TaskBuf[i].Param = param;
			TaskBuf[i].Interval = interval;
			TaskBuf[i].Counter = interval;
			TaskBuf[i].Next = 0;
			return 1;
		}
		h += TASK_SIZE;
	}
	return 0;
}

void task_del(s_TaskList *tasks, void *hook)
{
	s_Task *task, *prev;

	for(task = prev = tasks->First; task; task = task->Next)
	{
		if(task->Hook == hook || !hook)
		{
			tasks->Changed = 1;
			
			if(task != tasks->Current || !tasks->Processing)
			{
				task_param_free(task->Param);
				task->Hook = 0;
				
				if(task == tasks->First)
				{
					tasks->First = task->Next;
					if(!tasks->First)
					{
						tasks->Last = 0;
					}
				}
				else if(task == tasks->Last)
				{
					tasks->Last = prev;
					prev->Next = 0;
				}
				else
				{
					prev->Next = task->Next;
				}
			}
			else
			{
				task->Interval = -1;
				prev = task;
			}
			
			if(hook)
			{
				return;
			}
		}
		else
		{
			prev = task;
		}
	}
}

void task_dispatch(s_TaskList *tasks)
{
	s_Task *task;

	for(task = tasks->First; task; task = task->Next)
	{
		if(task->Interval < 0)
		{
			task_del(tasks, (void*)task->Hook);
		}
		else if(task->Counter)
		{
			task->Counter--;
		}
	}
}

void task_process(s_TaskList *tasks)
{
	if(!tasks->First)
	{
		return;
	}
		
	do
	{
		if(tasks->Current)
		{
			tasks->Current = tasks->Current->Next;
		}
		else
		{
			tasks->Current = tasks->First;
		}
			
		if(tasks->Current && tasks->Current->Interval >= 0)
		{
			tasks->Processing = 1;
			
			if(tasks->Changed)
			{
				tasks->Current = 0;
				tasks->Changed = 0;
				break;
			}
			
			if(tasks->Current->Hook && !tasks->Current->Counter)
			{
				if(!tasks->Current->Interval)
					tasks->Current->Interval--;
					
				tasks->Current->Hook(tasks->Current->Param);
				tasks->Current->Counter = tasks->Current->Interval;
				break;
			}
			tasks->Processing = 0;
		}
	} while(tasks->Current);
	
	tasks->Processing = 0;
}
