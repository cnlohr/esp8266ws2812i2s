#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../user/pattern.h"

#ifndef WS_SLEEP
	#define WS_SLEEP 14000
#endif

int sockfd;
struct sockaddr_in servaddr;

int main( int argc, char ** argv )
{
	int firstoverride = -1;
	uint16_t lights = 186;
	uint8_t pattern = 0;
	unsigned ws_sleep = WS_SLEEP;
	if( argc < 2 )
	{
		fprintf( stderr, "Error: usage: %s ip [no. of light=186] [override 1st LED (i.e. white on some systems))=-1] [pattern=-1] [sleep seconds=%g]\n", argv[0], (double)ws_sleep );
		return -1;
	}
	uint32_t frame = 0;
	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(argv[1]);
	servaddr.sin_port=htons(COM_PORT);

	if( argc >= 3 ) lights = atoi( argv[2] );
	if( argc >= 4 ) firstoverride = atoi( argv[3] );
	if( argc >= 5 ) pattern = atoi( argv[4] );
	if( argc >= 6 ) ws_sleep = atoi( argv[5] );

	printf( "Lights: %d\n", lights );

	while(1) {
		uint8_t buffer[lights*3];
		unsigned i;
		for( i = 0; i < lights; i++ ) {
			uint32_t hex = 0;

			hex = hex_pattern(  pattern, i, lights, frame, NULL );

			buffer[0+i*3] = (hex>>8);
			buffer[1+i*3] = (hex);
			buffer[2+i*3] = (hex>>16);

		}

		if( firstoverride >= 0 )
			buffer[0] = firstoverride;

#ifdef BEAT
		for( i = 0; i < 4;i ++ )
		{
			uint32_t hex = ((-frame % 48)+48)*256/48;//((frame %48) == 0)?0xffffff:0x000000;
			hex |= hex<<8 | hex<<16;
			buffer[0+i*3] = (hex>>8);
			buffer[1+i*3] = (hex);
			buffer[2+i*3] = (hex>>16);
		}
#endif

		sendto(sockfd,buffer,sizeof(buffer),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
		frame++;
		#ifdef DEBUG
		for(i=0; i<sizeof(buffer)/sizeof(buffer[0]); i+=3) {
			unsigned j;
			for( j=0; j<3; ++j)
				printf( "%02x", buffer[i+j] );
			printf( " " );
		}
		printf( "\n" );
		#else
		putc('.', stdout);
		#endif
		fflush( stdout );
		usleep(ws_sleep);
	}
}

