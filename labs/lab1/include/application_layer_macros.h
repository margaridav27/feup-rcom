#ifndef APPLICATION_LAYER_MACROS_H
#define APPLICATION_LAYER_MACROS_H

#define BIT(n) (0x01 << (n))

#define OCTET_SIZE 256

/* ctrl & data packets */
#define CTRL_IX 0

/* ctrl packet */
#define CTRL_START 2     /* start data packet emission */
#define CTRL_END 3       /* end data packet emission */
#define DATA_FILE_SIZE 0 /* ctrl packet file size parameter identifier */
#define DATA_FILE_NAME 1 /* ctrl packet file name parameter identifier */

/* data packet */
#define CTRL_DATA 1 /* data packet identifier */
#define DATA_SEQ_NUM_IX 1
#define DATA_LENGTH_MSB_IX 2
#define DATA_LENGTH_LSB_IX 3
#define DATA_START_IX 4
#define DATA_SEQ_NUM_SIZE 256

#define TRANSMITTER_ID 0
#define RECEIVER_ID 1

#endif