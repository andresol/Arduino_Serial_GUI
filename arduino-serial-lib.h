/*
 * arduino-serial-lib.h
 *
 *  Created on: May 14, 2013
 *      Author: soland
 */

#ifndef ARDUINO_SERIAL_LIB_H_
#define ARDUINO_SERIAL_LIB_H_

#include <stdint.h>   // Standard types

int serialport_init(const char* serialport, int baud);
int serialport_close(int fd);
int serialport_writebyte( int fd, uint8_t b);
int serialport_write(int fd, const char* str);
int serialport_read_until(int fd, char* buf, char until, int buf_max,int timeout);
int serialport_flush(int fd);

#endif /* ARDUINO_SERIAL_LIB_H_ */
