#include <os.h>
#include <syscall.h>

// #include "initcode.inc"
int kputc(task_t *task, char ch) {
  putch(ch); // safe for qemu even if not lock-protected
  return 0;
}

int getpid(task_t *task) {
  return task->pid; // 返回当前进程的进程号
}

int sleep(task_t *task, int seconds) {
  int64_t wakeup_time = uptime(task) + seconds * 1000; // 转换为毫秒

  while (uptime(task) < wakeup_time) {
    os->yield(task); // 让出 CPU，等待唤醒
  }

  return 0;
}

int64_t uptime(task_t *task) {
  return timer_get_ticks() * 10; // 将 timer 的 tick 转换为毫秒
}

int fork(task_t *task) {
    task_t *child = pmm->alloc(sizeof(task_t));
    if (child == NULL) {
        return -1; // 失败，返回负数表示错误
    }

    // 复制父进程的内容到子进程
    memcpy(child, task, sizeof(task_t));

    // 设置子进程的返回值为 0，设置父进程的返回值为子进程的进程号
    if (task->pid == child->pid) {
        task->context->GPRx = child->pid; // 父进程返回子进程的进程号
    } else {
        child->context->GPRx = 0; // 子进程返回 0
    }

    // 进行 Copy-on-Write 处理
    // 这里需要根据具体的内存管理机制实现，标记父子进程的页面为只读
    // 在页面访问异常时，根据需要进行页面复制和写权限修改

    // 增加引用计数等其他必要的处理

    // 返回 0 表示成功
    return 0;
}

int wait(task_t *task, int *status) {
    // 遍历子进程列表，寻找直接子进程
    for (int i = 1; i < task->pid; i++) {
        task_t *child = &cpu_task[task->cpu].idle_task; // 初始化为一个默认值
        // TODO: 在你的数据结构中查找子进程 i 的 task_t 指针

        if (child && child->status != SLEEP) {
            // 子进程找到，等待其结束
            while (child->status != SLEEP) {
                os->yield(); // 切换到其他可执行任务，防止忙等
            }
            // 获取子进程的退出状态
            *status = child->exit_status;

            // 清理子进程资源
            kmt_teardown(child);

            return 0; // 成功等待子进程并获取退出状态
        }
    }

    return -1; // 没有直接子进程可等待
}

int exit(task_t *task, int status) {
    // 设置当前进程的状态为 SLEEP
    task->status = SLEEP;
    // 设置当前进程的退出状态
    task->exit_status = status;

    // 释放当前进程的资源
    // TODO: 根据您的代码结构和资源管理方式进行资源释放操作

    // 切换到下一个可执行的任务
    os->yield();

    // 此处的代码不会执行，因为已经切换到了其他任务
    return 0;
}

int kill(task_t *task, int pid) {
    // 遍历所有进程，找到进程号为 pid 的进程
    for (int i = 0; i < MAX_CPU; i++) {
        if (cpu_task[i].current->pid == pid) {
            // 设置进程状态为 SLEEP
            cpu_task[i].current->status = SLEEP;

            // 切换到下一个可执行的任务
            os->yield();
            return 0;
        }
    }

    // 如果没有找到进程号为 pid 的进程，返回错误码
    return -1;
}



MODULE(uproc) {
  void (*init)();
  int (*kputc)(task_t *task, char ch);
  int (*fork)(task_t *task);
  int (*wait)(task_t *task, int *status);
  int (*exit)(task_t *task, int status);
  int (*kill)(task_t *task, int pid);
  void *(*mmap)(task_t *task, void *addr, int length, int prot, int flags);
  int (*getpid)(task_t *task);
  int (*sleep)(task_t *task, int seconds);
  int64_t (*uptime)(task_t *task);
};