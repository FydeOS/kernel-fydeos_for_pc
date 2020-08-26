// SPDX-License-Identifier: GPL-2.0
/* Bluetooth HCI driver model support. */

#include <linux/module.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

static struct class *bt_class;

static void bt_link_release(struct device *dev)
{
	struct hci_conn *conn = to_hci_conn(dev);
	kfree(conn);
}

static const struct device_type bt_link = {
	.name    = "link",
	.release = bt_link_release,
};

/*
 * The rfcomm tty device will possibly retain even when conn
 * is down, and sysfs doesn't support move zombie device,
 * so we should move the device before conn device is destroyed.
 */
static int __match_tty(struct device *dev, void *data)
{
	return !strncmp(dev_name(dev), "rfcomm", 6);
}

void hci_conn_init_sysfs(struct hci_conn *conn)
{
	struct hci_dev *hdev = conn->hdev;

	BT_DBG("conn %p", conn);

	conn->dev.type = &bt_link;
	conn->dev.class = bt_class;
	conn->dev.parent = &hdev->dev;

	device_initialize(&conn->dev);
}

void hci_conn_add_sysfs(struct hci_conn *conn)
{
	struct hci_dev *hdev = conn->hdev;

	BT_DBG("conn %p", conn);

	dev_set_name(&conn->dev, "%s:%d", hdev->name, conn->handle);

	if (device_add(&conn->dev) < 0) {
		bt_dev_err(hdev, "failed to register connection device");
		return;
	}

	hci_dev_hold(hdev);
}

void hci_conn_del_sysfs(struct hci_conn *conn)
{
	struct hci_dev *hdev = conn->hdev;

	if (!device_is_registered(&conn->dev))
		return;

	while (1) {
		struct device *dev;

		dev = device_find_child(&conn->dev, NULL, __match_tty);
		if (!dev)
			break;
		device_move(dev, NULL, DPM_ORDER_DEV_LAST);
		put_device(dev);
	}

	device_del(&conn->dev);

	hci_dev_put(hdev);
}

static void bt_host_release(struct device *dev)
{
	struct hci_dev *hdev = to_hci_dev(dev);
	kfree(hdev);
	module_put(THIS_MODULE);
}

static ssize_t identity_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct hci_dev *hdev = to_hci_dev(dev);

	return scnprintf(buf, 18, "%pMR", &hdev->bdaddr);
}
DEVICE_ATTR_RO(identity);

static ssize_t prepare_for_suspend_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct hci_dev *hdev = to_hci_dev(dev);
	char *state = hdev->enable_suspend_notifier ? "enabled" : "disabled";

	return scnprintf(buf, strlen(state) + 1, "%s", state);
}

static ssize_t prepare_for_suspend_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct hci_dev *hdev = to_hci_dev(dev);

	if (!buf)
		return -EINVAL;

	if (strncmp("enabled", buf, 7) == 0)
		hdev->enable_suspend_notifier = true;
	else if (strncmp("disabled", buf, 8) == 0)
		hdev->enable_suspend_notifier = false;
	else
		return -EINVAL;

	return count;
}
DEVICE_ATTR_RW(prepare_for_suspend);

static struct attribute *bt_host_attrs[] = {
	&dev_attr_identity.attr,
	&dev_attr_prepare_for_suspend.attr,
	NULL,
};
ATTRIBUTE_GROUPS(bt_host);

static const struct device_type bt_host = {
	.name    = "host",
	.release = bt_host_release,
	.groups = bt_host_groups,
};

void hci_init_sysfs(struct hci_dev *hdev)
{
	struct device *dev = &hdev->dev;

	dev->type = &bt_host;
	dev->class = bt_class;

	__module_get(THIS_MODULE);
	device_initialize(dev);
}

int __init bt_sysfs_init(void)
{
	bt_class = class_create(THIS_MODULE, "bluetooth");

	return PTR_ERR_OR_ZERO(bt_class);
}

void bt_sysfs_cleanup(void)
{
	class_destroy(bt_class);
}
