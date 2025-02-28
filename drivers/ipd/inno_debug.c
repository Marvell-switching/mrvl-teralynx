/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/pci.h>

#include "pci_common_ipd.h"
#include "ipd.h"
#include "inno_ring.h"
#include "inno_ioctl.h"
#include "ipd_version.h"

static struct device *ipd_dev;
static int inno_max_dev;

uint32_t ipd_loglevel = IPD_LOGLEVEL_WARN;
module_param_named(loglevel, ipd_loglevel, uint, 0644);
MODULE_PARM_DESC(loglevel, "Log level(0-7)");

ssize_t
ipd_info_r( struct device * dev,
              struct device_attribute * attr,
              char * buf ){
    inno_device_t  *idev;
    int len,i;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    len = scnprintf(buf,PAGE_SIZE,"Innovium Platform Driver Version - %s\n",ipd_version);
    len += scnprintf(buf+len,PAGE_SIZE-len,"Innovium Device Info:\n");
    for(i=0;i<inno_max_dev;i++){
        if(idev[i].vendor_id != 0) {
            len += scnprintf(buf+len,PAGE_SIZE-len,"Chip - %d\n",idev[i].instance);
            len += scnprintf(buf+len,PAGE_SIZE-len,"  VendorId    - 0x%x\n",idev[i].vendor_id);
            len += scnprintf(buf+len,PAGE_SIZE-len,"  DeviceId    - 0x%x\n",idev[i].device_id);
            len += scnprintf(buf+len,PAGE_SIZE-len,"  RevId       - 0x%x\n",idev[i].rev_id);
            len += scnprintf(buf+len,PAGE_SIZE-len,"  DeviceOpen  - %d\n",idev[i].inno_stats.inno_drv_stats.num_open);
            len += scnprintf(buf+len,PAGE_SIZE-len,"  DeviceClose - %d\n",idev[i].inno_stats.inno_drv_stats.num_close);
        }else {
            break;
        }
    }
    return len;
}


ssize_t
ring_stat_r( struct device * dev,
              struct device_attribute * attr,
              char * buf ) {
    inno_device_t  *idev;
    int len,i,j;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    len = scnprintf(buf,PAGE_SIZE,"Ring Stats\n");
    for(i=0;i<inno_max_dev;i++){
        if(idev[i].vendor_id) {
            len += scnprintf(buf+len,PAGE_SIZE-len,"Chip - %d\n",idev[i].instance);
            len += scnprintf(buf+len,PAGE_SIZE-len," Ring %20s %20s %20s %20s %20s\t%s\n",
                                                   "Descriptor","Packets","Bytes",
                                                   "Drops","Full","Type");
            for(j=0;j<NUM_TX_RINGS;j++) {
                if(idev[i].tx_ring[j].flags & INNO_RING_INIT) {
                    len += scnprintf(buf+len,PAGE_SIZE-len," TX-%d %20llu %20llu %20llu %20llu %20llu\t%s\n", j,
                                                            idev[i].inno_stats.tx_ring_stats[j].descs,
                                                            idev[i].inno_stats.tx_ring_stats[j].packets,
                                                            idev[i].inno_stats.tx_ring_stats[j].bytes,
                                                            idev[i].inno_stats.tx_ring_stats[j].drops,
                                                            idev[i].inno_stats.tx_ring_stats[j].ring_full,
                                                            (idev[i].tx_ring[j].flags & INNO_RING_NETDEV)?"KERNEL":"USER");
                }
            }
            len += scnprintf(buf+len,PAGE_SIZE-len,"\n Ring %20s %20s %20s %20s\t%s\n",
                                                   "Descriptor","Packets","Bytes",
                                                   "Drops","Type");
            for(j=0;j<NUM_RX_RINGS;j++) {
                if(idev[i].rx_ring[j].flags & INNO_RING_INIT) {
                    if(idev[i].rx_ring[j].flags & INNO_RING_NETDEV) {
                        len += scnprintf(buf+len,PAGE_SIZE-len,
                                " RX-%d %20llu %20llu %20llu %20llu\t%s\n", j,
                                idev[i].inno_stats.rx_ring_stats[j].descs,
                                idev[i].inno_stats.rx_ring_stats[j].packets,
                                idev[i].inno_stats.rx_ring_stats[j].bytes,
                                idev[i].inno_stats.rx_ring_stats[j].drops,
                                "KERNEL");
                    }else{
                        len += scnprintf(buf+len,PAGE_SIZE-len,
                                " RX-%d %20s %20s %20s %20s\t%s\n", j,
                                "NA","NA","NA","NA","USER");
                    }
                }
            }
            len += scnprintf(buf+len,PAGE_SIZE-len, "NA: Data not available in kernel space\n");
        }else {
            break;
        }
    }
    return len;
}

ssize_t
ring_stat_w( struct device * dev,
               struct device_attribute * attr,
               const char * buf,
               size_t count ){
    inno_device_t  *idev;
    int val, i;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    if(kstrtoint(buf,10,&val) < 0 ){
        ipd_err("Unexpected value");
        return -EINVAL;
    }else {
        if(val == 0) {
            for(i=0;i<inno_max_dev;i++){
                if(idev[i].vendor_id) {
                    memset(&(idev[i].inno_stats.tx_ring_stats),0,sizeof(idev[i].inno_stats.tx_ring_stats));
                    memset(&(idev[i].inno_stats.rx_ring_stats),0,sizeof(idev[i].inno_stats.rx_ring_stats));
                }
            }
        }

    }
    return count;

}

ssize_t
interrupt_stat_r( struct device * dev,
              struct device_attribute * attr,
              char * buf ){
    inno_device_t  *idev;
    int len,i,j;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    len = scnprintf(buf,PAGE_SIZE,"Interrupt Stats\n");
    for(i=0;i<inno_max_dev;i++){
        if(idev[i].vendor_id) {
            len += scnprintf(buf+len,PAGE_SIZE-len,"Chip - %d\n",idev[i].instance);
            len += scnprintf(buf+len,PAGE_SIZE-len," Count%*c\t   Name\n",15, ' ');
            for(j=0;j<idev[i].num_vectors;j++) {
                len += scnprintf(buf+len,PAGE_SIZE-len," %-20llu\t%s\n",idev[i].inno_stats.inno_rupt_stats.num_int[j], idev[i].msix_names[j]);
            }
        }else {
            break;
        }
    }
    return len;
}

ssize_t
interrupt_stat_w( struct device * dev,
               struct device_attribute * attr,
               const char * buf,
               size_t count ){
    inno_device_t  *idev;
    int val, i;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    if(kstrtoint(buf,10,&val) < 0 ){
        ipd_err("Unexpected value");
        return -EINVAL;
    }else {
        if(val == 0) {
            for(i=0;i<inno_max_dev;i++){
                if(idev[i].vendor_id) {
                    memset(&(idev[i].inno_stats.inno_rupt_stats.num_int),0,sizeof(idev[i].inno_stats.inno_rupt_stats.num_int));
                }
            }
        }
    }
    return count;
}

ssize_t
loglevel_r( struct device * dev,
              struct device_attribute * attr,
              char * buf ){
    inno_device_t  *idev;
    int len;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    len = scnprintf(buf,PAGE_SIZE,"IPD loglevel - %d\n", ipd_loglevel);
    len += scnprintf(buf+len,PAGE_SIZE,"0 - IPD_LOGLEVEL_CRIT\n");
    len += scnprintf(buf+len,PAGE_SIZE,"1 - IPD_LOGLEVEL_ERR\n");
    len += scnprintf(buf+len,PAGE_SIZE,"2 - IPD_LOGLEVEL_WARN\n");
    len += scnprintf(buf+len,PAGE_SIZE,"3 - IPD_LOGLEVEL_NOT\n");
    len += scnprintf(buf+len,PAGE_SIZE,"4 - IPD_LOGLEVEL_INFO\n");
    len += scnprintf(buf+len,PAGE_SIZE,"5 - IPD_LOGLEVEL_TRACE\n");
    len += scnprintf(buf+len,PAGE_SIZE,"6 - IPD_LOGLEVEL_DEBUG\n");
    len += scnprintf(buf+len,PAGE_SIZE,"7 - IPD_LOGLEVEL_VERBOSE\n");
    len += scnprintf(buf+len,PAGE_SIZE,"Note: DEBUG & VERBOSE are very intrusive loglevel. Please use these options sparingly.\n");
    return len;
}

ssize_t
loglevel_w( struct device * dev,
               struct device_attribute * attr,
               const char * buf,
               size_t count ){
    inno_device_t  *idev;
    int val;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    if(kstrtoint(buf,10,&val) < 0 ){
        ipd_err("Unexpected value");
        return -EINVAL;
    }else {
        switch (val){
            case 0 ... 7:
                ipd_loglevel = val;
                break;
            default:
                ipd_err("Invalid loglevel %d", val);
                return -EINVAL;
        }
    }
    return count;
}

ssize_t
tx_stat_r( struct device * dev,
            struct device_attribute * attr,
            char * buf ){
    inno_device_t  *idev;
    int len, i;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    len = scnprintf(buf,PAGE_SIZE,"TX Stats\n");
    for(i=0;i<inno_max_dev;i++){
        if(idev[i].vendor_id) {
            len += scnprintf(buf+len,PAGE_SIZE-len,"Chip - %d\n",idev[i].instance);
            len += scnprintf(buf+len,PAGE_SIZE-len," Lookup packets - %20llu\n", idev[i].inno_stats.tx_stats.tx_lookup_packets);
            len += scnprintf(buf+len,PAGE_SIZE-len," Bypass packets - %20llu\n", idev[i].inno_stats.tx_stats.tx_bypass_packets);
        }
    }
    return len;
}

ssize_t
tx_stat_w( struct device * dev,
            struct device_attribute * attr,
            const char * buf,
            size_t count ){
    inno_device_t  *idev;
    int val, i;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    if(kstrtoint(buf,10,&val) < 0 ){
        ipd_err("Unexpected value");
        return -EINVAL;
    }else {
        if(val == 0) {
            for(i=0;i<inno_max_dev;i++){
                if(idev[i].vendor_id) {
                    memset(&(idev[i].inno_stats.tx_stats),0,sizeof(idev[i].inno_stats.tx_stats));
                }
            }
        }
    }
    return count;
}

ssize_t
wb_stat_r( struct device * dev,
            struct device_attribute * attr,
            char * buf ){
    inno_device_t  *idev;
    int len, i, ring;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    len = scnprintf(buf,PAGE_SIZE,"WB Stats\n");
    for(i=0;i<inno_max_dev;i++){
        if(idev[i].vendor_id) {
            len += scnprintf(buf+len,PAGE_SIZE-len,"Chip - %d\n",idev[i].instance);
            len += scnprintf(buf+len,PAGE_SIZE-len," WB crc :       %20llu\n", idev[i].inno_stats.wb_stats.wb_crc);
            len += scnprintf(buf+len,PAGE_SIZE-len," WB err :       %20llu\n", idev[i].inno_stats.wb_stats.wb_err);
            for (ring = 0; ring < NUM_TX_RINGS; ring++) {
                len += scnprintf(buf+len,PAGE_SIZE-len," Ring %d WB tx->cidx :  %13d\n",
                        ring, idev[i].tx_ring[ring].cidx);
                len += scnprintf(buf+len,PAGE_SIZE-len," Ring %d WB cur->cidx : %13d\n",
                        ring, idev[i].tx_ring[ring].work_cidx);
            }
        }
    }
    return len;
}

ssize_t
wb_stat_w( struct device * dev,
            struct device_attribute * attr,
            const char * buf,
            size_t count ){
    inno_device_t  *idev;
    int val, i;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    if(kstrtoint(buf,10,&val) < 0 ){
        ipd_err("Unexpected value");
        return -EINVAL;
    }else {
        if(val == 0) {
            for(i=0;i<inno_max_dev;i++){
                if(idev[i].vendor_id) {
                    memset(&(idev[i].inno_stats.wb_stats),0,sizeof(idev[i].inno_stats.wb_stats));
                }
            }
        }
    }
    return count;
}

ssize_t
genl_stats_r( struct device * dev,
        struct device_attribute * attr,
        char * buf ){
    inno_device_t  *idev;
    int len;
    inno_gen_netlink_tid_t *inno_genl_trapid;
    struct list_head *l = NULL ;
    inno_gen_netlink_t *inno_genl=NULL;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    if(list_empty(&idev->trapid_list) == 1) {
        return 0;
    }

    len = scnprintf(buf,PAGE_SIZE,"Generic Netlink Trapid Stats\n");
    len += scnprintf(buf+len,PAGE_SIZE-len," %20s %20s %20s %20s %20s %20s\n",
                     "Trap","Name","Type","Packets","Bytes","Drops");
    list_for_each(l, &idev->trapid_list){
        inno_genl_trapid = list_entry(l, struct inno_gen_netlink_tid_s, list);
        inno_genl = inno_genl_trapid->inno_genl;
        len += scnprintf(buf+len,PAGE_SIZE-len," %20u %20s %20u %20llu %20llu %20llu\n",
                inno_genl_trapid->trapid, inno_genl->inno_genl_family.name, inno_genl->nl_type,
                inno_genl_trapid->stats.rx_packets, inno_genl_trapid->stats.rx_bytes,
                inno_genl_trapid->stats.rx_drops);
    }
    return len;
}

ssize_t
genl_stats_w( struct device * dev,
              struct device_attribute * attr,
              const char * buf,
              size_t count ){
    inno_device_t  *idev;
    int val;

    idev = dev_get_drvdata(dev);
    if(idev == NULL) {
        ipd_err("Innovium device info not found");
        return -EINVAL;
    }

    if(kstrtoint(buf,10,&val) < 0 ){
        ipd_err("Unexpected value");
        return -EINVAL;
    }else {
        if(val == 0) {
            ipd_err("Not supported\n");
        }
    }
    return count;
}

DEVICE_ATTR(info,0644,ipd_info_r,NULL);
DEVICE_ATTR(ring_stat,0644,ring_stat_r,ring_stat_w);
DEVICE_ATTR(interrupt_stat,0644,interrupt_stat_r,interrupt_stat_w);
DEVICE_ATTR(tx_stat,0644,tx_stat_r,tx_stat_w);
DEVICE_ATTR(wb_stat,0644,wb_stat_r,wb_stat_w);
DEVICE_ATTR(loglevel,0644,loglevel_r,loglevel_w);
DEVICE_ATTR(genl_stats,0644,genl_stats_r,genl_stats_w);

void
inno_sysfs_init(inno_device_t  *idev,
        int max_device) {

    if(ipd_dev == NULL) {
        ipd_dev = root_device_register("ipd");
        if (IS_ERR(ipd_dev)) {
            ipd_err("Could not create sysfs dir");
            ipd_dev = NULL;
            return;
        }
    }
    inno_max_dev = max_device;
    dev_set_drvdata(ipd_dev,idev);
    if(sysfs_create_file(&ipd_dev->kobj,&dev_attr_info.attr) < 0 ){
        ipd_err("Sysfs file %s create failed",dev_attr_info.attr.name);
    }

    if(sysfs_create_file(&ipd_dev->kobj,&dev_attr_ring_stat.attr) < 0){
        ipd_err("Sysfs file %s create failed",dev_attr_ring_stat.attr.name);
    }

    if(sysfs_create_file(&ipd_dev->kobj,&dev_attr_interrupt_stat.attr) < 0){
        ipd_err("Sysfs file %s create failed",dev_attr_interrupt_stat.attr.name);
    }

    if(sysfs_create_file(&ipd_dev->kobj,&dev_attr_loglevel.attr) < 0){
        ipd_err("Sysfs file %s create failed",dev_attr_loglevel.attr.name);
    }

    if(sysfs_create_file(&ipd_dev->kobj,&dev_attr_tx_stat.attr) < 0){
        ipd_err("Sysfs file %s create failed",dev_attr_tx_stat.attr.name);
    }

    if(sysfs_create_file(&ipd_dev->kobj,&dev_attr_wb_stat.attr) < 0){
        ipd_err("Sysfs file %s create failed",dev_attr_wb_stat.attr.name);
    }

    if(sysfs_create_file(&ipd_dev->kobj,&dev_attr_genl_stats.attr) < 0){
        ipd_err("Sysfs file %s create failed",dev_attr_genl_stats.attr.name);
    }
}

void
inno_sysfs_deinit(void) {
    if(ipd_dev != NULL){
        sysfs_remove_file(&ipd_dev->kobj,&dev_attr_info.attr);
        sysfs_remove_file(&ipd_dev->kobj,&dev_attr_ring_stat.attr);
        sysfs_remove_file(&ipd_dev->kobj,&dev_attr_interrupt_stat.attr);
        sysfs_remove_file(&ipd_dev->kobj,&dev_attr_loglevel.attr);
        sysfs_remove_file(&ipd_dev->kobj,&dev_attr_tx_stat.attr);
        sysfs_remove_file(&ipd_dev->kobj,&dev_attr_genl_stats.attr);
        root_device_unregister(ipd_dev);
    }
}
