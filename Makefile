# sysfs_gpio_driver : A driver for GPIOs using sysfs filesystem.
# Copyright (C)2016  Aman Kumar
#
# This file is part of sysfs_gpio_driver.
#
# sysfs_gpio_driver is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# sysfs_gpio_driver is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with sysfs_gpio_driver.
# If not, see <http://www.gnu.org/licenses/>

BASE_PATH=${PWD}
GREEN=\033[01;32m
RED=\033[01;31m
NONE=\033[0m
EXAMPLE_DIR = ${BASE_PATH}/example
BIN_DIR = ${BASE_PATH}/bin
CC = gcc
DRIVER_DIR = ${BASE_PATH}/driver
DOC_DIR = ${BASE_PATH}/doc
DOC_NAME = "API_Manual_for_sysfs_gpio_driver.pdf"
INCLUDE_DIR = ${BASE_PATH}/include
CFLAGS = -I${INCLUDE_DIR} -Wall

.PHONY: all driver example bin doc clean clean_doc

all : driver example bin

driver:
	@${CC} -c ${CFLAGS} ${DRIVER_DIR}/gpio_driver.c -o ${DRIVER_DIR}/gpio_driver.o

example:
	@${CC} -c ${CFLAGS} ${EXAMPLE_DIR}/gpio_app.c -o ${EXAMPLE_DIR}/gpio_app.o

bin:
	@${CC} ${EXAMPLE_DIR}/gpio_app.o ${DRIVER_DIR}/gpio_driver.o -o ${BIN_DIR}/gpio_app
	@echo -e "${GREEN}All Done..!!${NONE}"

doc:
	@doxygen ${DOC_DIR}/dox.cfg
	@cd latex && make && cd -
	@mv latex/refman.pdf ${DOC_DIR}/${DOC_NAME}
	@rm -rf latex/
	@echo -e "${GREEN}All Done..!!${NONE}"

clean_doc:
	@rm -f ${DOC_DIR}/${DOC_NAME}
	@echo -e "${RED}All Done..!!${NONE}"

clean:
	@find . -name "*.o" | xargs rm -f
	@rm -f ${BIN_DIR}/*
	@echo -e "${RED}All Done..!!${NONE}"
