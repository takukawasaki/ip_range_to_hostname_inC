#ifndef lint
char copyright[] =
     "@ copyright (c) 2016 realm00.com.\n\
AA rights reserved.\n";

#endif



#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define S_HEX "%x.%x.%x.%x"
#define S_DEC "%d.%d.%d.%d"


void
bad_addr(addr)
int addr;
{
     fprintf(stderr, "Value %d is not valid.\n", addr);
     exit(-1);
}


void
check_addr(addr)
int addr[];
{
     register int i;
     for (i = 0; i < 4; i++) {
          if (addr[i] < 0 || addr[i] >255) {
               bad_addr(addr[i]);
          }
     }
}

void
usage(name)
char *name;
{
     fprintf(stderr,
             "usage: %s [-x] [-a] [-f <outfile>] aaa.bbb.[ccc||0].[ddd||0]\n",
             name);
     exit(-1);
}


int
main(argc, argv)
int argc;
char *argv[];
{
     char addr[4], **ptr;
     char *progname = argv[0];
     register int s;
     int a[4], arg, c,d,
          classB, classC, single,
          alias, hex, file;

     extern char *optarg;
     struct hostent *host,
          *gethostbyaddr();

     FILE *outfd = stdout;

     classB = classC = single = alias = hex = file = 0;
     c = d = 0;

     while ((arg = getopt(argc, argv, "xaf:")) != EOF) {
          switch (arg) {
          case 'x':
               hex++;
               break;
          case 'a':
               alias++;
               break;
          case 'f':
               file++;
               if ((outfd = fopen(optarg, "a")) == NULL) {
                    perror("open");
                    exit(-1);
               }
               break;
          default:
               usage(progname);
          }
     }

     argv += ((file)?2:0) + ((alias)?1:0) + ((hex)?1:0);
     argc -= ((file)?2:0) + ((alias)?1:0) + ((hex)?1:0);

     if (argc != 2) {
          usage(progname);
     }
     sscanf(argv[1], (hex)?S_HEX:S_DEC, &a[0], &a[1], &a[2], &a[3]);

     check_addr(a);
     
     if (!a[3]) {
          if (!a[2]) {
               classB++;
          }else{
               classC++;
          }
     }else{
          single++;
     }

     if (!classB && !classC & !single) {
          usage(progname);
     }

     if (file) {
          if ((s = fork()) > 0) {
               fprintf(stderr, "[%s - pid %d]\n", progname, s);
               exit(0);
          }else if(s < 0){
               perror("fork");
               exit(-1);
          }

          if ((s = open("/dev/tty", O_RDWR)) > 0) {
               ioctl(s, TIOCNOTTY, (char *)NULL);
               close(s);
          }
     }

     addr[0] = (unsigned char ) a[0];
     addr[1] = (unsigned char ) a[1];

     if (classC) {
          goto jmpC;
     }else if(single){
          goto jmpS;
          
     }

     fprintf(((file)?outfd:stderr), "Scanning Class B network %d.%d...\n",
             a[0], a[1]);

     while (c < 256) {
          a[2] = c++;
          d = 0;
         

     jmpC:
          fprintf(((file)?outfd:stderr), "Scanning Class C network %d.%d.%d...\n",
                  a[0], a[1], a[2]);

          while (d < 256) {
               a[3] = d++;
          

          jmpS:
               addr[2] = (unsigned char) a[2];
               addr[3] = (unsigned char) a[3];
               
               if ((host = gethostbyaddr(addr, 4, AF_INET)) != NULL) {
                    fprintf(outfd, "%d.%d.%d.%d => %s\n", a[0], a[1], a[2],a[3], host->h_name);
                    ptr = host->h_aliases;
                    if(alias){
                         while (*ptr != NULL) {
                              fprintf(outfd, "%d.%d.%d.%d => %s (alias)\n",
                                      a[0],a[1],a[2],a[3], *ptr);
                              ptr++;
                         }
                    }
                    fflush(outfd);
                    if (single) {
                         exit(0);
                    }
          
               }else if(single){
                    fprintf(stderr, "Cannot resolve %d.%d.%d.%d\n", a[0],a[1],a[2],a[3]);
                    exit(0);
               }
          }
          if(classC) {
               exit(0);
          }
     }
}
