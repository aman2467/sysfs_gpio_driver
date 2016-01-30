/*
 * sysfs_gpio_driver : A driver for GPIOs using sysfs filesystem.
 * Copyright (C)2016  Aman Kumar
 *
 * This file is part of sysfs_gpio_driver.
 *
 * sysfs_gpio_driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * sysfs_gpio_driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sysfs_gpio_driver.
 * If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @example gpio_app.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gpio.h>

#define CPU_GPIO_MIN		0
#define CPU_GPIO_MAX		120
#define EXPANDER_GPIO_MIN	121
#define EXPANDER_GPIO_MAX	128

/**
 * GPIO return code names
 */
static char *error[] = {
	"GPIO_FILE_UNAVAILABLE",
	"GPIO_OPEN_FAIL",
	"GPIO_WRITE_FAIL",
	"GPIO_READ_FAIL",
	"GPIO_FAILURE",
	"GPIO_SUCCESS",
};

/**
 * @func error_name
 *
 * @param ret return code
 * @return return name
 */
static char *error_name(int ret)
{
	int index = ret + 5;

	return error[index];
}

/**
 * @main
 *
 * Main body of the example application
 */
int main(int argc, char **argv)
{
	gpio_device_t dev;
	device_desc_t desc;
	int gpio, value, ret;

	if (argc != 2) {
		printf("Usage : %s <gpio number>\n", argv[0]);
		return -1;
	}
	gpio = atoi(argv[1]);

	if (gpio <= CPU_GPIO_MAX &&
	    gpio >= CPU_GPIO_MIN) {
		desc.range.min = CPU_GPIO_MIN;
		desc.range.max = CPU_GPIO_MAX;
	} else if (gpio >= EXPANDER_GPIO_MIN &&
		   gpio <= EXPANDER_GPIO_MAX) {
		desc.range.min = EXPANDER_GPIO_MIN;
		desc.range.max = EXPANDER_GPIO_MAX;
	} else {
		printf("GPIO <%d> not available\n", gpio);
		return -1;
	}
	/* Init Driver */
	dev = gpio_sysfs_driver_init(&desc);
	if (NULL == dev) {
		printf("Device driver init failed\n");
		return -1;
	}
	/* GPIO setup as Output */
	ret = gpio_setup(dev, gpio, GPIO_DIRECTION_OUTPUT);
	if (ret != GPIO_SUCCESS) {
		printf("gpio_setup() failed : %s\n", error_name(ret));
		return -1;
	}
	ret = gpio_set_value(dev, gpio, GPIO_VALUE_HIGH);
	if (ret != GPIO_SUCCESS) {
		printf("gpio_set_value() failed : %s\n", error_name(ret));
		return -1;
	}
	value = gpio_get_value(dev, gpio);
	if (value != GPIO_VALUE_HIGH &&
	    value != GPIO_VALUE_LOW) {
		printf("gpio_get_value() failed : %s\n", error_name(value));
		return -1;
	}
	sleep(1);
	ret = gpio_set_value(dev, gpio, GPIO_VALUE_LOW);
	if (ret != GPIO_SUCCESS) {
		printf("gpio_set_value() failed : %s\n", error_name(ret));
		return -1;
	}
	value = gpio_get_value(dev, gpio);
	if (value != GPIO_VALUE_HIGH &&
	    value != GPIO_VALUE_LOW) {
		printf("gpio_get_value() failed : %s\n", error_name(value));
		return -1;
	}
	ret = gpio_cleanup(dev, gpio);
	if (ret != GPIO_SUCCESS) {
		printf("gpio_cleanup() failed : %s\n", error_name(ret));
		return -1;
	}

	/* GPIO setup as Input */
	ret = gpio_setup(dev, gpio, GPIO_DIRECTION_INPUT);
	if (ret != GPIO_SUCCESS) {
		printf("gpio_setup() failed : %s\n", error_name(ret));
		return -1;
	}
	value = gpio_get_value(dev, gpio);
	if (value != GPIO_VALUE_HIGH &&
	    value != GPIO_VALUE_LOW) {
		printf("gpio_get_value() failed : %s\n", error_name(value));
		return -1;
	}
	ret = gpio_cleanup(dev, gpio);
	if (ret != GPIO_SUCCESS) {
		printf("gpio_cleanup() failed : %s\n", error_name(ret));
		return -1;
	}

	/* Destroy driver */
	gpio_sysfs_driver_destroy(dev);

	return 0;
}
