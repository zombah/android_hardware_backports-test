/*
 *
 *  Bluetooth virtual HCI driver
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2004-2006  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/module.h>
#include <asm/unaligned.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/poll.h>

#include <linux/skbuff.h>
#include <linux/miscdevice.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

#define VERSION "1.4"

static bool amp;

struct vhci_data {
	struct hci_dev *hdev;

	wait_queue_head_t read_wait;
	struct sk_buff_head readq;

	struct delayed_work open_timeout;
};

static int vhci_open_dev(struct hci_dev *hdev)
{
	set_bit(HCI_RUNNING, &hdev->flags);

	return 0;
}

static int vhci_close_dev(struct hci_dev *hdev)
{
	struct vhci_data *data = hci_get_drvdata(hdev);

	if (!test_and_clear_bit(HCI_RUNNING, &hdev->flags))
		return 0;

	skb_queue_purge(&data->readq);

	return 0;
}

static int vhci_flush(struct hci_dev *hdev)
{
	struct vhci_data *data = hci_get_drvdata(hdev);

	skb_queue_purge(&data->readq);

	return 0;
}

static int vhci_send_frame(struct sk_buff *skb)
{
	struct hci_dev* hdev = (struct hci_dev *) skb->dev;
	struct vhci_data *data;

	if (!hdev) {
		BT_ERR("Frame for unknown HCI device (hdev=NULL)");
		return -ENODEV;
	}

	if (!test_bit(HCI_RUNNING, &hdev->flags))
		return -EBUSY;

	data = hci_get_drvdata(hdev);

	memcpy(skb_push(skb, 1), &bt_cb(skb)->pkt_type, 1);
	skb_queue_tail(&data->readq, skb);

	wake_up_interruptible(&data->read_wait);
	return 0;
}

static int vhci_create_device(struct vhci_data *data, __u8 dev_type)
{
	struct hci_dev *hdev;
	struct sk_buff *skb;

	skb = bt_skb_alloc(4, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	hdev = hci_alloc_dev();
	if (!hdev) {
		kfree_skb(skb);
		return -ENOMEM;
	}

	data->hdev = hdev;

	hdev->bus = HCI_VIRTUAL;
	hdev->dev_type = dev_type;
	hci_set_drvdata(hdev, data);

	hdev->open  = vhci_open_dev;
	hdev->close = vhci_close_dev;
	hdev->flush = vhci_flush;
	hdev->send  = vhci_send_frame;

	if (hci_register_dev(hdev) < 0) {
		BT_ERR("Can't register HCI device");
		hci_free_dev(hdev);
		data->hdev = NULL;
		kfree_skb(skb);
		return -EBUSY;
	}

	bt_cb(skb)->pkt_type = HCI_VENDOR_PKT;

	*skb_put(skb, 1) = 0xff;
	*skb_put(skb, 1) = dev_type;
	put_unaligned_le16(hdev->id, skb_put(skb, 2));
	skb_queue_tail(&data->readq, skb);

	wake_up_interruptible(&data->read_wait);
	return 0;
}

static inline ssize_t vhci_get_user(struct vhci_data *data,
				    const char __user *buf, size_t count)
{
	struct sk_buff *skb;
	__u8 pkt_type, dev_type;
	int ret;

	if (count < 2 || count > HCI_MAX_FRAME_SIZE)
		return -EINVAL;

	skb = bt_skb_alloc(count, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	if (copy_from_user(skb_put(skb, count), buf, count)) {
		kfree_skb(skb);
		return -EFAULT;
	}

	pkt_type = *((__u8 *) skb->data);
	skb_pull(skb, 1);

	switch (pkt_type) {
	case HCI_EVENT_PKT:
	case HCI_ACLDATA_PKT:
	case HCI_SCODATA_PKT:
		if (!data->hdev) {
			kfree_skb(skb);
			return -ENODEV;
		}

		bt_cb(skb)->pkt_type = pkt_type;

		ret = hci_recv_frame(data->hdev, skb);
		break;

	case HCI_VENDOR_PKT:
		if (data->hdev) {
			kfree_skb(skb);
			return -EBADFD;
		}

		cancel_delayed_work_sync(&data->open_timeout);

		dev_type = *((__u8 *) skb->data);
		skb_pull(skb, 1);

		if (skb->len > 0) {
			kfree_skb(skb);
			return -EINVAL;
		}

		kfree_skb(skb);

		if (dev_type != HCI_BREDR && dev_type != HCI_AMP)
			return -EINVAL;

		ret = vhci_create_device(data, dev_type);
		break;

	default:
		kfree_skb(skb);
		return -EINVAL;
	}

	return (ret < 0) ? ret : count;
}

static inline ssize_t vhci_put_user(struct vhci_data *data,
				    struct sk_buff *skb,
				    char __user *buf, int count)
{
	char __user *ptr = buf;
	int len;

	len = min_t(unsigned int, skb->len, count);

	if (copy_to_user(ptr, skb->data, len))
		return -EFAULT;

	if (!data->hdev)
		return len;

	data->hdev->stat.byte_tx += len;

	switch (bt_cb(skb)->pkt_type) {
	case HCI_COMMAND_PKT:
		data->hdev->stat.cmd_tx++;
		break;
	case HCI_ACLDATA_PKT:
		data->hdev->stat.acl_tx++;
		break;
	case HCI_SCODATA_PKT:
		data->hdev->stat.sco_tx++;
		break;
	}

	return len;
}

static ssize_t vhci_read(struct file *file,
			 char __user *buf, size_t count, loff_t *pos)
{
	struct vhci_data *data = file->private_data;
	struct sk_buff *skb;
	ssize_t ret = 0;

	while (count) {
		skb = skb_dequeue(&data->readq);
		if (skb) {
			ret = vhci_put_user(data, skb, buf, count);
			if (ret < 0)
				skb_queue_head(&data->readq, skb);
			else
				kfree_skb(skb);
			break;
		}

		if (file->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			break;
		}

		ret = wait_event_interruptible(data->read_wait,
					       !skb_queue_empty(&data->readq));
		if (ret < 0)
			break;
	}

	return ret;
}

static ssize_t vhci_write(struct file *file,
			  const char __user *buf, size_t count, loff_t *pos)
{
	struct vhci_data *data = file->private_data;

	return vhci_get_user(data, buf, count);
}

static unsigned int vhci_poll(struct file *file, poll_table *wait)
{
	struct vhci_data *data = file->private_data;

	poll_wait(file, &data->read_wait, wait);

	if (!skb_queue_empty(&data->readq))
		return POLLIN | POLLRDNORM;

	return POLLOUT | POLLWRNORM;
}

static void vhci_open_timeout(struct work_struct *work)
{
	struct vhci_data *data = container_of(work, struct vhci_data,
					      open_timeout.work);

	vhci_create_device(data, amp ? HCI_AMP : HCI_BREDR);
}

static int vhci_open(struct inode *inode, struct file *file)
{
	struct vhci_data *data;

	data = kzalloc(sizeof(struct vhci_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	skb_queue_head_init(&data->readq);
	init_waitqueue_head(&data->read_wait);

	INIT_DELAYED_WORK(&data->open_timeout, vhci_open_timeout);

	file->private_data = data;
	nonseekable_open(inode, file);

	schedule_delayed_work(&data->open_timeout, msecs_to_jiffies(1000));

	return 0;
}

static int vhci_release(struct inode *inode, struct file *file)
{
	struct vhci_data *data = file->private_data;
	struct hci_dev *hdev = data->hdev;

	cancel_delayed_work_sync(&data->open_timeout);

	if (hdev) {
		hci_unregister_dev(hdev);
		hci_free_dev(hdev);
	}

	file->private_data = NULL;
	kfree(data);

	return 0;
}

static const struct file_operations vhci_fops = {
	.owner		= THIS_MODULE,
	.read		= vhci_read,
	.write		= vhci_write,
	.poll		= vhci_poll,
	.open		= vhci_open,
	.release	= vhci_release,
	.llseek		= no_llseek,
};

static struct miscdevice vhci_miscdev= {
	.name	= "vhci",
	.fops	= &vhci_fops,
	.minor	= MISC_DYNAMIC_MINOR,
};

static int __init vhci_init(void)
{
	BT_INFO("Virtual HCI driver ver %s", VERSION);

	return misc_register(&vhci_miscdev);
}

static void __exit vhci_exit(void)
{
	misc_deregister(&vhci_miscdev);
}

module_init(vhci_init);
module_exit(vhci_exit);

module_param(amp, bool, 0644);
MODULE_PARM_DESC(amp, "Create AMP controller device");

MODULE_AUTHOR("Marcel Holtmann <marcel@holtmann.org>");
MODULE_DESCRIPTION("Bluetooth virtual HCI driver ver " VERSION);
MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL");
MODULE_ALIAS("devname:vhci");
