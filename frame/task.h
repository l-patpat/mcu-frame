#ifndef __TASK_H
#define __TASK_H

#define TASK_FREQ 256
#define TASK_ONCE 0
#define TASK_SEC(TICK) ((signed short)(TICK * TASK_FREQ + 0.5))
#define TASK_CREATE_PARAM(TYPE)
#define TASK_CLEAR_PARAM(NAME, TYPE)

typedef void (*_TaskHook)(void*);

typedef struct _TASK {
	_TaskHook		Hook;
	void			*Param;
	signed short	Interval;
	signed short	Counter;
	struct _TASK	*Next;
} _Task;

typedef struct _TASKLIST {
	_Task			*First;
	_Task			*Current;
	_Task			*Last;
} _TaskList;

extern void task_buf_init(void);
extern void task_init(_TaskList *tasks);
extern void *task_param_alloc(unsigned int size);
extern void task_param_free(void *param);
extern signed char task_add(_TaskList *tasks, void *hook, void *param, signed short interval);
extern void task_del(_TaskList *tasks, void *hook);
extern void task_dispatch(_TaskList *tasks);
extern void task_process(_TaskList *tasks);

#endif /* __TASK_H */
