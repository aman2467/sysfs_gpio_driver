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
 * @file gpio.h
 *
 * GPIO driver
 */

#ifndef _GPIO_H
#define _GPIO_H

/**
 * GPIO return codes type
 */
enum gpio_return_code {
	GPIO_FILE_UNAVAILABLE = -5,
	GPIO_OPEN_FAIL,
	GPIO_WRITE_FAIL,
	GPIO_READ_FAIL,
	GPIO_FAILURE,
	GPIO_SUCCESS,
};

/**
 * GPIO values
 */
enum gpio_value {
	GPIO_VALUE_UNKNOWN = -1,
	GPIO_VALUE_LOW,
	GPIO_VALUE_HIGH,
};

/**
 * GPIO directions
 */
enum gpio_direction {
	GPIO_DIRECTION_UNKNOWN = -1,
	GPIO_DIRECTION_INPUT,
	GPIO_DIRECTION_OUTPUT,
};

/**
 * GPIO range parameters
 * Used to set boundary for a GPIO device
 */
typedef struct gpio_range {
	int min;	/**< Minimum GPIO number for a device */
	int max;	/**< Maximum GPIO number for a device */
} gpio_range_t;

/**
 * Device descriptor
 */
typedef struct device_desc {
	gpio_range_t range;/**< Describes GPIO range */
} device_desc_t;

/** GPIO device handle */
typedef void *gpio_device_t;

/**
 * Initializes a gpio driver device for a given range of GPIO
 *
 * @param range  pointer to gpio_range_t
 * @return gpio_device_t returns valid gpio device driver handle
 *			on success else returns NULL
 */
gpio_device_t gpio_sysfs_driver_init(device_desc_t *desc);

/**
 * Cleans up a given GPIO device driver
 *
 * @param device handle to a GPIO device driver
 * @return void
 */
void gpio_sysfs_driver_destroy(gpio_device_t device);

/**
 * Initializes a given GPIO
 *
 * @param device Handle to a valid GPIO device driver
 * @param gpio GPIO pin to setup
 * @param dir Direction of the GPIO pin to set
 * @return Non zero on failure or 0 on success
 */
int gpio_setup(gpio_device_t device, int gpio, int dir);

/**
 * Cleans up a given GPIO
 *
 * @param device Handle to a valid GPIO device driver
 * @param gpio GPIO pin to cleanup
 * @return Non zero on failure or 0 on success
 */
int gpio_cleanup(gpio_device_t device, int gpio);

/**
 * Writes a given value to a given GPIO
 *
 * @param device Handle to a valid GPIO device driver
 * @param gpio GPIO pin to write a value
 * @param value Value of GPIO pin to set
 * @return Non zero on failure or 0 on success
 */
int gpio_set_value(gpio_device_t device, int gpio, int value);

/**
 * Reads the value of a given GPIO
 *
 * @param device Handle to a valid GPIO device driver
 * @param gpio GPIO pin to read
 * @return value of GPIO on success or negative number on failure
 */
int gpio_get_value(gpio_device_t device, int gpio);

#endif /* _GPIO_H */
