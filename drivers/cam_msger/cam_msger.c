// MIUI ADD: Camera_CameraOpt
#include "cam_msger.h"
#include "cam_sched.h"
#include "cam_pid.h"

#define CREATE_READER_ERROR_EXIST -1002
#define CREATE_READER_ERROR_NOSPACE -1003
#define CREATE_READER_ERROR_NAME_TOO_LONG -1004

#define EXIT_SIGNALE_ERROR_READER_NOT_FOUND -1005

#define RELEASE_ERROR_FILE_ADDR_INVALID -1006
#define RELEASE_ERROR_READER_NOT_FOUND -1007

#define WRITE_ERROR_UNSPORT_MSG_TYPE -1008
#define WRITE_ERROR_BUF_ADDR_INVALID -1009
#define WRITE_ERROR_FORMAT_INVALID -1010
#define WRITE_ERROR_NO_RECEVER -1011
#define WRITE_ERROR_NO_MSG_SPACE -1012
#define WRITE_ERROR_LEN_INVALID -1013
#define WRITE_ERROR_CPY_FROM_USER -1014

#define READED_ERROR_NO_REG -1015
#define READER_ERROR_EMPTY -1016
#define READER_ERROR_COPY_TO_USER_FAILED -1017

#define WRITE_ERROR_NO_PERMISSION -1018

#define NORMAL_APP_MIN_UID 10000

#include <linux/smp.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/uidgid.h>

unsigned int msger_siwtches = COC_ENABLE;
static cammsger_dev c_msger_dev;
static submodule submodules[] = {
    //{init, write, read, pool, ioctl, release}
    {cam_sched_init, cam_sched_w_pids, NULL, NULL, cam_sched_ioctl, NULL},//cam_sched.h
    {NULL, get_tgid_by_cmdline, NULL, NULL, NULL, NULL}//cam_pid.h
};


u8 debug(int log_type){
    return (log_type == c_msger_dev.debug);
}

static void dump(void){
    int   j = 0;
    int idx = 0;
    mutex_lock(&c_msger_dev.g_lock);
    printk(KERN_INFO "Reader count%d debug:%d\n", c_msger_dev.r_entity_count, c_msger_dev.debug);
    for(idx = 0; idx < READER_COUNT_MAX; idx++){
        if(!c_msger_dev.readers[idx].used) continue;
        printk(KERN_INFO "Reader %s:\n", c_msger_dev.readers[idx].name);
        printk(KERN_INFO "   used:%d pid:%d tid:%d fp:%p name:%s:\n"
            , c_msger_dev.readers[idx].used, c_msger_dev.readers[idx].tgid
            , c_msger_dev.readers[idx].pid , c_msger_dev.readers[idx].fp
            , c_msger_dev.readers[idx].name);
        printk(KERN_INFO "    front:%d rear:%d Messages:\n"
             , c_msger_dev.readers[idx].front
             , c_msger_dev.readers[idx].rear);
        printk(KERN_INFO "Message List:\n");
        for(j = 0; j < MSG_COUNT_MAX; j++){
            printk(KERN_INFO "    %d:from:%s to:%s [%s]\n", j, c_msger_dev.readers[idx].msgs[j].from
               , c_msger_dev.readers[idx].msgs[j].to
               , c_msger_dev.readers[idx].msgs[j].data);
        }
    }
    mutex_unlock(&c_msger_dev.g_lock);
}

static void clear_r_entity(int idx){
    if(idx < 0 || idx >= READER_COUNT_MAX)return;

    mutex_lock(&c_msger_dev.g_lock);
    c_msger_dev.readers[idx].pid   = 0;
    c_msger_dev.readers[idx].tgid  = 0;
    c_msger_dev.readers[idx].used  = 0;
    c_msger_dev.readers[idx].rear  = 0;
    c_msger_dev.readers[idx].front = 0;
    c_msger_dev.readers[idx].fp = NULL;
    memset(&(c_msger_dev.readers[idx].name), 0, sizeof(char) * READER_ID_MAX);
    memset(&(c_msger_dev.readers[idx].msgs), 0, sizeof(m_item) * MSG_COUNT_MAX);
    c_msger_dev.r_entity_count--;
    mutex_unlock(&c_msger_dev.g_lock);

    wake_up_interruptible(&(c_msger_dev.readers[idx].wait_queue));
}
static int find_reader_by_fp(struct file *filp){
    int idx = 0;
    if(filp == NULL) return idx;
    for(idx = 0; idx < READER_COUNT_MAX; idx++){
        if(!c_msger_dev.readers[idx].used) continue;
        if(c_msger_dev.readers[idx].fp == filp) return idx;
    }
    return idx;
}
static int find_reader_by_name_or_tgid(char* name, int tgid) {
    int idx = 0;
    for(idx = 0; idx < READER_COUNT_MAX; idx++){
        if(!c_msger_dev.readers[idx].used) continue;
        if(name != NULL && strcmp(name, c_msger_dev.readers[idx].name) == 0) return idx;
        if(tgid > 0 && c_msger_dev.readers[idx].tgid == tgid) return idx;
    }
    return idx;
}

static int find_new_reader_idx(void){
    int idx = 0;
    m_pr(LTYPE_COC, "find_new_reader_idx \n");
    for(idx = 0; idx < READER_COUNT_MAX; idx++){
        if(!c_msger_dev.readers[idx].used) return idx;
    }
    return idx;
}

static int cameramsger_open(struct inode *inode, struct file *filp) {
    m_pr(LTYPE_COC, "cameramsger_open, thread:%d file:%p\n", current->pid, filp);
    return 0;
}
static unsigned int cameramsger_poll(struct file *file, struct poll_table_struct *poll_table)
{
    unsigned int mask = 0;
    u8  hit = 0;
    int idx = 0, len_submodules = 0;

    switch_enable_and_exec(COC_ENABLE){
        m_pr(LTYPE_COC, "cameramsger_poll pid:%d tid:%d file:%p\n", current->tgid, current->pid, file);
        mutex_lock(&c_msger_dev.g_lock);
        for(idx = 0; idx < READER_COUNT_MAX; idx++){
            if(!c_msger_dev.readers[idx].used) continue;
            if(c_msger_dev.readers[idx].fp != file) continue;
            if(current->tgid != c_msger_dev.readers[idx].tgid) continue;
            break;
        }
        mutex_unlock(&c_msger_dev.g_lock);

        if(idx < READER_COUNT_MAX){//found reader
            if(c_msger_dev.readers[idx].front != c_msger_dev.readers[idx].rear) { //have messages
               mask = POLLIN | POLLRDNORM;
               m_pr(LTYPE_COC, "cameramsger_poll tid:%d have messages.\n", current->pid);
               return mask;
            }
            m_pr(LTYPE_COC, "cameramsger_poll poll_wait_begin.\n");
            poll_wait(file, &(c_msger_dev.readers[idx].wait_queue), poll_table);
            m_pr(LTYPE_COC, "cameramsger_poll poll_wait_end.\n");
            if(c_msger_dev.readers[idx].front == c_msger_dev.readers[idx].rear) {
                mask = 0;
                return mask;//message array empty
            }
            mask = POLLIN | POLLRDNORM;
            m_pr(LTYPE_COC, "cameramsger_poll reader pid:%d tid:%d fp:%p have messages.\n", current->pid, current->tgid, file);
            return mask;
        }
    }
    //check submodules
    len_submodules = ARRAY_SIZE(submodules);
    for(idx = 0; idx < len_submodules; idx++){
        if(submodules[idx].poll != NULL){
            mask = submodules[idx].poll(file, poll_table, &hit);
            if(hit) return mask;
        }
    }
    return mask;
}

static ssize_t cameramsger_read(struct file *fp, char __user *buff,  size_t length, loff_t *ppos) {
    u8  hit = 0;
    int idx = 0;
    int len = 0, len_submodules = 0;
    int ret = READER_ERROR_EMPTY;
    r_entity* reader = NULL;

    switch_enable_and_exec(COC_ENABLE){
        mutex_lock(&c_msger_dev.g_lock);
        for(idx = 0; idx < READER_COUNT_MAX; idx++){
            reader = &(c_msger_dev.readers[idx]);
            if(!reader->used) continue;
            if(reader->fp != fp) continue;
            if(current->tgid != reader->tgid) continue;
            break;
        }
        if(idx < READER_COUNT_MAX) {
            if(reader->front == reader->rear){
               ret = READER_ERROR_EMPTY;
               goto _func_done;//message array empty
            }

            m_pr(LTYPE_COC, "cameramsger_read find message curr-tid:%d c_msger_dev.readers[idx]_info pid:%d file:%p.\n"
                            , current->pid, reader->tgid, reader->fp);

            len = (length < MSG_LEN_MAX)? length : MSG_LEN_MAX;
            ret = copy_to_user(buff, &(reader->msgs[reader->front].data), len);

            if(ret == 0) {
               ret = strnlen(&(reader->msgs[reader->front].data[0]), len);
            } else if(ret > 0){
               ret = READER_ERROR_COPY_TO_USER_FAILED;
            }

            m_pr(LTYPE_COC, "cameramsger_read message(msg-idx:%d) read finish, mssage:%s, len:%d, ret:%d, message_len:%lu\n"
                 , reader->front, &(reader->msgs[reader->front]).data[0]
                 , len, ret, strnlen(&(reader->msgs[reader->front].data[0]), len));

            reader->front = (reader->front + 1) % MSG_COUNT_MAX;
        _func_done:
            mutex_unlock(&c_msger_dev.g_lock);
            return ret;
        }
    }
    //check submodules
    len_submodules = ARRAY_SIZE(submodules);
    for(idx = 0; idx < len_submodules; idx++){
        if(submodules[idx].read != NULL){
            ret = submodules[idx].read(fp, buff, length, &hit);
            if(hit) return ret;
        }
    }
    return ret;
}

static int switch_update(struct file *fp, char *buff, int length){
    int  idx  = 0;
    int v_swh = 0;
    char *token = NULL;
    char *rest  = buff;

    if(buff[3] != '|') return WRITE_ERROR_FORMAT_INVALID;

    while((token = strsep(&rest, SPLIT_DELIM)) != NULL){
        if(idx > 1)break;
        if(idx == 1 && !kstrtoint(token, 10, &v_swh)){
            m_pr(LTYPE_COC, "%s v_swh:%d msger_siwtches_0x%x\n", __func__, v_swh, msger_siwtches);
            msger_siwtches |= v_swh;
            msger_siwtches &= v_swh;
            m_pr(LTYPE_COC, "%s v_swh:%d v_swh_0x%x msger_siwtches_0x%x\n", __func__, v_swh, v_swh, msger_siwtches);
        }
        idx++;
    }
    return length;
}
/*Acquires c_msger_dev.g_lock before invoking reg_reader*/
static int reg_reader(struct file *fp, char *buff, int length){
    int reader_idx = -1;
    r_entity* reader = NULL;

    m_pr(LTYPE_COC, "%s\n", __func__);

    if(length < 5) return WRITE_ERROR_LEN_INVALID;
    if(buff[3] != '|') return WRITE_ERROR_FORMAT_INVALID;
    if(strlen(&buff[4]) < 1) return WRITE_ERROR_FORMAT_INVALID;
    if(strlen(&buff[4]) >= READER_ID_MAX) return CREATE_READER_ERROR_NAME_TOO_LONG;
    m_pr(LTYPE_COC, "%s data:%s reader_name:%s\n", __func__, buff, &buff[4]);

    //check exist
    reader_idx = find_reader_by_name_or_tgid(&buff[4], current->tgid);
    m_pr(LTYPE_COC, "%s reader_already_registed reader_idx:%d\n", __func__, reader_idx);
    if(reader_idx && reader_idx < READER_COUNT_MAX){
        return CREATE_READER_ERROR_EXIST;
    }

    //check have space
    reader_idx = find_new_reader_idx();
    if(reader_idx && reader_idx >= READER_COUNT_MAX){
        return CREATE_READER_ERROR_NOSPACE;
    }
    //add reader
    reader = &(c_msger_dev.readers[reader_idx]);
    reader->fp   = fp;
    reader->used =  1;
    reader->pid  =  current->pid;
    reader->tgid = current->tgid;
    c_msger_dev.r_entity_count ++;
    snprintf(reader->name, READER_ID_MAX, "%s", &buff[4]);

    m_pr(LTYPE_COC, "reader reg success! tid:%d, pid:%d, name:%s, fp:%p \n"
        , reader->pid, reader->tgid, reader->name, reader->fp);
    return length;
}

/*Acquires c_msger_dev.g_lock before invoking add_message*/
static int add_message(int reader_idx, m_item* item){
    int  rear = 0;
    r_entity* reader = NULL;

    if(reader_idx < 0 || reader_idx >= READER_COUNT_MAX) return WRITE_ERROR_NO_MSG_SPACE;
    reader = &(c_msger_dev.readers[reader_idx]);

    rear = (reader->rear + 1) % MSG_COUNT_MAX;
    if(rear == reader->front){
        reader->front = (reader->front + 1) % MSG_COUNT_MAX;
    }
    memcpy(&(reader->msgs[reader->rear]), item, sizeof(m_item));
    reader->rear = rear;

    wake_up_interruptible(&(reader->wait_queue));

    m_pr(LTYPE_COC, "%s add message to %d success\n", __func__, reader->pid);
    return 0;
}

/*Acquires c_msger_dev.g_lock before invoking snd_message*/
static int snd_message(char *buff, int length){
    int i = 0;
    int j = 0;
    int reader_idx = -1;
    int at_pos[3]  = {-1, -1, -1};//| position

    m_item item;
    char* msg = NULL;
    char  sender[READER_ID_MAX];
    char recever[READER_ID_MAX];
    memset(sender, 0, READER_ID_MAX);
    memset(recever,0, READER_ID_MAX);
    memset(&item,  0, sizeof(m_item));

    if(length <= 7) return WRITE_ERROR_FORMAT_INVALID;
    for(i = 0; i< length; i++){
        if(buff[i] == '|') at_pos[j++] = i;
        if(j > 2) break;
    }
    m_pr(LTYPE_COC, "%s at position:%d - %d - %d\n", __func__, at_pos[0], at_pos[1], at_pos[2]);
    if(at_pos[1] - at_pos[0] <= 1 || at_pos[2] - at_pos[1] <= 1) return WRITE_ERROR_FORMAT_INVALID;

    i = 0;
    for(j = at_pos[0] + 1; j < at_pos[1]; j++){
        recever[i++] = buff[j];
    }

    i = 0;
    for(j = at_pos[1] + 1; j < at_pos[2]; j++){
        sender[i++] = buff[j];
    }

    msg = (char*)&(buff[at_pos[1]+1]);
    m_pr(LTYPE_COC, "%s sender:%s recever:%s msg:%s\n", __func__, sender, recever, msg);

    reader_idx = find_reader_by_name_or_tgid(recever, -1);
    if(reader_idx && reader_idx >= READER_COUNT_MAX) return WRITE_ERROR_NO_RECEVER;

    memcpy(item.from, sender,  READER_ID_MAX);
    memcpy(item.to,   recever, READER_ID_MAX);
    memcpy(item.data, msg,     length - (at_pos[1]+1));
    if(add_message(reader_idx, &item) == 0) return length;

    return 0;
}

static ssize_t cameramsger_write(struct file *fp, const char *buff,  size_t length, loff_t *ppos){
    u8  hit = 0;
    int cpy_len = -1, len_submodules = 0;
    int idx = -1;
    int retv = WRITE_ERROR_UNSPORT_MSG_TYPE;

    m_pr(LTYPE_COC, "%s len:%zu", __func__, length);
    if(length <= 4)  return WRITE_ERROR_LEN_INVALID;
    if(buff == NULL) return WRITE_ERROR_BUF_ADDR_INVALID;

    m_pr(LTYPE_COC, "%s current thread tid:%d uid:%d\n", __func__, current->pid, from_kuid(current_user_ns(), current_uid()));

    if(from_kuid(current_user_ns(), current_uid()) >= NORMAL_APP_MIN_UID){
        return WRITE_ERROR_NO_PERMISSION;
    }

    mutex_lock(&c_msger_dev.g_lock);
    memset(&(c_msger_dev.d_cache), 0, sizeof(char) * MSG_LEN_MAX);
    cpy_len = copy_from_user(&(c_msger_dev.d_cache[0]), buff, length);

    m_pr(LTYPE_COC, "%s: copy after:%s len:%zu cpy_len:%d",__func__, &(c_msger_dev.d_cache[0]), length, cpy_len);

    if(strncmp(&(c_msger_dev.d_cache[0]), "swh", 3) == 0){
        retv = switch_update(fp, &(c_msger_dev.d_cache[0]), length);
        goto __func_done;
    }

    switch_enable_and_exec(COC_ENABLE){
        if(strncmp(&(c_msger_dev.d_cache[0]), "reg", 3) == 0){
            retv = reg_reader(fp, &(c_msger_dev.d_cache[0]), length);
            goto __func_done;
        }
        if(strncmp(&(c_msger_dev.d_cache[0]), "snd", 3) == 0){
            retv = snd_message(&(c_msger_dev.d_cache[0]), length);
            goto __func_done;
        }
    }
    //check submodules
    len_submodules = ARRAY_SIZE(submodules);
    for(idx = 0; idx < len_submodules; idx++){
        if(submodules[idx].write != NULL){
            retv = submodules[idx].write(fp, &(c_msger_dev.d_cache[0]), length, &hit);
            m_pr(LTYPE_COC, "%s submodules[%d].write:%p hit:%d retv:%d", __func__, idx, submodules[idx].write, hit, retv);
            if(hit)break;
        }
    }

__func_done:
    mutex_unlock(&c_msger_dev.g_lock);
    return retv;
}
static int cameramsger_release(struct inode *inode, struct file *filp){
    int idx = 0;
    int rl_pid= 0;
    int reader_idx = 0, len_submodules = 0;
    bool is_rl_task_die = false;
    struct task_struct* rl_task = NULL;

    if(in_interrupt()) return 0;
    if(filp == NULL) return RELEASE_ERROR_FILE_ADDR_INVALID;

    switch_enable_and_exec(COC_ENABLE){
        mutex_lock(&c_msger_dev.g_lock);
        reader_idx = find_reader_by_fp(filp);
        if(reader_idx && reader_idx < READER_COUNT_MAX){
            m_pr(LTYPE_COC, "cameramsger_release find reader:pid:%d tid:%d fp:%p name:%s:\n",  c_msger_dev.readers[reader_idx].tgid
                , c_msger_dev.readers[reader_idx].pid
                , c_msger_dev.readers[reader_idx].fp
                , c_msger_dev.readers[reader_idx].name);

            rl_pid = c_msger_dev.readers[reader_idx].tgid;
            mutex_unlock(&c_msger_dev.g_lock);

            rl_task = get_pid_task(find_vpid(rl_pid), PIDTYPE_PID);
            if(rl_task == NULL){
              is_rl_task_die = true;
            }else{
              is_rl_task_die = !(pid_alive(rl_task));
              m_pr(LTYPE_COC, "[%s] pid_alive:%d", __func__, pid_alive(rl_task));
              if(!is_rl_task_die 
                 && (rl_task->__state == TASK_DEAD
                      || rl_task->exit_state == EXIT_DEAD
                      || rl_task->exit_state == EXIT_ZOMBIE
                      || rl_task->files == NULL)){
                 is_rl_task_die = true;
              }
              m_pr(LTYPE_COC, "[%s] other state check result: task_die:%d", __func__, is_rl_task_die);
              put_task_struct(rl_task);
            }

            mutex_lock(&c_msger_dev.g_lock);
            if(is_rl_task_die){
                for(idx = 0; idx < READER_COUNT_MAX; idx++){
                    if(!c_msger_dev.readers[idx].used || idx == reader_idx) continue; 
                    memset(&(c_msger_dev.d_cache), 0, sizeof(char) * MSG_LEN_MAX);
                    snprintf(c_msger_dev.d_cache, MSG_LEN_MAX, READER_DEATH_OMEN_TEMPLATE
                                , &(c_msger_dev.readers[idx].name[0])
                                , READER_DEATH_CAMMIND_MSG_CODE
                                , READER_DEATH_CAMMIND_MSG_CODE
                                , ktime_to_ns(ktime_get())
                                , c_msger_dev.readers[reader_idx].pid
                                , c_msger_dev.readers[reader_idx].tgid
                                , c_msger_dev.readers[reader_idx].name);

                    snd_message(&(c_msger_dev.d_cache[0]), strlen(&(c_msger_dev.d_cache[0])));
                    m_pr(LTYPE_COC, "[%s] notify pid:%d that proc-%d is death.\n" , __func__
                       , c_msger_dev.readers[idx].tgid, c_msger_dev.readers[reader_idx].tgid);
                }
            }
            mutex_unlock(&c_msger_dev.g_lock);
            clear_r_entity(reader_idx);
        }else{
            mutex_unlock(&c_msger_dev.g_lock);
        }
    }
    //check submodules
    len_submodules = ARRAY_SIZE(submodules);
    for(idx = 0; idx < len_submodules; idx++){
        if(submodules[idx].release != NULL){
            submodules[idx].release(inode, filp);
        }
    }
    return 0;
}

static long cameramsger_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg) {
    u8 hit = 0;
    int idx = 0, len_submodules = 0;
    m_pr(LTYPE_COC, "%s: %u %lu\n",__func__, cmd, arg);
    switch(cmd){
        case IOC_CMD_PID_DEBUG_ON:
            mutex_lock(&c_msger_dev.g_lock);
            c_msger_dev.debug = LTYPE_PID;
            mutex_unlock(&c_msger_dev.g_lock);
            return 0;
        case IOC_CMD_COC_DEBUG_ON:
            mutex_lock(&c_msger_dev.g_lock);
            c_msger_dev.debug = LTYPE_COC;
            mutex_unlock(&c_msger_dev.g_lock);
            return 0;
        case IOC_CMD_SCHED_DEBUG_ON:
            mutex_lock(&c_msger_dev.g_lock);
            c_msger_dev.debug = LTYPE_SCHED;
            mutex_unlock(&c_msger_dev.g_lock);
            return 0;
        case IOC_CMD_DEBUG_OFF:
            mutex_lock(&c_msger_dev.g_lock);
            c_msger_dev.debug = 0;
            mutex_unlock(&c_msger_dev.g_lock);
            return 0;
        case IOC_CMD_MSGER_SWITCHES_DUMP:
            printk(KERN_INFO "[%s] msger_siwtches:%x\n", __func__, msger_siwtches);
            return 0;
        case IOC_CMD_DUMP:
            dump();
            return 0;
    }
    //check submodules
    len_submodules = ARRAY_SIZE(submodules);
    for(idx = 0; idx < len_submodules; idx++){
        if(submodules[idx].ioctl != NULL){
            submodules[idx].ioctl(filp, cmd, arg, &hit);
            m_pr(LTYPE_COC, "%s: submodules[%d].ioctl:%p hit:%d.\n", __func__, idx, submodules[idx].ioctl, hit);
            if(hit) break;
        }
    }
    if(!hit) return -ENOTTY;
    return 0;
}

static int __init cameramsger_init(void)
{
    int idx = 0, len_submodules = 0;

    m_pr(LTYPE_COC, "mode init cameramsger_init begin\n");
    strcpy(c_msger_dev.name,    "cam_msger");
    strcpy(c_msger_dev.version, "2.0"); 

    c_msger_dev.dev_fops.owner = THIS_MODULE,
    c_msger_dev.dev_fops.open  = cameramsger_open,
    c_msger_dev.dev_fops.poll  = cameramsger_poll,
    c_msger_dev.dev_fops.read  = cameramsger_read,
    c_msger_dev.dev_fops.write = cameramsger_write,
    c_msger_dev.dev_fops.release = cameramsger_release,
    c_msger_dev.dev_fops.unlocked_ioctl = cameramsger_ioctl,

    c_msger_dev.cammsger_major =
        register_chrdev(0, c_msger_dev.name, &(c_msger_dev.dev_fops));
    c_msger_dev.cammsger_class = class_create(c_msger_dev.name);
    if (IS_ERR(c_msger_dev.cammsger_class)) {
         unregister_chrdev(c_msger_dev.cammsger_major, c_msger_dev.name);
        pr_warn("Failed to create class.\n");
        return PTR_ERR(c_msger_dev.cammsger_class);
    }

    c_msger_dev.devt   = MKDEV(c_msger_dev.cammsger_major, 0);
    c_msger_dev.device = device_create(c_msger_dev.cammsger_class, NULL, c_msger_dev.devt,
                NULL, c_msger_dev.name);

    c_msger_dev.debug = 0;
    c_msger_dev.r_entity_count = 0;
    memset(&(c_msger_dev.d_cache), 0, sizeof(char) * MSG_LEN_MAX);
    mutex_init(&c_msger_dev.g_lock);

    for(idx = 0; idx < READER_COUNT_MAX; idx++){
       c_msger_dev.readers[idx].pid   = 0;
       c_msger_dev.readers[idx].tgid  = 0;
       c_msger_dev.readers[idx].used  = 0;
       c_msger_dev.readers[idx].rear  = 0;
       c_msger_dev.readers[idx].front = 0;
       memset(&(c_msger_dev.readers[idx].name), 0, sizeof(char) * READER_ID_MAX);
       memset(&(c_msger_dev.readers[idx].msgs), 0, sizeof(m_item) * MSG_COUNT_MAX);
       init_waitqueue_head(&(c_msger_dev.readers[idx].wait_queue));
    }

    if (IS_ERR(c_msger_dev.device)) {
        pr_err("error while trying to create %s\n", c_msger_dev.name);
        return -EINVAL;
    }

    len_submodules = ARRAY_SIZE(submodules);
    for(idx = 0; idx < len_submodules; idx++){
        if(submodules[idx].init != NULL)submodules[idx].init();
    }

    m_pr(LTYPE_COC, "mode init sucesss\n");
    return 0;
}

/*static void __exit cameramsger_exit(void)
{
    device_destroy(c_msger_dev.cammsger_class, c_msger_dev.devt);
    class_destroy(c_msger_dev.cammsger_class);
    unregister_chrdev(c_msger_dev.cammsger_major, c_msger_dev.name);
    mutex_destroy(&c_msger_dev.g_lock);
    unregister_trace_android_rvh_wake_up_new_task(m_wake_up_new_task, NULL);
}*/

module_init(cameramsger_init);
//module_exit(cameramsger_exit);
MODULE_DESCRIPTION("camera messager device driver");
MODULE_LICENSE("GPL v2");
// END Camera_CameraOpt
