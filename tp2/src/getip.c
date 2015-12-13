#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    struct hostent *h;
    struct in_addr **addr_list;
    int i;

    if (argc != 2) {  
        fprintf(stderr,"usage: getip address\n");
        exit(1);
    }

    /*
    struct hostent {
        char    *h_name;       //Official name of the host. 
        char    **h_aliases;   //A NULL-terminated array of alternate names for the host. 
        int     h_addrtype;    //The type of address being returned; usually AF_INET.
        int     h_length;      //The length of the address in bytes.
        char    **h_addr_list; //A zero-terminated array of network addresses for the host. 
                //Host addresses are in Network Byte Order. 
    };

    #define h_addr h_addr_list[0]   The first address in h_addr_list. 

*/
    if ((h=gethostbyname(argv[1])) == NULL) {  
        herror("gethostbyname");
        exit(1);
    }

    printf("Host name  : %s\n", h->h_name);
    addr_list = (struct in_addr **)h->h_addr_list;
    printf("  IP addresses: ");
    for(i = 0; addr_list[i] != NULL; i++) {
        printf("%s ", inet_ntoa(*addr_list[i]));
    }
    printf("\n");
    return 0;
}