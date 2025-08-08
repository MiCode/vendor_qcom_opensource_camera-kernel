// MIUI ADD: Camera_CameraOpt
#include "cam_pid.h"
#include "cam_msger_common.h"

#define WRITE_ERR_CMD_LIENE_INVALID  -3001
#define WRITE_ERR_FORMAT_INVALID     -3002
#define DEFAULT_VALUE_TGID_NOT_FOUND -3003
#define WRITE_ERR_KERL_ALLOC_FAILE   -3004
#define WRITE_PID_NOT_FOUND          -3005

int get_tgid_by_cmdline(struct file *fp, char *buff, int length, u8* hit){
    //struct task_struct *task = NULL;

    int  idx = 0;
    int  token_len = 0;
    int  pid = DEFAULT_VALUE_TGID_NOT_FOUND;
    char *rest    = buff;
    char *token   = NULL;
    if(buff == NULL || length <= 4) return WRITE_ERR_CMD_LIENE_INVALID;

    *hit = 0;
    switch_disable_and_exec(PID_ENABLE) return -1;
    if(strncmp(buff, "ctp", 3) == 0) *hit = 1;
    if(!(*hit)) return -1;

    if(strstr(buff, SPLIT_DELIM) == NULL)  return WRITE_ERR_FORMAT_INVALID;

    while((token = strsep(&rest, SPLIT_DELIM)) != NULL){
       if(idx > 1) break;
       if(idx == 1){
           token_len = strnlen(token, CMDLINE_MAX_LEN)-1;
           if(token_len <= 0) {
               pid = WRITE_ERR_CMD_LIENE_INVALID;
               return pid;
           }
           if((pid = find_pid_cmdline(token)) < 0) {
              pid = WRITE_PID_NOT_FOUND;
              break;
           }
       }
       idx++;
    }

    return pid;
}
// END Camera_CameraOpt
