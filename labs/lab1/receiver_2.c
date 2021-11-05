/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7D
#define CMD_BUF 0x01
#define MSG_BUF 0x03 
#define C_UA 0x07
#define C_SET 0x03
#define BCC CMD_BUF^C_UA

enum state {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
};

int main(int argc, char** argv)
{
    int fd, c, res;
    unsigned char ua[5];
    unsigned char rcv[1];
    
    enum state msg_state = START;
    struct termios oldtio,newtio;

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS0", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }


    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) { perror(argv[1]); exit(-1); }

    if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */


    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    printf("Received from emissor: ");
    
    /* read SET sent from emissor */
    while (msg_state != STOP) {
        res = read(fd,rcv,1); 
        if (res > 0) printf("%x ", rcv[0]);
        
        switch(msg_state) {
            case START:
                if (rcv[0] == FLAG) msg_state = FLAG_RCV;
                break;
            case FLAG_RCV:
                if (rcv[0] == MSG_BUF) msg_state = A_RCV;
                else msg_state = START;
                break;
            case A_RCV:
                if (rcv[0] == C_SET) msg_state = C_RCV;
                else if (rcv[0] == FLAG) msg_state = FLAG_RCV;
                else msg_state = START;
                break;
            case C_RCV:
                if (rcv[0] == BCC) msg_state = BCC_OK;
                else if (rcv[0] == FLAG) msg_state = FLAG_RCV;
                else msg_state = START;
                break;
            case BCC_OK:
                if (rcv[0] == FLAG) msg_state = STOP;
                else msg_state = START;
                break;
            default:
                msg_state = START;
                break;
        }
    }
      
    printf("\n");

    /* assemble UA */
    ua[0] = FLAG;
    ua[1] = CMD_BUF;
    ua[2] = C_UA;
    ua[3] = BCC;
    ua[4] = FLAG;
    
    /* send UA to emissor */
    res = write(fd,ua,5);
    printf("Sent to emissor: %x %x %x %x %x\n", ua[0], ua[1], ua[2], ua[3], ua[4]);

    sleep(2);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
