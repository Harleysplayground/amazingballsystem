//
// CS 431 - Lab 03 Server Skeleton
// PC/Linux (Provided)
//

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "pc_crc16.h"
#include "lab03.h"

#include <string.h>

int main(int argc, char* argv[])
{
    double troll_pct=0;		// Perturbation % for the troll (if needed)
    int ifd,ofd,troll=0;	// Input and Output file descriptors (serial/troll)
    char str[MSG_BYTES_MSG],opt;	// String input
    struct termios oldtio, tio;	// Serial configuration parameters
    int VERBOSE = 0;		// Verbose output - can be overriden with -v

    // Command line options
    while ((opt = getopt(argc, argv, "t:v")) != -1) {
        switch (opt) {
            case 't':	troll = 1; 
                        troll_pct = atof(optarg);
                        break;
            case 'v':	VERBOSE = 1; break;
            default: 	break;
        }
    }

    printf("CS431 - Lab 03 Server\n(Enter a message to send.  Type \"quit\" to exit)\n");


    //
    // WRITE ME: Open the serial port (/dev/ttyS0) read-write
    //

    ifd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);

    // Start the troll if necessary
    if (troll)
    {
        // Open troll process (lab03_troll) for output only
        FILE * pfile;		// Process FILE for troll (used locally only)
        char cmd[128];		// Shell command

        snprintf(cmd, 128, "./lab03_troll -p%f %s", troll_pct, (VERBOSE) ? "-v" : "");

        pfile = popen(cmd, "w");
        if (!pfile) { perror("lab03_troll"); exit(-1); }
        ofd = fileno(pfile);
    }
    else ofd = ifd;		// Use the serial port for both input and output



    //
    // WRITE ME: Set up the serial port parameters and data format
    //

    tcgetattr(ifd, &oldtio);
    tio.c_cflag 	= B9600 | CS8 | CLOCAL | CREAD;
    tio.c_iflag 	= 0;
    tio.c_oflag 	= 0;
    tio.c_lflag 	= 0;
    tcflush(ifd, TCIFLUSH);
    tcsetattr(ifd, TCSANOW, &tio);


    while(1)
    {

        //
        // WRITE ME: Read a line of input (Hint: use fgetc(stdin) to read each character)
        //

        int count = 0;
        /*
        while (((str[count] = fgetc(stdin)) != EOF) && (str[count++] != '\n') && count < MSG_BYTES_MSG_LEN);
        str[count - 1] = '\0';
        */
        fgets(str, MSG_BYTES_MSG, stdin);
        count = strlen(str);
        str[count - 1] = 0;

        if (strcmp(str, "quit") == 0) break;

        //
        // WRITE ME: Compute crc (only lowest 16 bits are returned)
        //

        int crc = pc_crc16(str, count);
        char ack = 0;
        int attempts = 0;
        char sendbuf[MSG_BYTES_MSG + 4];
        sendbuf[0] = 0;
        sendbuf[2] = crc & 0xFF;
        sendbuf[1] = (crc >> 8) & 0xFF;
        sendbuf[3] = count;
        strcpy(&sendbuf[4], str);

        while (!ack)
        {
            printf("Sending (attempt %d)...\n", ++attempts);


            // 
            // WRITE ME: Send message
            //

            write(ofd, sendbuf, count + 4);

            printf("Message sent, waiting for ack... ");


            //
            // WRITE ME: Wait for MSG_ACK or MSG_NACK
            //

            read(ifd, &ack, 1); 

            printf("%s\n", ack ? "ACK" : "NACK, resending");
        }
        printf("\n");
    }



    //
    // WRITE ME: Reset the serial port parameters
    //

    tcflush(ifd, TCIFLUSH);
    tcsetattr(ifd, TCSANOW, &oldtio);


    // Close the serial port
    close(ifd);

    return 0;
}
