#include "netcon.h"


static int debug=0;

#define SELECT_USE_FDSET 1

/***/

#if (SELECT_USE_FDSET + SELECT_USE_INT_P) > 1
 error must define only one of SELECT_USE_FDSET or SELECT_USE_INT_P
#endif

#if (SELECT_USE_FDSET + SELECT_USE_INT_P) < 1
 error must define either SELECT_USE_FDSET or SELECT_USE_INT_P
#endif

/***/

#if SELECT_USE_FDSET
   typedef fd_set FD_SET;
#endif

#if SELECT_USE_INT_P
   typedef struct fd_set FD_SET;
#endif

/***/



@implementation netcon : Object

/* these are private methods that shouldn't be called by anything
   but this object.  They are not listed in the interface file.

- (int) pick_new_channel;
- (int) stream_send : (int) channel : (char *) send_buffer : (int) length;
- (int) stream_read : (int) channel : (char *) buf : (int) length;
- (int) swap_you_know_me_as : (int) new_channel;
- (void) hang_up : (int) channel;

*/

- (int) stream_send : (int) channel : (char *) send_buffer : (int) length
{
    int number_sent = 0;
    int write_return;
    struct timeval wait;
    FD_SET ready;
    FD_SET err;


    while (number_sent < length)
    {
      /*** make sure the remote process hasn't hung up... ***/
      FD_ZERO (&ready);
      FD_ZERO (&err);
      FD_SET (stream_sockets[ channel ], &ready);
      FD_SET (stream_sockets[ channel ], &err);

      wait.tv_sec = 0;
      wait.tv_usec = 0;

#     if SELECT_USE_FDSET
      if (select (max_socket, 0, &ready, &err, &wait) > 0)
#     else
      if (select (max_socket, 0, (int *) &ready, (int*) &err, &wait) > 0)
#     endif
	{
	  if (FD_ISSET (stream_sockets[ channel ], &err))
	    {
	      /* there is an error condition on this socket... close it! */
	      [self close_channel : channel];
	      [self handle_gone_connection : channel];
	      return -1;
	    }
	}

      if (!FD_ISSET (stream_sockets[ channel ], &ready))
	{
	  /* socket isn't ready? */
	  if (debug)
	    printf ("channel %d isn't ready?  closing it.\n", channel);
	  [self close_channel : channel];
	  [self handle_gone_connection : channel];
	  return -1;
	}

      /* socket is ok... go ahead and write. */

      write_return = write (stream_sockets[ channel ],
			    &send_buffer[ number_sent ],
			    length - number_sent);

      if (write_return < 0)
	{
	  /*** close the channel here? ***/
	  /*** should check on the error. ***/
	  perror ("write");
	  [self close_channel : channel];
	  [self handle_gone_connection : channel];
	  return -1;
	}
      
      number_sent += write_return;
    }

    return number_sent;
}


- (int) stream_read : (int) channel : (char *) buf : (int) length
{
    /* peek at first character to get length, if there
       are not enough characters ready, don't read from it? */
    /*** read the first character, then read the correct
      amount. ***/

    int number_read = 0;
    int read_return_value;


    while (number_read < length)
    {
	read_return_value = read (stream_sockets[ channel ],
				  &buf[ number_read ],
				  length - number_read);

	if (read_return_value < 0)
	{
	  if (debug)
	    perror ("read from stream socket");
	}

	if (read_return_value <= 0)
	{
	    [self close_channel : channel];
	    [self handle_gone_connection : channel];
	    return -1;
	}

	number_read += read_return_value;
    }

    return length;
}


- (void) hang_up : (int) channel /* be rude, then close channel */
{
  char *message = "hi! what's your name?\n";

  [self stream_send : channel : message : strlen (message)];
  [self close_channel : channel];
}


- (int) swap_you_know_me_as : (int) new_channel
{
    char buf[4];
    unsigned char cint[2];
    unsigned short int other_dgram_port;

    /*** perhaps 2 bytes should be used to send the channel?
      this limits things to 256 channels... ***/

    buf[0] = 126;
    buf[1] = '%'; /* magic chars */
    [self stream_send : new_channel : buf : 2];

    buf[0] = (unsigned char) new_channel;
    [self stream_send : new_channel : buf : 1];

    /*conv.i = htons (dgram_listen_port);*/
    cint[0] = dgram_listen_port & 0xFF;
    cint[1] = (dgram_listen_port & 0xFF00) >> 8;
    [self stream_send : new_channel : cint : 2];

    /* check magic... */
    [self stream_read : new_channel : buf : 1];
    if (buf[ 0 ] != 126)
      {
	[self hang_up : new_channel];
	return -1;
      }
    [self stream_read : new_channel : buf : 1];
    if (buf[ 0 ] != '%')
      {
	[self hang_up : new_channel];
	return -1;
      }

    /* this limits us to 256 channels... (one byte)*/
    [self stream_read : new_channel : buf : 1];
    you_know_me_as[ new_channel ] = (int) ( buf[0]);

    [self stream_read : new_channel : cint : 2];
    other_dgram_port = (cint[1] << 8) | cint[0];
    /*other_dgram_port = ntohs (conv.i);*/

    addresses[ new_channel ]->sin_family = AF_INET;
    addresses[ new_channel ]->sin_port = htons ( other_dgram_port);

    if (debug)
      printf ("other (%d) knows me as channel %d, and has dgram port %d\n",
	     new_channel,
	     you_know_me_as[ new_channel ],
	     other_dgram_port);

    return new_channel;
}


- (int) pick_new_channel
{
    int lop;

    for (lop=0; lop<max_others; lop++)
    {
	if (addresses[lop] == NULL)
	{
	    if (lop >= number_of_others)
		number_of_others = lop + 1;
	    return lop;
	}
    }
    
    addresses = (struct sockaddr_in **)realloc (addresses,
					       sizeof (struct sockaddr_in *) *
					       max_others * 2);
    stream_sockets = (int *)realloc (stream_sockets,
				    sizeof (int) * max_others * 2);
    you_know_me_as = (int *)realloc (you_know_me_as,
				    sizeof (int) * max_others * 2);


    if (addresses == NULL ||
	stream_sockets == NULL ||
	you_know_me_as == NULL)
    {
	perror ("realloc");
	return -1;
    }

    
    for (lop=max_others; lop<max_others*2; lop++)
	addresses[lop] = NULL;
    
    number_of_others = max_others + 1;
    max_others *= 2;
    return (max_others / 2);
}


- init : (int) listen_port
{
    int address_length;
    int lop;

    /******************************/

    stream_listen_socket = socket (AF_INET, SOCK_STREAM, 0);
    if (stream_listen_socket < 0)
    {
      if (debug)
	perror ("opening stream listen socket");
      return NULL;
    }

    max_socket = stream_listen_socket + 1;

    dgram_listen_socket = socket (AF_INET, SOCK_DGRAM, 0);
    if (dgram_listen_socket < 0)
    {
      if (debug)
	perror ("opening dgram listen socket");
      close (stream_listen_socket);
      return NULL;
    }

    if (dgram_listen_socket >= max_socket)
	max_socket = dgram_listen_socket + 1;

    /******************************/

    stream_listen_address.sin_family = AF_INET;
    stream_listen_address.sin_addr.s_addr = INADDR_ANY;
    stream_listen_address.sin_port = htons (listen_port);

    if (bind (stream_listen_socket,
	      (struct sockaddr *) (&stream_listen_address),
	      sizeof (stream_listen_address)))
    {
      if (debug)
	perror ("bind of stream listen socket");
      close (stream_listen_socket);
      close (dgram_listen_socket);
      return NULL;
    }

    /* this could be changed in the future... if the bind
       fails, try different ports? */
    /* if the requested stream port isn't available, print
       a warning message, but don't worry too much about
       the dgram listen port being changed... after the
       stream sockets are hooked up, they exchange info
       on how to reach each others dgram ports */

    stream_listen_port = listen_port;



    dgram_listen_address.sin_family = AF_INET;
    dgram_listen_address.sin_addr.s_addr = INADDR_ANY;
    dgram_listen_address.sin_port = htons ( listen_port);

    dgram_listen_port = listen_port;

    while (bind (dgram_listen_socket,
	     (struct sockaddr *) (&dgram_listen_address),
	     sizeof (dgram_listen_address)))
    {
	fprintf (stderr,"bind of dgram failed on port %d\n",
		dgram_listen_port);
	dgram_listen_port++;
	dgram_listen_address.sin_port = htons ( dgram_listen_port);

	if (dgram_listen_port > listen_port + 100)
	{
	    fprintf (stderr,"giving up.\n");
	    return NULL;
	}
    }

    /******************************/

    address_length = sizeof (stream_listen_address);  

    if (getsockname (stream_listen_socket,
		    (struct sockaddr *) (&stream_listen_address),
		    &address_length))
    {
      if (debug)
	perror ("getsockname on stream listen socket");
      close (stream_listen_socket);
      close (dgram_listen_socket);
      return NULL;
    }

    if (debug)
      printf ("stream listen socket on port %d\n",
	     ntohs (stream_listen_address.sin_port));


    listen (stream_listen_socket, 5);

    number_of_others = 0;
    max_others = 10;

    addresses = (struct sockaddr_in **)
	malloc (sizeof (struct sockaddr_in *) * max_others);
    stream_sockets = (int *) malloc ( sizeof (int) * max_others);
    you_know_me_as = (int *) malloc ( sizeof (int) * max_others);

    if (addresses == NULL ||
	stream_sockets == NULL ||
	you_know_me_as == NULL)
    {
	perror ("malloc");
	return NULL;
    }

    for (lop=0; lop<max_others; lop++)
	addresses[lop] = NULL;

    break_select=NULL;
    num_break_select=0;

    /* if a peer hangs up, don't kill this program.  The
       error will be noted, and the socket closed. */
#   if HAVE_SIGIGNORE
    sigignore (SIGPIPE);
#   elif HAVE_SIGNAL
    signal (SIGPIPE, SIG_IGN);
#   else
    fprintf (stderr, "warning -- no way to trap SIGPIPE.\n");
#   endif

    return self;
}


- (int) get_stream_file_d : (int) channel
{
    if (channel < 0 || channel > number_of_others)
    {
	fprintf (stderr,"netcon, cannot get file_d for channel %d\n",channel);
	return -1;
    }

    return stream_sockets[ channel ];
}

- (int) get_stream_listen_file_d
{
  return stream_listen_socket;
}

- (int) get_dgram_file_d
{
    return dgram_listen_socket;
}

- (void) add_break_select : (int) socket_int
{
    int lop;

    /* make sure this socket isn't allready listed... */

    for (lop=0; lop<num_break_select; lop++)
	if (break_select[lop] == socket_int)
	    return;

    num_break_select++;
    break_select = (int *)realloc (
				  break_select,
				  sizeof (int)*num_break_select
				 );

    break_select[num_break_select-1] = socket_int;
}


- (void) remove_break_select : (int) socket_int
{
    int lop;

    /* find where this int is stored */

    for (lop=0; lop<num_break_select; lop++)
	if (break_select[lop] == socket_int)
	    break;

    if (lop == num_break_select)
    {
	fprintf (stderr,"tried to remove a bogus break_select\n");
	return;
    }

    num_break_select--;
    for (; lop<num_break_select; lop++)
	break_select[lop] = break_select[lop+1];
}


- (void) catch : (long int) seconds : (long int) micro_seconds
{
    FD_SET ready;
    int other;
    struct timeval wait;
    int select_return_value;
    int new_channel;
    static char BUFFER[2048];
    int length;
    int address_length;
    unsigned char cint[2];



    FD_ZERO (&ready);
    FD_SET (stream_listen_socket, &ready);
    FD_SET (dgram_listen_socket, &ready);

    /* add the break_selects to the select list */
    for (other=0; other<num_break_select; other++)
	FD_SET (break_select[other], &ready);


    for (other=0; other<number_of_others; other++)
	if (addresses[other] != NULL)
	    FD_SET (stream_sockets[ other ], &ready);

    wait.tv_sec = seconds;
    wait.tv_usec = micro_seconds;


    if (seconds < 0 || micro_seconds < 0)
#       if SELECT_USE_FDSET
	select_return_value = select (max_socket, &ready, 0, 0, NULL);
#       endif
#       if SELECT_USE_INT_P
	select_return_value = select (max_socket, (int *)&ready, 0, 0, NULL);
#       endif
    else
#       if SELECT_USE_FDSET
	select_return_value = select (max_socket, &ready, 0, 0, &wait);
#       endif
#       if SELECT_USE_INT_P
	select_return_value = select (max_socket, (int *)&ready, 0, 0, &wait);
#       endif



    if (select_return_value < 0)
	perror ("select");

    if (select_return_value <= 0)
	return;


    /*** check the dgram listen socket ***/

    if (FD_ISSET (dgram_listen_socket, &ready))
    {
	length = read (dgram_listen_socket, BUFFER, 2048);

	if (length < 0)
	    perror ("read dgram socket");
	else
	    [self process_data : BUFFER[ 0 ] : &BUFFER[ 1 ] : length];
    }



    /*** check all the open stream channels ***/

    for (other=0; other<number_of_others; other++)
    {
	if (FD_ISSET (stream_sockets[other], &ready))
	{
	    if ( [self stream_read : other : cint : 2] < 0)
		continue;

	    length = (cint[1]<<8) | cint[0];

	    if ((length = [ self stream_read : other : BUFFER : length ]) > 0)
		[self process_data : other : BUFFER : length];
	}
    }


    /*** check the stream listen socket ***/

    if (FD_ISSET (stream_listen_socket, &ready))
    {
	new_channel = [self pick_new_channel];

	/*** fill in the address structure ***/
	addresses[ new_channel ] = (struct sockaddr_in *)
	    malloc (sizeof (struct sockaddr_in));

	address_length = sizeof (struct sockaddr_in);

	stream_sockets[ new_channel ] =
	    accept (stream_listen_socket,
		   (struct sockaddr *) addresses[ new_channel ],
		   &address_length);

	if (stream_sockets[ new_channel ] == (-1))
	{
	  if (debug)
	    perror ("accept of new stream connection");
	  [self close_channel : new_channel];
	}
	else
	{
	  if (stream_sockets[ new_channel ] >= max_socket)
	    max_socket = stream_sockets[ new_channel ] + 1;
	  
	  if ([self swap_you_know_me_as : new_channel] >= 0)
	    [self handle_new_connection : new_channel];
	}
    }
}


- (int) open_channel : (char *) remote_host_name : (int) remote_port
{
    int new_channel;
    struct hostent *remote_hostent;

    new_channel = [self pick_new_channel];

    remote_hostent = gethostbyname (remote_host_name);
    if (remote_hostent == NULL)
    {
	long addr = inet_addr (remote_host_name);
	if (addr < 0)
	{
	    perror ("inet_addr");
	    fprintf (stderr,"%s: unknown host?\n", remote_host_name);
	    return -1;
	}
	    
	remote_hostent =
	    gethostbyaddr ((char *)&addr, sizeof (addr), AF_INET);

	if (remote_hostent == NULL)
	{
	  /*herror ("gethostbyaddr");*/
	  fprintf (stderr,"%s: unknown host?\n", remote_host_name);
	  return -1;
	}
    }

    addresses[ new_channel ] = (struct sockaddr_in *)
	malloc ( sizeof (struct sockaddr_in));

    addresses[ new_channel ]->sin_family = AF_INET;

    bcopy (remote_hostent->h_addr_list[ 0 ],
	  (char *) (& (addresses[ new_channel ]->sin_addr)),
	  remote_hostent->h_length);

    addresses[ new_channel ]->sin_port = htons ( remote_port);


    stream_sockets[ new_channel ] = socket (AF_INET, SOCK_STREAM, 0);
    if (stream_sockets[ new_channel ] < 0)
    {
      if (debug)
	perror ("creation of stream socket during open channel");
      [self close_channel : new_channel];
      return -1;
    }

    if (stream_sockets[ new_channel ] >= max_socket)
	max_socket = stream_sockets[ new_channel ] + 1;

    if (connect (stream_sockets[ new_channel ],
		 (struct sockaddr *) ( addresses[ new_channel ]),
		sizeof ( *addresses[ new_channel ])) < 0)
    {
      if (debug)
	perror ("connect with stream socket");
      [self close_channel : new_channel];
      return -1;
    }

    if (debug)
      printf ("just opened a new channel (%d).\n", new_channel);

    if ([self swap_you_know_me_as : new_channel] < 0)
      return -1;

    /*[self handle_new_connection : new_channel];*/ /* ??? */

    return new_channel;
}


- (void) throw_to_all : (const unsigned char *) data
                      : (size_t) length
                      : (int) path
{
    int channel;

    for (channel=0; channel<number_of_others; channel++)
    {
	if (addresses[ channel ] != NULL)
	{
	    [self throw : channel
	                : data
	                : length
                        : path];
	}
    }
}


- (void) throw : (int) channel
               : (const unsigned char *) data
               : (size_t) length
               : (int) path /* dgram or stream */
{
    /* is 259 correct??? */

    static char send_buffer[259];


    if (addresses[ channel ] == NULL)
    {
	fprintf (stderr,"Tried to use channel %d, which isn't open.\n",
		channel);
	return;
    }

    if (path == 0)
    {
	/*** send it via a stream connection ***/
	if (length > 0xFFFF)
	{
	    fprintf (stderr, "SEND PACKET TOO LONG!!! (%d)\n", length);
	    return;
	}

	send_buffer[ 0 ] = length & 0x00FF;
	send_buffer[ 1 ] = (length & 0xFF00) >> 8;


	bcopy ((void *) data, (void *) (&send_buffer[ 2 ]), (int) length);
	length += 2;

	[self stream_send : channel : send_buffer : length];
    }
    else if (path == 1)
    {
	/*** sent is via datagram packet ***/
	send_buffer[ 0 ] = (char) (you_know_me_as[ channel ]);
	bcopy ((void *) data, (void *) (&send_buffer[ 1 ]), (int) length);
	length++;

	if (sendto (dgram_listen_socket,
		    send_buffer,
		    length,
		    0,
		    (struct sockaddr *)addresses[ channel ],
		    sizeof (struct sockaddr_in)) < 0)
	{
	    perror ("sending datagram");
	}
    }
    else
    {
	fprintf (stderr,"invalid path passed to throw: must be:\n");
	fprintf (stderr," 0 for stream\n");
	fprintf (stderr," 1 for datagram\n");
    }
}


- (void) close_channel : (int) channel
{
    if (addresses[ channel ] == NULL)
    {
	fprintf (stderr,"Tried to close channel %d, which isn't open.\n",
		channel);
	return;
    }

    if (debug)
      printf ("closing channel %d...\n", channel);

    close ( stream_sockets[ channel ]);
    free (addresses[ channel ]);
    addresses[ channel ] = NULL;
    /* [self handle_gone_connection : channel]; */ /* ??? */
}





- (int) throw_connectionless_dgram
    : (char *) remote_host_name
    : (unsigned short) port
    : (const unsigned char *) data
    : (size_t) length
{
    /* is 259 correct?  why 259? */
    static char send_buffer[259];
    struct sockaddr_in sa;
    struct hostent *remote_hostent;

    send_buffer[ 0 ] = (char) (-1); /* no return channel */
    bcopy ((void *) data, (void *) (&send_buffer[ 1 ]), (int) length);
    length++;


    remote_hostent = gethostbyname (remote_host_name);
    if (remote_hostent == NULL)
    {
	long addr = inet_addr (remote_host_name);
	if (addr < 0)
	{
	    perror ("inet_addr");
	    fprintf (stderr,"%s: unknown host?\n", remote_host_name);
	    return -1;
	}
	    
	remote_hostent =
	    gethostbyaddr ((char *)&addr, sizeof (addr), AF_INET);

	if (remote_hostent == NULL)
	{
	  /*herror ("gethostbyaddr");*/
	  fprintf (stderr,"%s: unknown host?\n", remote_host_name);
	  return -1;
	}
    }
    

    sa.sin_family = AF_INET;
    sa.sin_port = htons (port);
    bcopy (remote_hostent->h_addr_list[ 0 ],
	   (char *) (& (sa.sin_addr)),
	   remote_hostent->h_length);

    if (sendto (dgram_listen_socket,
		send_buffer,
		length,
		0,
		 (struct sockaddr *) &sa,
		sizeof (struct sockaddr_in)) < 0)
    {
	perror ("sending datagram");
    }

    return 0;
}


- (char *) get_local_ip_addr
{
  char hostname[ 128 ];
  struct hostent *local_hostent;

  gethostname (hostname, sizeof (hostname));

  local_hostent = gethostbyname (hostname);

  return inet_ntoa (*(struct in_addr *)(local_hostent->h_addr_list[ 0 ]));
}


- (char *) get_remote_ip_addr : (int) channel
{
  if (channel < 0) return "";
  if (channel >= max_others) return "";
  if (addresses[ channel ] == NULL) return "";

  /*char *inet_ntoa (const struct in_addr in);*/
  /*struct hostent *gethostbyname(const char *name);*/

#if 0
  struct hostent {
    char   *h_name;         /* canonical name of host */
    char   **h_aliases;     /* alias list */
    int    h_addrtype;      /* host address type */
    int    h_length;        /* length of address */
    char   **h_addr_list;   /* list of addresses */
  };

  struct sockaddr_in {
    short   sin_family;
    u_short sin_port;
    struct  in_addr sin_addr;
    char    sin_zero[8];
  };

  struct in_addr {
    union {
      struct { u_char s_b1, s_b2, s_b3, s_b4; } S_un_b;
      struct { u_short s_w1, s_w2; } S_un_w;
      u_long S_addr;
    } S_un;
#endif

  return inet_ntoa (addresses[ channel ]->sin_addr);
}


/*****************************************************
 these are intended to be overridded by a sub class.
 *****************************************************/

- (void) handle_new_connection : (int) new_channel
{
  if (debug)
    printf ("done catching inbound connection on channel %d\n", new_channel);
}

- (void) handle_gone_connection : (int) channel
{
  if (debug)
    printf ("channel %d has been closed remotely\n", channel);
}

- (void) process_data : (int) channel : (char *) data : (int) length
{
    data[length] = '\0';
    printf ("channel %d : %s\n", channel, data);
}


@end



void int_to_two_bytes (int i, char *ret)
{
  unsigned char a, b;

  a = (i & 0xFF00) >> 8;
  b = (i & 0xFF);

  ret[ 0 ] = (char) a;
  ret[ 1 ] = (char) b;
}


int two_bytes_to_int (char *bytes)
{
  int i;
  unsigned char a, b;

  a = ((unsigned char *) bytes)[ 0 ];
  b = ((unsigned char *) bytes)[ 1 ];

  i = (a << 8) | b;

  return i;
}


/* ripped off from sshd */

void close_controlling_terminal ()
{
  /* If not in debugging mode, disconnect from the controlling
     terminal, and fork.  The original process exits. */

  if (!debug)
    {
#     ifdef TIOCNOTTY
      int fd;
#     endif /* TIOCNOTTY */
      
      /* Fork, and have the parent exit.  The child becomes the server. */
      if (fork ())
        exit (0); 
 
      /* Redirect stdin, stdout, and stderr to /dev/null. */
	/*
      freopen ("/dev/null", "r", stdin);
      freopen ("/dev/null", "w", stdout);
      freopen ("/dev/null", "w", stderr);
	*/

      /* Disconnect from the controlling tty. */
#     ifdef TIOCNOTTY
      fd = open ("/dev/tty", O_RDWR|O_NOCTTY);
      if (fd >= 0)
        {
          (void)ioctl (fd, TIOCNOTTY, NULL);
          close (fd);
        }
#     endif /* TIOCNOTTY */
#     ifdef HAVE_SETSID
#     ifdef ultrix
      setpgrp (0, 0);
#     else /* ultrix */
      if (setsid () < 0)
	perror ("setsid");
#     endif /* ultrix */
#     endif /* HAVE_SETSID */
    }
}
