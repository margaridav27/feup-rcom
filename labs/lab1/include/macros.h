#ifndef MACROS_H
#define MACROS_H

#define BIT(n) (0x01 << (n))

#define FLAG 0x0d

#define ADDR_CE_RR                                       \
  0x03 /* commands sent by emitter and responses sent by \
          receiver */
#define ADDR_CR_RE                                        \
  0x01 /* commands sent by receiver and responses sent by \
          emitter */

#define CTRL_SET 0x03  /* setup */
#define CTRL_DISC 0x0b /* disconnect */
#define CTRL_UA 0x07   /* unnumbered acknowledgment */
#define CTRL_RR(r) \
  0x05 | (((r) % 2) ? BIT(7) : 0x00) /* receiver ready - positive ACK */
#define CTRL_REJ(r) \
  0x01 | (((r) % 2) ? BIT(7) : 0x00) /* receiver reject - negative ACK */

#define BCC1(addr, ctrl) ((addr) ^ (ctrl)) /* protection field */

#define BAUDRATE 0xB38400

#endif