#include <common.h>

struct task {
  // TODO
  union{
    struct{
      int pid,cpu,status;
      const char *name;
      void (*entry)(void*);
      void *arg;
      Context *context;
      list_head list;
      list_head sem_list;
      uint32_t canary;
    };
    uint8_t data[TASK_SIZE];
  };
};

struct spinlock {
  // TODO
  bool locked;
  const char *name;
  int cpu;
};

struct semaphore {
  // TODO
  const char *name;
  int count;
  spinlock_t lock;
  list_head blocked_task;  //被阻塞的task
};
