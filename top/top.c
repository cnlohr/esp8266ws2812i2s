#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int sockfd;
struct sockaddr_in servaddr;


unsigned long HSVtoHEX( float hue, float sat, float value )
{

	float pr = 0;
	float pg = 0;
	float pb = 0;

	short ora = 0;
	short og = 0;
	short ob = 0;

	float ro = fmod( hue * 6, 6. );

	float avg = 0;

	ro = fmod( ro + 6 + 1, 6 ); //Hue was 60* off...

	if( ro < 1 ) //yellow->red
	{
		pr = 1;
		pg = 1. - ro;
	} else if( ro < 2 )
	{
		pr = 1;
		pb = ro - 1.;
	} else if( ro < 3 )
	{
		pr = 3. - ro;
		pb = 1;
	} else if( ro < 4 )
	{
		pb = 1;
		pg = ro - 3;
	} else if( ro < 5 )
	{
		pb = 5 - ro;
		pg = 1;
	} else
	{
		pg = 1;
		pr = ro - 5;
	}

	//Actually, above math is backwards, oops!
	pr *= value;
	pg *= value;
	pb *= value;

	avg += pr;
	avg += pg;
	avg += pb;

	pr = pr * sat + avg * (1.-sat);
	pg = pg * sat + avg * (1.-sat);
	pb = pb * sat + avg * (1.-sat);

	ora = pr * 255;
	og = pb * 255;
	ob = pg * 255;

	if( ora < 0 ) ora = 0;
	if( ora > 255 ) ora = 255;
	if( og < 0 ) og = 0;
	if( og > 255 ) og = 255;
	if( ob < 0 ) ob = 0;
	if( ob > 255 ) ob = 255;

	return (ob<<16) | (og<<8) | ora;
}

int main( int argc, char ** argv )
{
	int firstoverride = -1;
	if( argc < 2 )
	{
		fprintf( stderr, "Error: usage: [tool] [ip address] [optional: no of lights] [optional (if present, first override (i.e. white on some systems))]\n" );
		return -1;
	}
	uint32_t frame = 0;
	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(argv[1]);
	servaddr.sin_port=htons(7777);

	int lights = 186;

	if( argc >= 3 )
	{
		lights = atoi( argv[2] );
	}
	if( argc >= 4 )
	{
		firstoverride = atoi( argv[3] );
	}

	printf( "Lights: %d\n", lights );

	while(1)
	{
//#define lights 186
		uint8_t buffer[lights*3];
		int i;
		for( i = 0; i < lights; i++ )
		{

//#define BEAT
			uint32_t hex;
			//For button
//			hex = (i == (frame % lights))?0xFFFFFF:0x000000;
//			hex=0xffffff;
//			hex = HSVtoHEX( i*(1/12.) + frame*(1./48.), 1, 1.0 );
//			hex = (((frame+i)%lights)>(lights-2))?0xffffff:0; //The worm.
			hex = HSVtoHEX( i*.03 - frame*.04, 1,  (((((-i<<3)%256) - ((frame<<3)%256)+256)%256) ) /256.0*0.9-0.1); //Long, smooth, transitioning. 1.0 


			//For wall art.

//			hex = 0x404040;

			hex = HSVtoHEX( i*.05 + frame*.01, 1, 1.0 ); //0.50 = overload. 0.45 = overheat? =0.40 = HOT

//			hex = (((frame+i)%186)>160)?0xff8f8f:0; //The worm.
//			hex = (((frame+i)%186)>130)?0x0000ff:0; //The red worm.
//			hex = HSVtoHEX( i*.005 - frame*.001, 1,  ((((-i%256) - ((frame>>1)%256)+256)%256) ) /256.0*1.2-0.1);
//			hex = HSVtoHEX( i*.00500 + ((int)(frame*0.42))*.17, 1, 0.40 ); //Fiesta

			//and my favorite:
//			hex = HSVtoHEX( i*.001 - frame*.001, 1,  ((((i%256) - ((frame>>1)%256)+256)%256) ) /256.0*1.5-0.1); //Long, smooth, transitioning. 1.0 overloads.  Trying 0.9. If 0.9 works, should back off.

//			hex = HSVtoHEX( i*0.005376344 - frame*.001, 1.3,  1.0); //Long, smooth, transitioning. full-brigth


			//Chaser
//			hex = HSVtoHEX( i*.002 + frame*.002, 4, 1.0 ); //0.50 = overload. 0.45 = overheat? =0.40 = HOT
//			hex = HSVtoHEX( frame*.001, 1.0,  1.0); //Long, smooth, transitioning. full-brigth

//			hex = 0x0000ff;

//			hex = HSVtoHEX( i * 0.0002+.3333, 1, 1.0 );


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
		printf( "." );
		fflush( stdout );
		usleep(14000);
	}
}

