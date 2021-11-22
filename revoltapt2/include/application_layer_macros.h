#ifndef APPLICATION_LAYER_MACROS_H
#define APPLICATION_LAYER_MACROS_H

#define BIT(n) (0x01 << (n))

#define OCTET_SIZE 256

/* ctrl & data packets */
#define PACKET_CTRL_IX 0

/* ctrl packet */
#define PACKET_CTRL_START 2     /* start data packet emission */
#define PACKET_CTRL_END 3       /* end data packet emission */
#define PACKET_DATA_FILE_SIZE 0 /* ctrl packet file size parameter identifier */
#define PACKET_DATA_FILE_NAME 1 /* ctrl packet file name parameter identifier */
#define PACKET_CTRL_V1_IX 3

/* data packet */
#define PACKET_CTRL_DATA 1 /* data packet identifier */
#define PACKET_DATA_SEQ_NUM_IX 1
#define PACKET_DATA_LENGTH_MSB_IX 2
#define PACKET_DATA_LENGTH_LSB_IX 3
#define PACKET_DATA_START_IX 4
#define PACKET_DATA_SEQ_NUM_SIZE 255

#define TRANSMITTER_ID 0
#define RECEIVER_ID 1

#endif
