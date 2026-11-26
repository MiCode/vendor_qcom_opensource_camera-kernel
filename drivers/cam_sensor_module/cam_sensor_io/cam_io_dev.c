#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>

#include "cam_io_dev.h"
#include "cam_sensor_io.h"
#include "cam_sensor_dev.h"
#include "cam_cci_dev.h"
#include "cam_sensor_cmn_header.h"

#define CAMIO_DEV_NAME "camiodev"
#define CAMIO_IOC_MAGIC 'V'

/* 定义 ioctl 命令 */
#define CAMIO_IOC_READ_REG       _IOWR(CAMIO_IOC_MAGIC, BASE_VIDIOC_PRIVATE+1, cam_reg_info)
#define CAMIO_IOC_WRITE_REG      _IOW(CAMIO_IOC_MAGIC,  BASE_VIDIOC_PRIVATE+2, cam_reg_info)
#define CAMIO_IOC_READ_REG_SLOT  _IOWR(CAMIO_IOC_MAGIC, BASE_VIDIOC_PRIVATE+3, cam_slot_reg_info)
#define CAMIO_IOC_WRITE_REG_SLOT _IOW(CAMIO_IOC_MAGIC,  BASE_VIDIOC_PRIVATE+4, cam_slot_reg_info)

static struct class *camio_class;
static unsigned int camio_major;
static dev_t camio_devt;
static struct mutex camio_mutex;

// 根据设备ID获取对应的I2C客户端
static const char *qupv3_se_i2c_addr[] = {
    [0] = "i2c@a80000",   // qupv3_se0_i2c
    [1] = "i2c@a84000",   // qupv3_se1_i2c
    [2] = "i2c@a88000",   // qupv3_se2_i2c
    [3] = "i2c@a8c000",   // qupv3_se3_i2c
    [4] = "i2c@a90000",   // qupv3_se4_i2c
    [5] = "i2c@a94000",   // qupv3_se5_i2c
    [6] = "i2c@a98000",   // qupv3_se6_i2c
    [8] = "i2c@880000",   // qupv3_se8_i2c
    [9] = "i2c@884000",   // qupv3_se9_i2c
    [10] = "i2c@888000",  // qupv3_se10_i2c
    [11] = "i2c@88c000",  // qupv3_se11_i2c
    [12] = "i2c@890000",  // qupv3_se12_i2c
    [13] = "i2c@198000",  // qupv3_se13_i2c
    [14] = "i2c@1984000", // qupv3_se14_i2c
    [15] = "i2c@1988000", // qupv3_se15_i2c
    [16] = "i2c@198c000", // qupv3_se16_i2c
    [17] = "i2c@1990000", // qupv3_se17_i2c
    [19] = "i2c@1a80000", // qupv3_se19_i2c
    [20] = "i2c@1a84000", // qupv3_se20_i2c
    [21] = "i2c@1a88000", // qupv3_se21_i2c
    [22] = "i2c@1a8c000", // qupv3_se22_i2c
    [23] = "i2c@1a90000", // qupv3_se23_i2c
};
#define MAX_DEVICE_ID 23

/* 文件操作结构体 */
static int camio_open(struct inode *inode, struct file *file)
{
    CAM_DBG(CAM_SENSOR, "camiodev: device opened\n");
    return 0;
}
static int camio_release(struct inode *inode, struct file *file)
{
    CAM_DBG(CAM_SENSOR, "camiodev: device closed\n");
    return 0;
}

static int handle_read_reg(unsigned long arg)
{
    cam_reg_info reg_info;
    int ret = 0;
    int32_t rc;
    struct camera_io_master cam_io_master;
    struct cam_sensor_cci_client cci_client0;
    struct cam_sensor_qup_client qup_client0;
    struct v4l2_subdev *cci_sd = NULL;
    struct i2c_client *i2c_sd = NULL;

    /* 验证用户空间指针 */
    if (copy_from_user(&reg_info, (void __user *)arg, sizeof(reg_info))) {
        return -EFAULT;
    }
    CAM_DBG(CAM_SENSOR, "camiodev: reg_info.i2c_type %d\n",reg_info.i2c_type);
    if (reg_info.i2c_type == 1)
    {
        cci_sd = cam_cci_get_subdev(reg_info.device_id);
        if (cci_sd == NULL)
        {
            CAM_ERR(CAM_SENSOR, "camiodev: Failed to get subdev\n");
            return -ENODEV;
        }
        cci_client0.cci_subdev = cci_sd;
        cci_client0.cci_device = reg_info.device_id;
        cci_client0.i2c_freq_mode = I2C_FAST_PLUS_MODE;
        cci_client0.cci_i2c_master = reg_info.cci_master == 0 ? MASTER_0 : MASTER_1;
        cci_client0.sid = reg_info.slave_addr;
        cci_client0.cid = 0;
        cci_client0.retries = 3;
        cci_client0.id_map = 0;
        cci_client0.is_probing = false;

        cam_io_master.master_type = CCI_MASTER;
        cam_io_master.cci_client = &cci_client0;
        CAM_DBG(CAM_SENSOR, "camiodev: is cci\n");
    }else if(reg_info.i2c_type == 2)
    {
        //i2c@880000 qupv3_se8_i2c
        //i2c@884000 qupv3_se9_i2c
        if (reg_info.device_id == 8) {
            i2c_sd = cam_get_i2c_client("i2c@880000");
        } else if (reg_info.device_id == 9) {
            i2c_sd = cam_get_i2c_client("i2c@884000");
        } else {
            CAM_ERR(CAM_SENSOR, "Unsupported device_id: %d\n", reg_info.device_id);
            return -EINVAL;
        }

        if (!i2c_sd) {
            CAM_ERR(CAM_SENSOR, "Failed to get i2c client for device_id: %d\n", reg_info.device_id);
            return -ENODEV;
        }
        qup_client0.i2c_client = i2c_sd;
        CAM_DBG(CAM_SENSOR, "camiodev: is i2c\n");
        cam_io_master.master_type = I2C_MASTER;
        cam_io_master.qup_client = &qup_client0;
    }

    /* 读取寄存器操作 */
    CAM_DBG(CAM_SENSOR, "camiodev: READ_REG - i2c_type: %u, device_id: %u, slave_addr: 0x%02X, reg_addr: 0x%04X\n",
            reg_info.i2c_type, reg_info.device_id, reg_info.slave_addr, reg_info.reg_addr);

    if (cci_sd != NULL)
    {
        rc = cam_cci_i2c_read(&cci_client0, reg_info.reg_addr, &reg_info.reg_data,
            reg_info.addr_type, reg_info.data_type, false);
        if (rc < 0)
        {
            CAM_ERR(CAM_SENSOR, "camiodev: cci iic read fail\n");
            ret = -EFAULT;
        }
        CAM_DBG(CAM_SENSOR, "camiodev: cci read success\n");
    }
    if (i2c_sd != NULL)
    {
        rc = cam_qup_i2c_read(i2c_sd, reg_info.reg_addr, &reg_info.reg_data,
            reg_info.addr_type, reg_info.data_type);
        if (rc < 0)
        {
            CAM_ERR(CAM_SENSOR, "camiodev: qup iic read fail\n");
            ret = -EFAULT;
        }
        CAM_DBG(CAM_SENSOR, "camiodev: qup iic read success\n");
    }

    /* 将读取的数据返回给用户空间 */
    if (copy_to_user((void __user *)arg, &reg_info, sizeof(reg_info))) {
        ret = -EFAULT;
    }

    return ret;
}

static int handle_write_reg(unsigned long arg)
{
    cam_reg_info reg_info;
    int ret = 0;
    int32_t rc;
    struct camera_io_master cam_io_master;
    struct cam_sensor_cci_client cci_client0;
    struct cam_sensor_qup_client qup_client0;
    struct v4l2_subdev *cci_sd = NULL;
    struct i2c_client *i2c_sd = NULL;
    struct cam_sensor_i2c_reg_setting write_setting;
    struct cam_sensor_i2c_reg_array i2c_reg_array;

    /* 验证用户空间指针 */
    if (copy_from_user(&reg_info, (void __user *)arg, sizeof(reg_info))) {
        return -EFAULT;
    }
    CAM_DBG(CAM_SENSOR, "camiodev: reg_info.i2c_type %d\n",reg_info.i2c_type);
    if (reg_info.i2c_type == 1)
    {
        cci_sd = cam_cci_get_subdev(reg_info.device_id);
        if (cci_sd == NULL)
        {
            CAM_ERR(CAM_SENSOR, "camiodev: Failed to get subdev\n");
            return -ENODEV;
        }
        cci_client0.cci_subdev = cci_sd;
        cci_client0.cci_device = reg_info.device_id;
        cci_client0.i2c_freq_mode = I2C_FAST_PLUS_MODE;
        cci_client0.cci_i2c_master = reg_info.cci_master == 0 ? MASTER_0 : MASTER_1;
        cci_client0.sid = reg_info.slave_addr;
        cci_client0.cid = 0;
        cci_client0.retries = 3;
        cci_client0.id_map = 0;
        cci_client0.is_probing = false;

        cam_io_master.master_type = CCI_MASTER;
        cam_io_master.cci_client = &cci_client0;
        CAM_DBG(CAM_SENSOR, "camiodev: is cci\n");
    }else if(reg_info.i2c_type == 2)
    {
        if (reg_info.device_id > MAX_DEVICE_ID ||
            qupv3_se_i2c_addr[reg_info.device_id] == NULL) {
            CAM_ERR(CAM_SENSOR, "Unsupported device_id: %d\n", reg_info.device_id);
            return -EINVAL;
        }
        i2c_sd = cam_get_i2c_client(qupv3_se_i2c_addr[reg_info.device_id]);
        if (!i2c_sd) {
            CAM_ERR(CAM_SENSOR, "Failed to get i2c client for device_id: %d\n", reg_info.device_id);
            return -ENODEV;
        }

        qup_client0.i2c_client = i2c_sd;
        CAM_DBG(CAM_SENSOR, "camiodev: is i2c\n");
        cam_io_master.master_type = I2C_MASTER;
        cam_io_master.qup_client = &qup_client0;
    }

    /* 写入寄存器操作 */
    CAM_DBG(CAM_SENSOR, "camiodev: WRITE_REG - i2c_type: %u, device_id: %u, slave_addr: 0x%02X, reg_addr: 0x%04X, reg_data: 0x%04X\n",
            reg_info.i2c_type, reg_info.device_id, reg_info.slave_addr,
            reg_info.reg_addr, reg_info.reg_data);

    i2c_reg_array.reg_addr = reg_info.reg_addr;
    i2c_reg_array.reg_data = reg_info.reg_data;
    i2c_reg_array.delay = 0;
    i2c_reg_array.data_mask = 0;

    write_setting.reg_setting = &i2c_reg_array;
    write_setting.size = 1;
    write_setting.addr_type = reg_info.addr_type;
    write_setting.data_type = reg_info.data_type;

    if (cci_sd != NULL)
    {
        rc = cam_cci_i2c_write_table(&cam_io_master, &write_setting);
        if (rc < 0)
        {
            CAM_ERR(CAM_SENSOR, "camiodev: cci iic write fail\n");
            ret = -EFAULT;
        }
        CAM_DBG(CAM_SENSOR, "camiodev: cci write success\n");
    }
    if (i2c_sd != NULL)
    {
        rc = cam_qup_i2c_write_table(&cam_io_master, &write_setting);
        if (rc < 0)
        {
            CAM_ERR(CAM_SENSOR, "camiodev: qup iic read fail\n");
            ret = -EFAULT;
        }
        CAM_DBG(CAM_SENSOR, "camiodev: qup iic read success\n");
    }

    return ret;
}

static int handle_read_reg_slot(unsigned long arg)
{
    int rc = 0;
    uint32_t slave_addr = 0;
    cam_slot_reg_info slot_reg_info;
    struct camera_io_master *cam_io_master = NULL;

    /* 验证用户空间指针 */
    if (copy_from_user(&slot_reg_info, (void __user *)arg, sizeof(cam_slot_reg_info))) {
        return -EFAULT;
    }
    cam_io_master = get_camera_io_master(slot_reg_info.slot_id);
    switch (cam_io_master->master_type) {

    case I2C_MASTER:
        if (!cam_io_master->qup_client) {
            CAM_ERR(CAM_SENSOR, "Invalid Args: qup_client: NULL");
            return -EINVAL;
        }
        slave_addr = cam_io_master->qup_client->i2c_client->addr;
        cam_io_master->qup_client->i2c_client->addr =
            (slot_reg_info.slave_addr == 0) ? slave_addr : slot_reg_info.slave_addr;
        rc = camera_io_dev_read(cam_io_master, slot_reg_info.reg_addr, &slot_reg_info.reg_data, slot_reg_info.addr_type, slot_reg_info.data_type, false);
        cam_io_master->qup_client->i2c_client->addr = slave_addr;
        break;
    case CCI_MASTER:
        if (!cam_io_master->cci_client) {
            CAM_ERR(CAM_SENSOR, "Invalid Args: cci_client: NULL");
            return -EINVAL;
        }
        slave_addr = cam_io_master->cci_client->sid;
        cam_io_master->cci_client->sid =
            (slot_reg_info.slave_addr == 0) ? slave_addr : (slot_reg_info.slave_addr >> 1);
        rc = camera_io_dev_read(cam_io_master, slot_reg_info.reg_addr, &slot_reg_info.reg_data, slot_reg_info.addr_type, slot_reg_info.data_type, false);
        cam_io_master->cci_client->sid = slave_addr;
        break;
    default:
        CAM_ERR(CAM_SENSOR, "Invalid Master Type: %d", cam_io_master->master_type);
    }

    if (rc < 0) {
        CAM_ERR(CAM_SENSOR,"Failed: random read I2C settings: %d", rc);
        return rc;
    }

    /* 将读取的数据返回给用户空间 */
    if (copy_to_user((void __user *)arg, &slot_reg_info, sizeof(slot_reg_info))) {
        rc = -EFAULT;
    }

    return rc;
}

static int handle_write_reg_slot(unsigned long arg)
{
    int rc = 0;
    cam_slot_reg_info slot_reg_info;
    struct camera_io_master *cam_io_master = NULL;
    uint32_t slave_addr = 0;
    struct cam_sensor_i2c_reg_setting write_setting;
    struct cam_sensor_i2c_reg_array i2c_reg_array;
    /* 验证用户空间指针 */
    if (copy_from_user(&slot_reg_info, (void __user *)arg, sizeof(cam_slot_reg_info))) {
        return -EFAULT;
    }
    cam_io_master = get_camera_io_master(slot_reg_info.slot_id);
    if (cam_io_master == NULL)
    {
        CAM_ERR(CAM_SENSOR, "cam_io_master is null");
        return rc;
    }

    i2c_reg_array.reg_addr = slot_reg_info.reg_addr;
    i2c_reg_array.reg_data = slot_reg_info.reg_data;
    i2c_reg_array.delay = 0;
    i2c_reg_array.data_mask = 0;

    write_setting.reg_setting = &i2c_reg_array;
    write_setting.size = 1;
    write_setting.addr_type = slot_reg_info.addr_type;
    write_setting.data_type = slot_reg_info.data_type;

    switch (cam_io_master->master_type) {

    case I2C_MASTER:
        if (!cam_io_master->qup_client) {
            CAM_ERR(CAM_SENSOR, "Invalid Args: qup_client: NULL");
            return -EINVAL;
        }
        slave_addr = cam_io_master->qup_client->i2c_client->addr;
        cam_io_master->qup_client->i2c_client->addr =
            (slot_reg_info.slave_addr == 0) ? slave_addr : slot_reg_info.slave_addr;
        rc = camera_io_dev_write(cam_io_master, &write_setting);
        cam_io_master->qup_client->i2c_client->addr = slave_addr;
        break;
    case CCI_MASTER:
        if (!cam_io_master->cci_client) {
            CAM_ERR(CAM_SENSOR, "Invalid Args: cci_client: NULL");
            return -EINVAL;
        }
        slave_addr = cam_io_master->cci_client->sid;
        cam_io_master->cci_client->sid =
            (slot_reg_info.slave_addr == 0) ? slave_addr : (slot_reg_info.slave_addr >> 1);
        rc = camera_io_dev_write(cam_io_master, &write_setting);
        cam_io_master->cci_client->sid = slave_addr;
        break;
    default:
        CAM_ERR(CAM_SENSOR, "Invalid Master Type: %d", cam_io_master->master_type);
    }

    if (rc < 0) {
        CAM_ERR(CAM_SENSOR,"Failed: random write I2C settings: %d", rc);
    }

    return rc;
}

static long camio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    /* 检查命令的有效性 */
    if (_IOC_TYPE(cmd) != CAMIO_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > (BASE_VIDIOC_PRIVATE+4)) return -ENOTTY;
    if (_IOC_NR(cmd) < (BASE_VIDIOC_PRIVATE+1)) return -ENOTTY;

    mutex_lock(&camio_mutex);
    CAM_DBG(CAM_SENSOR, "camiodev: device ioctl\n");
    switch (cmd) {
    case CAMIO_IOC_READ_REG:
        ret = handle_read_reg(arg);
        break;
    case CAMIO_IOC_WRITE_REG:
        ret = handle_write_reg(arg);
        break;
    case CAMIO_IOC_READ_REG_SLOT:
        ret = handle_read_reg_slot(arg);
        break;
    case CAMIO_IOC_WRITE_REG_SLOT:
        ret = handle_write_reg_slot(arg);
        break;
    default:
        ret = -ENOTTY;
        break;
    }
    mutex_unlock(&camio_mutex);
    return ret;
}

static const struct file_operations camio_fops = {
    .owner = THIS_MODULE,
    .open = camio_open,
    .release = camio_release,
    .unlocked_ioctl = camio_ioctl,
};

int cam_io_dev_init(void)
{
    struct device *device;
    int ret = 0;
    /* 注册字符设备 */
    camio_major = register_chrdev(0, CAMIO_DEV_NAME, &camio_fops);
    if (camio_major < 0) {
        CAM_ERR(CAM_SENSOR, "camiodev: Failed to register chrdev\n");
        return camio_major;
    }
    /* 创建设备类 */
    camio_class = class_create(CAMIO_DEV_NAME);
    if (IS_ERR(camio_class)) {
        ret = PTR_ERR(camio_class);
        CAM_ERR(CAM_SENSOR, "camiodev: Failed to create class\n");
        goto err_class;
    }
    /* 创建设备节点 */
    camio_devt = MKDEV(camio_major, 0);
    device = device_create(camio_class, NULL, camio_devt, NULL, CAMIO_DEV_NAME);
    if (IS_ERR(device)) {
        ret = PTR_ERR(device);
        CAM_ERR(CAM_SENSOR, "camiodev: Failed to create device\n");
        goto err_device;
    }

    /* 初始化互斥锁 */
    mutex_init(&camio_mutex);
    CAM_DBG(CAM_SENSOR, "camiodev: Initialized successfully (major: %d)\n", camio_major);
    return 0;
err_device:
    class_destroy(camio_class);
err_class:
    unregister_chrdev(camio_major, CAMIO_DEV_NAME);
    return ret;
}
void cam_io_dev_exit(void)
{
    device_destroy(camio_class, camio_devt);
    class_destroy(camio_class);
    unregister_chrdev(camio_major, CAMIO_DEV_NAME);
    CAM_DBG(CAM_SENSOR, "camiodev: Unloaded successfully\n");
}
