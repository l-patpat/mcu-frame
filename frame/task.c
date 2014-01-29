#include "task.h"

#define TASK_MAX_NUMS		32
#define TASK_SIZE			sizeof(_Task)
#define TASK_BUF_SIZE		(TASK_MAX_NUMS * TASK_SIZE)
#define TASK_ITEM(NUM)		(&TaskBuf[NUM])

#define PARAM_BUF_ALIGN		(1<<2) //4
#define PARAM_BUF_SIZE		256
#define PARAM_INVALID(ADDRESS) \
	(((unsigned char*)(ADDRESS) < ParamBuf + sizeof(_Param)) || \
	((unsigned char*)(ADDRESS) > ParamBuf + PARAM_BUF_SIZE - PARAM_BUF_ALIGN))

enum _PARAM_FLAG {
	PF_FREE,
	PF_USED
};

typedef struct _PARAM {
#if PARAM_BUF_ALIGN > 2
	unsigned short	Size;
	unsigned short	Flag;
#else
	unsigned char	Size;
	unsigned char	Flag;
#endif
} _Param;

static _Task TaskBuf[TASK_MAX_NUMS];
static unsigned char ParamBuf[PARAM_BUF_SIZE];

void task_buf_init(void)
{
	unsigned char *p = (unsigned char*)TaskBuf;
	for(; p < (unsigned char*)TaskBuf + TASK_BUF_SIZE; p++)
		*p = 0;

	((_Param*)ParamBuf)->Size = PARAM_BUF_SIZE - sizeof(_Param);
	((_Param*)ParamBuf)->Flag = PF_FREE;
}

void task_list_init(_TaskList *tasks)
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
	_Param *pp, *p = (_Param*)ParamBuf;
	size += PARAM_BUF_ALIGN - 1;
	size &= ~(unsigned int)(PARAM_BUF_ALIGN - 1);

	do
	{
		if(p->Flag == PF_FREE && p->Size >= size)
		{
			pp = p;
			ret = (unsigned char*)pp + sizeof(_Param);

			if((pp->Size - size) >= PARAM_BUF_ALIGN)
			{
				p = (void*)((unsigned char*)p + size + sizeof(_Param));
				p->Size = pp->Size - size - sizeof(_Param);
				p->Flag = PF_FREE;
				pp->Size = size;
			}
			pp->Flag = PF_USED;
			break;
		}
		p = (void*)((unsigned char*)p + p->Size + sizeof(_Param));
	} while(ParamBuf + PARAM_BUF_SIZE - (unsigned char*)p >= sizeof(_Param) + PARAM_BUF_ALIGN);

	return ret;
}

void task_param_free(void *param)
{
	_Param *p, *pp;

	if(PARAM_INVALID(param))
		return;

	param = (unsigned char*)param - sizeof(_Param);
	p = (_Param*)ParamBuf;
	pp = (_Param*)ParamBuf;

	do
	{
		if(p == param)
		{
			p->Flag = PF_FREE;

			if(pp!= p && pp->Flag == PF_FREE)
			{
				pp->Size += p->Size + sizeof(_Param);
				param = pp;
				p = pp;
			}
		}
		else if(pp == param)
		{
			if(p->Flag == PF_FREE)
				pp->Size += p->Size + sizeof(_Param);

			break;
		}
		pp = p;
		p = (void*)((unsigned char*)p + p->Size + sizeof(_Param));
	} while(ParamBuf + PARAM_BUF_SIZE - (unsigned char*)p >= sizeof(_Param) + PARAM_BUF_ALIGN);
}

signed char task_exists(_TaskList *tasks, void *hook)
{
	_Task *task;

	for(task = tasks->First; task; task = task->Next)
	{
		if(task->Hook == hook)
			return 1;
	}
	return 0;
}

signed char task_add(_TaskList *tasks, void *hook, void *param, signed short interval)
{
	unsigned char *h = (unsigned char*)&TaskBuf[0].Hook;
	unsigned char i;
	
	for(i = 0; i < TASK_MAX_NUMS; i++, h += TASK_SIZE)
	{
		if(!*(_TaskHook*)h)
		{
			if(PARAM_INVALID(param))
				param = 0;
			if(tasks->Last)
				tasks->Last->Next = &TaskBuf[i];
			else
				tasks->First = &TaskBuf[i];
				
			tasks->Last = &TaskBuf[i];
			TaskBuf[i].Hook = (_TaskHook)hook;
			TaskBuf[i].Param = param;
			TaskBuf[i].Interval = interval;
			TaskBuf[i].Counter = interval;
			TaskBuf[i].Next = 0;
			return 1;
		}
	}
	return 0;
}

void task_del(_TaskList *tasks, void *hook)
{
	_Task *task, *prev;

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
					prev = task->Next;
					tasks->First = prev;
					if(!prev) tasks->Last = 0;
				}
				else if(task == tasks->Last)
				{
					prev->Next = 0;
				}
			}
			else
			{
				task->Interval = -1;
				prev = task;
			}
			if(hook) return;
		}
		else
		{
			prev = task;
		}
	}
}

void task_dispatch(_TaskList *tasks)
{
	_Task *task;

	for(task = tasks->First; task; task = task->Next)
	{
		if(task->Interval < 0)
			task_del(tasks, (void*)task->Hook);
		else if(task->Counter)
			task->Counter--;
	}
}

void task_process(_TaskList *tasks)
{
	if(!tasks->First)
		return;
		
	do
	{
		if(tasks->Current)
			tasks->Current = tasks->Current->Next;
		else
			tasks->Current = tasks->First;
			
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
