EPSON IM-310 POS touchscreen driver for Linux
=============================================

This driver is based on [wacom-serial-iv](https://github.com/tokenrove/wacom-serial-iv) and [elo.c](http://www.cs.fsu.edu/~baker/devices/lxr/http/source/linux/drivers/input/touchscreen/elo.c).

There is a file named "touchdump", it is a dump of the touchscreen, where I press in the following order : the top left corner, the top right corner, the bottom left corner, and then the bottom right corner.

You can check out `scanfrule.c` to see how the touchscreen packets should be handled, using `sscanf()`.

## Howto

```
cd /tmp
sudo apt-get install git build-essential linux-headers-$(uname -r)
git clone https://github.com/tibounise/im310-touchscreen
```

Now you can load the driver :
```
sudo insmod im310_touchscreen.ko
./inputattach -im310 /dev/ttyS3
```

Don't worry if the X or Y axis is inverted.

Finally, install [xinput_calibrator](https://github.com/tias/xinput_calibrator), and use it to calibrate the touchscreen.