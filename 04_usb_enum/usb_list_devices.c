// SPDX-License-Identifier: GPL-2.0
/*
 * List USB devices through libusb and print descriptor fields that are useful
 * when learning enumeration.
 */

#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>

static void print_device(libusb_device *dev)
{
    struct libusb_device_descriptor desc;
    uint8_t bus = libusb_get_bus_number(dev);
    uint8_t address = libusb_get_device_address(dev);
    int ret = libusb_get_device_descriptor(dev, &desc);

    if (ret != 0) {
        fprintf(stderr, "failed to read descriptor: %s\n", libusb_error_name(ret));
        return;
    }

    printf("bus=%03u addr=%03u vid=%04x pid=%04x class=0x%02x configs=%u\n",
           bus,
           address,
           desc.idVendor,
           desc.idProduct,
           desc.bDeviceClass,
           desc.bNumConfigurations);
}

int main(void)
{
    libusb_device **list = NULL;
    ssize_t count;
    ssize_t i;
    int ret;

    ret = libusb_init(NULL);
    if (ret != 0) {
        fprintf(stderr, "libusb_init failed: %s\n", libusb_error_name(ret));
        return 1;
    }

    count = libusb_get_device_list(NULL, &list);
    if (count < 0) {
        fprintf(stderr, "libusb_get_device_list failed: %s\n", libusb_error_name((int)count));
        libusb_exit(NULL);
        return 1;
    }

    for (i = 0; i < count; i++)
        print_device(list[i]);

    libusb_free_device_list(list, 1);
    libusb_exit(NULL);
    return 0;
}
