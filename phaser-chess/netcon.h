#ifndef __NETCON_INCLUDED
#define __NETCON_INCLUDED

#include "sys/time.h"
#include "unistd.h"
#include "sys/types.h"
#include "config.h"
#include "objc_inc.h"
#include "netdb.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"

void int_to_two_bytes (int i, char *ret);
int two_bytes_to_int (char *bytes);
void close_controlling_terminal ();


#define STREAM 0
#define DGRAM 1

@interface netcon : Object
{
  struct sockaddr_in stream_listen_address;
  int stream_listen_socket;
  unsigned short int stream_listen_port;

  struct sockaddr_in dgram_listen_address;
  int dgram_listen_socket;
  unsigned short int dgram_listen_port;

  struct sockaddr_in **addresses;

  int *stream_sockets;
  int *you_know_me_as;

  int number_of_others;
  int max_others;
  int max_socket;

  int *break_select;
  int num_break_select;
}


- init : (int) listen_port;
- (int) get_stream_file_d : (int) channel;
- (int) get_stream_listen_file_d;
- (int) get_dgram_file_d;
- (void) catch : (long int) seconds : (long int) micro_seconds;
- (int) open_channel : (char *) remote_host_name : (int) remote_port;
- (void) throw_to_all : (const unsigned char *) data
                      : (size_t) length
                      : (int) path;
- (void) throw : (int) channel
               : (const unsigned char *) data
               : (size_t) length
               : (int) path; /* 0 = stream, 1 = dgram */

- (int) throw_connectionless_dgram : (char *) remote_host_name
                                   : (unsigned short) port
                                   : (const unsigned char *) data
                                   : (size_t) length;


- (void) close_channel : (int) channel;

- (void) add_break_select : (int) socket_int;
- (void) remove_break_select : (int) socket_int;
- (char *) get_local_ip_addr;
- (char *) get_remote_ip_addr : (int) channel;

/* these should each be overridden with something useful */
- (void) handle_new_connection : (int) new_channel;
- (void) handle_gone_connection : (int) channel;
- (void) process_data : (int) channel : (char *) data : (int) length;


@end

#endif





/*
  this stuff was typed, but not read.  please excuse any spelling errors
  or typos or difficult sentence structures or annoying repetition or

  netcon is an object which hides the messy parts of socket
  connections.  It is almost entirely untested.

  Instead of dealing with sockets directly, you declare a
  netcon object, which provides 'channels'.  To start it up,
  do something like this:

  netcon *net;
  net = [[netcon alloc] init : 8000];

  this will try to open a stream listen socket on port 8000 of the
  local machine.  If there is some problem with the init, it will
  return NULL.  If the calling program detects this, it might be a
  good idea to change the listen port (8000) to something else and try
  again.  This would avoid problems with two different programs on the
  same machine running at the same time.

  At this point, the program can just wait for other programs to
  connect, or it can call netcon's open_channel method.

  Once a channel is open, information can be send across it by calling
  the 'throw' method.  It takes the channel number, a pointer to the
  data, the length of the data, and a path flag as arguments.  If path
  is 0, the data will be sent via a stream connection.  If path is 1,
  the data will be sent in a dgram packet.  Stream connections are
  reliable -- they will deliver all the data in the correct order with
  no duplication.  They are also fairly slow.  Sometimes its better to
  have speed instead of reliability.  In this case, use the dgram
  path.

  Periodically, the method 'catch' should be called.  If any data has
  been sent from a remote process to the local process, it will be
  dealt with in catch.  Catch takes care of any requests for new
  channels, it reads any data sent on existing channels, and it closes
  any channels it detects errors on.  If it gets any data, it will
  call the method 'process_data'.

  Process_data adds a NULL to the end of the data, and prints it to
  stdout.  This will not be very usefull for most programs.  I suggest
  that you create a new object with netcon as its superclass.  In the
  new object, override the method process_data with something that
  does what you want.

  various troubles:

  Data received from the net is stored in a static area, and must be
  copied if it is to be saved.

  If the bind to the requested port fails, it should most likely try
  different ports, but I have not added this yet.  This could be a
  problem if the program was a server type program, and other programs
  would need to know what port it was on.  It would, however, be ok to
  change the listen port if all this program did was to connect to
  some other program (rather than receiving connections itself).

  It would be nice if this program were more robust.  If something
  messed up comes along a stream connection, the program will lock up.
  This is because of the mechanism used to re-packatize the data being
  sent across the stream connection.  If one program makes two quick
  calls to write to a stream connection, there is a chance that the
  data given to both calls will be returned in one read call by the
  remote process.  For my purposes, this is undesirable.  To avoid
  this, I send the length of the the following data at the start of
  every call to write.  This allows the receiving program to know how
  much data to expect.  The drawback is: if someone simply telnets to
  the listen port of a server (or any program using this object) and
  types some random stuff, the server will lock up.  The server will
  get some key-presses, it will interpret them as the length of the
  following message, and will block on the read to the stream socket
  until it gets enough characters.  I haven't figured out a very
  pretty way around this yet.  Some possibilities: figure out a way to
  look ahead?  is there a way to know the number of characters waiting
  to be read? (some ioctl call?).  If that could be known, the server
  could avoid reading the data from the socket until it had all
  arrived.  This might cause problems with system buffers filling and
  the sending program being blocked.  Another (probably better)
  possiblity for avoiding the lock up problem: maintain a buffer of
  unresolved data for each stream connection.  Read what you can from
  the connection, if its not all there, don't block -- just go on.
  When more data arrives from the stream socket, append this data to the
  old buffer and give it to the program if enough has arrived.

  */  
