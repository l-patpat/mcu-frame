#ifndef __TASK_H
#define __TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_FREQ 256
#define TASK_ONCE 0
#define TASK_SEC(TICK) ((signed short)(TICK * TASK_FREQ + 0.5))
#define TASK_CREATE_PARAM(TYPE)
#define TASK_CLEAR_PARAM(NAME, TYPE)

typedef void (*f_TaskHook)(void*);

typedef struct S_TASK {
	f_TaskHook		Hook;
	void			*Param;
	signed short	Interval;
	signed short	Counter;
	struct S_TASK	*Next;
} s_Task;

typedef struct S_TASKLIST {
	s_Task			*First;
	s_Task			*Last;
	s_Task			*Current;
	char			Processing;
	char			Changed;
} s_TaskList;

extern void task_buf_init(void);
extern void task_list_init(s_TaskList *tasks);
extern void *task_param_alloc(unsigned int size);
extern void task_param_free(void *param);
extern signed char task_exists(s_TaskList *tasks, void *hook);
extern signed char task_add(s_TaskList *tasks, void *hook, void *param, signed short interval);
extern void task_del(s_TaskList *tasks, void *hook);
extern void task_dispatch(s_TaskList *tasks);
extern void task_process(s_TaskList *tasks);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_H */
