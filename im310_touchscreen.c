/*
 * EPSON IM-310 POS touchscreen driver
 *
 * This is experimental software, be carefull when using it !
 *
 * This driver was developed with reference to much code written by others,
 * particularly:
 *  - elo, gunze drivers by Vojtech Pavlik <vojtech@ucw.cz>;
 *  - wacom_w8001 driver by Jaya Kumar <jayakumar.lkml@gmail.com>;
 *  - the USB wacom input driver, credited to many people
 *    (see drivers/input/tablet/wacom.h);
 *  - new and old versions of linuxwacom / xf86-input-wacom credited to
 *    Frederic Lepied, France. <Lepied@XFree86.org> and
 *    Ping Cheng, Wacom. <pingc@wacom.com>;
 *  - and xf86wacom.c (a presumably ancient version of the linuxwacom code), by
 *    Frederic Lepied and Raph Levien <raph@gtk.org>;
 *  - wacom-serial-iv driver by Julian Squires <julian@cipht.net>.
 */

/* XXX To be removed before (widespread) release. */
#define DEBUG

#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "serio-ids.h"
#include <linux/slab.h>
#include <linux/completion.h>

MODULE_AUTHOR("Jean THOMAS <contact@tibounise.com>");
MODULE_DESCRIPTION("EPSON IM-310 POS touchscreen driver");
MODULE_LICENSE("GPL");

#define PACKET_LENGTH 11

struct im310 {
	struct input_dev *dev;
	struct completion cmd_done;
	int extra_z_bits;
	int idx;
	unsigned char data[32];
	char phys[32];
};

static void send_touch_event(struct im310 *im310, unsigned int pos_x, unsigned int pos_y, bool is_touching) {
	input_report_key(im310->dev, BTN_TOUCH,is_touching);
	input_report_abs(im310->dev, ABS_X, pos_x);
	input_report_abs(im310->dev, ABS_Y, pos_y);
	input_sync(im310->dev);
}

static void handle_packet(struct im310 *im310) {
	int x,y;
	char is_touching;

	sscanf(im310->data,"%c%4d,%4d\x0A",&is_touching,&x,&y);

	/* Finger away from the touchscreen */
	if (is_touching == 0x54) {
		send_touch_event(im310,x,y,1);
	} else if (is_touching == 0x52) {
		send_touch_event(im310,x,y,0);
	}
}


static irqreturn_t im310_interrupt(struct serio *serio, unsigned char data, unsigned int flags) {
	struct im310 *im310 = serio_get_drvdata(serio);

	if (im310->idx >= sizeof(im310->data)) {
		dev_dbg(&im310->dev->dev, "throwing away %d bytes of garbage\n",
			im310->idx);
		im310->idx = 0;
	}

	/* Filling the data buffer */
	im310->data[im310->idx++] = data;

	/* When we have our (holy) packet */
	if (im310->idx == PACKET_LENGTH) {
		handle_packet(im310);
		im310->idx = 0;
	}
	return IRQ_HANDLED;
}

static void im310_setup(struct im310 *im310) {
	input_set_abs_params(im310->dev, ABS_X, 0, 1024, 0, 0);
	input_set_abs_params(im310->dev, ABS_Y, 1024, 0, 0, 0);
}

static int im310_connect(struct serio *serio, struct serio_driver *drv) {
	struct im310 *im310;
	struct input_dev *input_dev;
	int err = -ENOMEM;

	im310 = kzalloc(sizeof(struct im310), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!im310 || !input_dev)
		goto fail0;

	im310->dev = input_dev;
	im310->extra_z_bits = 1;
	im310->idx = 0;
	snprintf(im310->phys, sizeof(im310->phys), "%s/input0", serio->phys);

	input_dev->name = "EPSON IM-310 POS touchscreen";
	input_dev->phys = im310->phys;
	input_dev->id.bustype = BUS_RS232;
	input_dev->id.vendor  = SERIO_IM310;
	input_dev->id.product = serio->id.extra;
	input_dev->id.version = 0x0100;
	input_dev->dev.parent = &serio->dev;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	//__set_bit(BTN_TOUCH, input_dev->keybit);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH); /* is it the same as above ? */

	serio_set_drvdata(serio, im310);

	err = serio_open(serio, drv);
	if (err)
		goto fail1;

	im310_setup(im310);

	err = input_register_device(im310->dev);
	if (err)
		goto fail2;

	return 0;

 fail2:	serio_close(serio);
 fail1:	serio_set_drvdata(serio, NULL);
 fail0:	input_free_device(input_dev);
	kfree(im310);
	return err;
}

static void im310_disconnect(struct serio *serio) {
	struct im310 *im310 = serio_get_drvdata(serio);

	serio_close(serio);
	serio_set_drvdata(serio, NULL);
	input_unregister_device(im310->dev);
	kfree(im310);
}

static struct serio_device_id im310_serio_ids[] = {
	{
		.type	= SERIO_RS232,
		.proto	= SERIO_IM310,
		.id	    = SERIO_ANY,
		.extra	= SERIO_ANY,
	},
	{ 0 }
};

MODULE_DEVICE_TABLE(serio, im310_serio_ids);

static struct serio_driver im310_drv = {
	.driver		= {
		.name	= "im310_touchscreen",
	},
	.description	= "EPSON IM-310 POS touchscreen driver",
	.id_table	= im310_serio_ids,
	.interrupt	= im310_interrupt,
	.connect	= im310_connect,
	.disconnect	= im310_disconnect,
};

static int __init im310_init(void) {
	return serio_register_driver(&im310_drv);
}

static void __exit im310_exit(void) {
	serio_unregister_driver(&im310_drv);
}

module_init(im310_init);
module_exit(im310_exit);
