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
 * @file gpio_driver.c
 *
 * sysfs_gpio_driver
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <gpio.h>

/** Path to GPIO files entry in sysfs */
#define SYSFS_CLASS_GPIO "/sys/class/gpio/"
#define MAX_DEVICE 10	/**< Maximum number of allowed devices */
#define MAX_SIZE 50	/**< Maximum size of any array for internal use */

/**
 * GPIO setup operations
 */
enum operation {
	GPIO_EXPORT,
	GPIO_UNEXPORT,
};

/**
 * function pointers for sysfs_gpio_driver
 */
struct sysfs_gpio_fops {
	/** Callback for init function */
	int (*init)(int gpio);
	/** Callback function to set value of a GPIO */
	int (*set_val)(int gpio, int value);
	/** Callback function to get value of a GPIO */
	int (*get_val)(int gpio);
	/** Callback function to set direction of a GPIO */
	int (*set_dir)(int gpio, int direction);
	/** Callback function to get direction of a GPIO */
	int (*get_dir)(int gpio);
	/** Callback function to cleanup a GPIO */
	int (*destroy)(int gpio);
};

/**
 * GPIO driver
 */
typedef struct gpio_driver {
	int id;		/**< id of a device */
	gpio_range_t gpio_range;/**< Supported GPIO range by a device */
	struct sysfs_gpio_fops *gpio; /**< driver functions */
} gpio_driver_t;

/** counter for devices */
static int device_count = 0;
gpio_device_t dev_hndlr[MAX_DEVICE];

/***************PRIVATE APIS******************/

static int is_valid_path(char *path)
{
	int rv;

	rv = access(path, F_OK);
	return (rv == 0);
}

static int is_valid_value(int value)
{
	return !(value != GPIO_VALUE_HIGH &&
		 value != GPIO_VALUE_LOW);
}

static int is_valid_direction(int value)
{
	return !(value != GPIO_DIRECTION_INPUT &&
		 value != GPIO_DIRECTION_OUTPUT);
}

static int export_helper(int gpio, int operation)
{
	int fd = -1, size = 0;
	char buffer[4] = {0};
	int rv = GPIO_SUCCESS;

	switch (operation) {
		case GPIO_EXPORT:
			fd = open(SYSFS_CLASS_GPIO"export", O_WRONLY);
			break;
		case GPIO_UNEXPORT:
			fd = open(SYSFS_CLASS_GPIO"unexport", O_WRONLY);
			break;
	}
	if (-1 == fd)
		return GPIO_OPEN_FAIL;

	size = snprintf(buffer, 4, "%d", gpio);
	size = write(fd, buffer, size);
	if (0 > size)
		rv = GPIO_WRITE_FAIL;
	close(fd);

	return rv;
}

static int gpio_sys_export(int gpio)
{
	return export_helper(gpio, GPIO_EXPORT);
}

static int gpio_sys_set_direction(int gpio, int direction)
{
	char path[MAX_SIZE] = {0};
	char buffer[4] = {0};
	int size = 0, fd;

	snprintf(path, MAX_SIZE, SYSFS_CLASS_GPIO"gpio%d", gpio);
	if (is_valid_path(path)) {
		if (!is_valid_direction(direction))
			return GPIO_FAILURE;
		snprintf(path, MAX_SIZE,
			 SYSFS_CLASS_GPIO"gpio%d/direction", gpio);
		fd = open(path, O_WRONLY);
		if (-1 == fd)
			return GPIO_OPEN_FAIL;
		if (GPIO_DIRECTION_INPUT == direction)
			size = snprintf(buffer, 4, "in");
		else if (GPIO_DIRECTION_OUTPUT == direction)
			size = snprintf(buffer, 4, "out");
		size = write(fd, buffer, size);
		if (0 > size) {
			close(fd);
			return GPIO_WRITE_FAIL;
		}
		close(fd);
		return GPIO_SUCCESS;
	}
	return GPIO_FILE_UNAVAILABLE;
}

static int gpio_sys_get_direction(int gpio)
{
	int dir, fd, size = 0;
	char path[MAX_SIZE] = {0};
	char buffer[4] = {0};

	snprintf(path, MAX_SIZE, SYSFS_CLASS_GPIO"gpio%d", gpio);
	if (is_valid_path(path)) {
		snprintf(path, MAX_SIZE,
			 SYSFS_CLASS_GPIO"gpio%d/direction", gpio);
		fd = open(path, O_RDONLY);
		if (-1 == fd)
			return GPIO_OPEN_FAIL;
		size = read(fd, buffer, 4);
		if (0 > size) {
			close(fd);
			return GPIO_READ_FAIL;
		}
		close(fd);
		if (!strcmp(buffer, "out"))
			dir = GPIO_DIRECTION_OUTPUT;
		else if (!strcmp(buffer, "in"))
			dir = GPIO_DIRECTION_INPUT;
		else
			dir = GPIO_DIRECTION_UNKNOWN;

		return dir;
	}
	return GPIO_FILE_UNAVAILABLE;
}

static int gpio_sys_set_value(int gpio, int value)
{
	char path[MAX_SIZE] = {0};
	char buffer[4] = {0};
	int size = 0, fd;

	snprintf(path, MAX_SIZE, SYSFS_CLASS_GPIO"gpio%d", gpio);
	if (is_valid_path(path)) {
		if (GPIO_DIRECTION_INPUT == gpio_sys_get_direction(gpio))
			return GPIO_SUCCESS;
		snprintf(path, MAX_SIZE,
			 SYSFS_CLASS_GPIO"gpio%d/value", gpio);
		fd = open(path, O_WRONLY);
		if (-1 == fd)
			return GPIO_OPEN_FAIL;
		size = snprintf(buffer, 4, "%d", value);
		size = write(fd, buffer, size);
		if (0 > size) {
			close(fd);
			return GPIO_WRITE_FAIL;
		}
		close(fd);
		return GPIO_SUCCESS;
	}
	return GPIO_FILE_UNAVAILABLE;
}

static int gpio_sys_get_value(int gpio)
{
	int size = 0, fd, value;
	char path[MAX_SIZE] = {0};
	char buffer[4] = {0};

	snprintf(path, MAX_SIZE, SYSFS_CLASS_GPIO"gpio%d", gpio);
	if (is_valid_path(path)) {
		snprintf(path, MAX_SIZE,
			 SYSFS_CLASS_GPIO"gpio%d/value", gpio);
		fd = open(path, O_RDONLY);
		if (-1 == fd)
			return GPIO_OPEN_FAIL;
		size = read(fd, buffer, 3);
		if (0 > size) {
			close(fd);
			return GPIO_READ_FAIL;
		}
		close(fd);
		value = atoi(buffer);
		if (!is_valid_value(value))
			return GPIO_VALUE_UNKNOWN;
		return value;
	}
	return GPIO_FILE_UNAVAILABLE;
}

static int gpio_sys_unexport(int gpio)
{
	return export_helper(gpio, GPIO_UNEXPORT);
}

static struct sysfs_gpio_fops gpio_sys_fops = {
	.init = gpio_sys_export,
	.set_dir = gpio_sys_set_direction,
	.get_dir = gpio_sys_get_direction,
	.set_val = gpio_sys_set_value,
	.get_val = gpio_sys_get_value,
	.destroy = gpio_sys_unexport,
};

/************** PUBLIC APIS *****************/

gpio_device_t gpio_sysfs_driver_init(device_desc_t *desc)
{
	gpio_driver_t *dev = NULL;
	int i;

	if (device_count >= MAX_DEVICE)
		return (gpio_device_t)dev;
	else if (device_count) {
		for (i = 0; i < device_count; i++) {
			gpio_driver_t *hndlr = (gpio_driver_t *)dev_hndlr[i];

			if (((desc->range.min >= hndlr->gpio_range.min) &&
			     (desc->range.min <= hndlr->gpio_range.max)) ||
			    ((desc->range.max >= hndlr->gpio_range.min) &&
			     (desc->range.max <= hndlr->gpio_range.max)) ||
			    ((desc->range.min > hndlr->gpio_range.min) &&
			     (desc->range.max < hndlr->gpio_range.max)) ||
			    ((desc->range.min < hndlr->gpio_range.min) &&
			     (desc->range.max > hndlr->gpio_range.max)))
				return (gpio_device_t)dev;
		}
	}
	dev = malloc(sizeof(gpio_driver_t));
	if (NULL != dev) {
		dev_hndlr[device_count] = (gpio_device_t)dev;
		dev->id = device_count++;
		dev->gpio_range.min = desc->range.min;
		dev->gpio_range.max = desc->range.max;
		dev->gpio = &gpio_sys_fops;
	}

	return (gpio_device_t)dev;
}

void gpio_sysfs_driver_destroy(gpio_device_t dev)
{
	if (NULL != dev) {
		free(dev);
		if (device_count > 0) {
			device_count--;
			dev_hndlr[device_count] = NULL;
		}
	}
}

int gpio_setup(gpio_device_t device, int gpio, int dir)
{
	gpio_driver_t *dev = (gpio_driver_t *)device;
	int rv;

	if (NULL == dev)
		return GPIO_FAILURE;
	if (gpio >= dev->gpio_range.min &&
	    gpio <= dev->gpio_range.max) {
		rv = dev->gpio->init(gpio);
		if (rv)
			return rv;
		rv = dev->gpio->set_dir(gpio, dir);
		if (rv)
			return rv;
		return GPIO_SUCCESS;
	}

	return GPIO_FAILURE;
}

int gpio_cleanup(gpio_device_t device, int gpio)
{
	gpio_driver_t *dev = (gpio_driver_t *)device;

	if (NULL != dev)
		return dev->gpio->destroy(gpio);

	return GPIO_FAILURE;
}

int gpio_set_value(gpio_device_t device, int gpio, int value)
{
	gpio_driver_t *dev = (gpio_driver_t *)device;

	if (NULL != dev)
		return dev->gpio->set_val(gpio, value);

	return GPIO_FAILURE;
}

int gpio_get_value(gpio_device_t device, int gpio)
{
	gpio_driver_t *dev = (gpio_driver_t *)device;

	if (NULL != dev)
		return dev->gpio->get_val(gpio);

	return GPIO_VALUE_UNKNOWN;
}
