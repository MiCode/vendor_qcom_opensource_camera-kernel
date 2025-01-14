// MIUI ADD: Camera_CameraOpt
#include "cam_msger_common.h"
#include <linux/string_helpers.h>
#include <linux/pid.h>

#define PID_ARRAY_SIZE 10

int find_pid_cmdline(char *cmdline) {
    struct task_struct *task = NULL;

    int ret_pid = -1;
    int idx = 0;
    int cmdline_len = 0;
    int pids[PID_ARRAY_SIZE] = {-1};
    char *tmp_cmdline = NULL;

    if(cmdline == NULL) return ret_pid;

    cmdline_len = strnlen(cmdline, CMDLINE_MAX_LEN)-1;
    if(cmdline_len <= 0) return ret_pid;

    m_pr(LTYPE_COC, "[%s] cmdline:%s cmdline_len:%d\n", __func__, cmdline, cmdline_len);
    if(tasklist_empty()) return ret_pid;
    rcu_read_lock();
    for_each_process(task){
       if(task == NULL || task->tasks.next == NULL || list_empty(&task->tasks)) break;
       if(task->pid != task->tgid) continue;
       if(cmdline_len < TASK_COMM_LEN && strnlen(task->comm, TASK_COMM_LEN) > cmdline_len) continue;

       if(idx >= PID_ARRAY_SIZE) break;

       if(strstr(cmdline, task->comm) != NULL){
          pids[idx++] = task->tgid;
          m_pr(LTYPE_COC, "[%s] cmdline:%s idx:%d pid:%d\n", __func__, task->comm, (idx-1), pids[idx-1]);
       }
    }
    rcu_read_unlock();
    m_pr(LTYPE_COC, "[%s] cmdline_len:%d idx:%d\n", __func__, cmdline_len, idx);
    for(idx = 0; idx < PID_ARRAY_SIZE; idx++){
        if(pids[idx] > 0){
            task = get_pid_task(find_vpid(pids[idx]), PIDTYPE_PID);
            if(!task) continue;
            tmp_cmdline = kstrdup_quotable_cmdline(task, GFP_KERNEL);
            m_pr(LTYPE_COC, "[%s] tm_cmdline:%s cmdline:%s camdline_len:%d strncmp:%d\n", __func__, tmp_cmdline, cmdline, cmdline_len, strncmp(tmp_cmdline, cmdline, cmdline_len));
            if(strncmp(tmp_cmdline, cmdline, cmdline_len) == 0){
                ret_pid = task->tgid;
                kfree(tmp_cmdline);
                put_task_struct(task);
                break;
            }
            kfree(tmp_cmdline);
            put_task_struct(task);
         }
    }

    return ret_pid;
}
// END Camera_CameraOpt
