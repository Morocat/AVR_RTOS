/*
 * serial.h
 *
 * Created: 12/22/2017 2:35:42 PM
 *  Author: alelop
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

// baud definitions for 32 MHz clock
#define  SR_BAUD_300_V		6666
#define  SR_BAUD_600_V		3332
#define  SR_BAUD_1200_V		1666
#define  SR_BAUD_2400_V		832
#define  SR_BAUD_4800_V		416
#define  SR_BAUD_9600_V		207
#define  SR_BAUD_14400_V	138
#define  SR_BAUD_19200_V	103
#define  SR_BAUD_28800_V	68
#define  SR_BAUD_38400_V	51
#define  SR_BAUD_57600_V	34
#define  SR_BAUD_76800_V	25
#define  SR_BAUD_115200_V	16
#define  SR_BAUD_230400_V	8

void serial_init(void);
void serial_send_string(char *str);


#endif /* SERIAL_H_ */