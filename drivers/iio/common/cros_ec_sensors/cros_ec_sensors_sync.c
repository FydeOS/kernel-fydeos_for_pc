/*
 * cros_ec_sensors_sync - Driver for synchronisation sensor behind CrOS EC.
 *
 * Copyright 2018 Google, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This driver uses the cros-ec interface to communicate with the Chrome OS
 * EC about counter sensors. Counters are presented through
 * iio sysfs.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/iio/buffer.h>
#include <linux/iio/common/cros_ec_sensors_core.h>
#include <linux/iio/iio.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/trigger.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/kernel.h>
#include <linux/mfd/cros_ec.h>
#include <linux/module.h>
#include <linux/platform_data/cros_ec_commands.h>
#include <linux/platform_data/cros_ec_proto.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

/*
 * One channel for timestamp.
 */
#define MAX_CHANNELS 1

/* State data for ec_sensors iio driver. */
struct cros_ec_sensors_sync_state {
	/* Shared by all sensors */
	struct cros_ec_sensors_core_state core;

	struct iio_chan_spec channels[MAX_CHANNELS];
};

static int cros_ec_sensors_sync_read(struct iio_dev *indio_dev,
				    struct iio_chan_spec const *chan,
				    int *val, int *val2, long mask)
{
	struct cros_ec_sensors_sync_state *st = iio_priv(indio_dev);
	int ret;

	mutex_lock(&st->core.cmd_lock);
	ret = cros_ec_sensors_core_read(&st->core, chan, val, val2, mask);
	mutex_unlock(&st->core.cmd_lock);
	return ret;
}

static int cros_ec_sensors_write(struct iio_dev *indio_dev,
				 struct iio_chan_spec const *chan,
				 int val, int val2, long mask)
{
	struct cros_ec_sensors_sync_state *st = iio_priv(indio_dev);
	int ret;

	mutex_lock(&st->core.cmd_lock);

	ret = cros_ec_sensors_core_write(
			&st->core, chan, val, val2, mask);

	mutex_unlock(&st->core.cmd_lock);
	return ret;
}

static const struct iio_info cros_ec_sensors_sync_info = {
	.read_raw = &cros_ec_sensors_sync_read,
	.write_raw = &cros_ec_sensors_write,
};

static int cros_ec_sensors_sync_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct iio_dev *indio_dev;
	struct cros_ec_sensors_sync_state *state;
	struct iio_chan_spec *channel;
	int ret;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*state));
	if (!indio_dev)
		return -ENOMEM;

	ret = cros_ec_sensors_core_init(pdev, indio_dev, true,
			cros_ec_sensors_capture, cros_ec_sensors_push_data);
	if (ret)
		return ret;

	indio_dev->info = &cros_ec_sensors_sync_info;
	state = iio_priv(indio_dev);
	channel = state->channels;
	channel->info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ);
	channel->info_mask_shared_by_all_available =
		BIT(IIO_CHAN_INFO_SAMP_FREQ);
	channel->type = IIO_TIMESTAMP;
	channel->channel = -1;
	channel->scan_index = 1;
	channel->scan_type.sign = 's';
	channel->scan_type.realbits = 64;
	channel->scan_type.storagebits = 64;

	indio_dev->channels = state->channels;
	indio_dev->num_channels = MAX_CHANNELS;

	state->core.read_ec_sensors_data = cros_ec_sensors_read_cmd;

	return devm_iio_device_register(dev, indio_dev);
}

static const struct platform_device_id cros_ec_sensors_sync_ids[] = {
	{
		.name = "cros-ec-sync",
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(platform, cros_ec_sensors_sync_ids);

static struct platform_driver cros_ec_sensors_sync_platform_driver = {
	.driver = {
		.name	= "cros-ec-sync",
	},
	.probe		= cros_ec_sensors_sync_probe,
	.id_table	= cros_ec_sensors_sync_ids,
};
module_platform_driver(cros_ec_sensors_sync_platform_driver);

MODULE_DESCRIPTION("ChromeOS EC synchronisation sensor driver");
MODULE_LICENSE("GPL v2");
