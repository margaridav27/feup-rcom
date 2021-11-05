/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define CMD_BUF 0x03
#define MSG_BUF 0x01
#define FLAG 0x7d
#define SET 0x03
#define UA 0x07


enum state {START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP};
int conta=1;

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	conta++;
}

int main(int argc, char** argv)
{
    (void) signal(SIGALRM, atende); 
    int fd,c, res;
    struct termios oldtio,newtio;
    int i, sum = 0, speed = 0;
    unsigned char cmd[5], ans[1], ack[5];
    enum state msg_state = START;

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


    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    printf("New termios structure set\n");


    //construct command
    cmd[0] = FLAG;
    cmd[1] = CMD_BUF;
    cmd[2] = SET;
    cmd[3] = CMD_BUF ^ SET;
    cmd[4] = FLAG;

    res = write(fd,cmd,5);   

    printf("Enviado para o irecetor: %x %x %x %x %x (%d bytes)\n",cmd[0], cmd[1], cmd[2], cmd[3], cmd[4],res);

    
    printf("Feedback do recetor: \n");

    
    while (msg_state!=STOP && conta<4) {
        alarm(1);
        res = read(fd,ans,1);
        if (res > 0) printf("%x ",ans[0]);
        
        switch (msg_state) {
            case START:
                if (ans[0] == FLAG) msg_state = FLAG_RCV;
                break;
            case FLAG_RCV: 
                if (ans[0] == MSG_BUF) msg_state = A_RCV;
                else msg_state = START;
                break;
            case A_RCV: 
                if (ans[0] == UA) msg_state = C_RCV;
                else if (ans[0] == FLAG) msg_state = FLAG_RCV;
                else msg_state = START;
                break;
            case C_RCV: 
                if (ans[0] == MSG_BUF^UA) msg_state = BCC_OK;
                else if (ans[0] == FLAG) msg_state = FLAG_RCV;
                else msg_state = START;
                break;
            case BCC_OK: 
                if (ans[0] == FLAG) msg_state = STOP;
                else msg_state = START;
                break;
            default:
                msg_state = START;
                break;
            }       
    }

    printf("\nUA received\n");
    
   
    sleep(2);
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


    close(fd);
    return 0;
}

