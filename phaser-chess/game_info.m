#include "config.h"
#include "game_info.h"
#include "netcon.h"
#include "enums.h"


static int debug=0;

game_info *new_game_info (char *uname,
			  char *host,
			  char *domain,
			  char *ip,
			  int port,
			  int num_players)
{
  game_info *gi = (game_info *) malloc (sizeof (game_info));

  gi->uname = (char *) malloc (strlen (uname) + 1);
  strcpy (gi->uname, uname);
  gi->host = (char *) malloc (strlen (host) + 1);
  strcpy (gi->host, host);
  gi->domain = (char *) malloc (strlen (domain) + 1);
  strcpy (gi->domain, domain);
  gi->ip = (char *) malloc (strlen (ip) + 1);
  strcpy (gi->ip, ip);
  gi->port = port;
  gi->num_players = num_players;
  gi->last_report = 0;

  return gi;
}


void delete_game_info (game_info *gi)
{
  if (gi == NULL)
    return;

  free (gi->uname);
  free (gi->host);
  free (gi->domain);
  free (gi->ip);
  free (gi);
}


char *encode_game_info (int pc, game_info *gi, int *len)
{
  char *buf;
  char *b;

  buf = (char *) malloc (2 + /* pc_im_a_server */
			 2 + strlen (gi->uname) +
			 2 + strlen (gi->host) +
			 2 + strlen (gi->domain) +
			 2 + strlen (gi->ip) +
			 2 + /* num players */
			 2   /* stream_listen_port */);

  b = buf;

  int_to_two_bytes (pc, b); b += 2;
  int_to_two_bytes (strlen (gi->uname), b); b += 2;
  strncpy (b, gi->uname, strlen (gi->uname)); b += strlen (gi->uname);
  int_to_two_bytes (strlen (gi->host), b); b += 2;
  strncpy (b, gi->host, strlen (gi->host)); b += strlen (gi->host);
  int_to_two_bytes (strlen (gi->domain), b); b += 2;
  strncpy (b, gi->domain, strlen (gi->domain)); b += strlen (gi->domain);
  int_to_two_bytes (strlen (gi->ip), b); b += 2;
  strncpy (b, gi->ip, strlen (gi->ip)); b += strlen (gi->ip);
  int_to_two_bytes (gi->num_players, b); b += 2;
  int_to_two_bytes (gi->port, b); b += 2;

  (*len) = b - buf;

  return buf;
}


game_info *decode_game_info (char *buf)
{
  game_info *gi;
  char *b = buf;
  char *uname; int uname_len;
  char *host; int host_len;
  char *domain; int domain_len;
  char *ip; int ip_len;
  int port;
  int num_players;


  /*b += 2;*/ /* skip the message */

  uname_len = two_bytes_to_int (b);  b += 2;
  uname = (char *) malloc (uname_len + 1);
  strncpy (uname, b, uname_len); uname[ uname_len ] = '\0';
  b += uname_len;

  host_len = two_bytes_to_int (b); b += 2;
  host = (char *) malloc (host_len + 1);
  strncpy (host, b, host_len); host[ host_len ] = '\0';
  b += host_len;

  domain_len = two_bytes_to_int (b); b += 2;
  domain = (char *) malloc (domain_len + 1);
  strncpy (domain, b, domain_len); domain[ domain_len ] = '\0';
  b += domain_len;

  ip_len = two_bytes_to_int (b); b += 2;
  ip = (char *) malloc (ip_len + 1);
  strncpy (ip, b, ip_len); ip[ ip_len ] = '\0';
  b += ip_len;

  num_players = two_bytes_to_int (b); b += 2;

  port = two_bytes_to_int (b); b += 2;


  if (debug)
    printf ("decode_game_info got: %s@%s.%s(%s):%d, %d players\n",
	    uname,
	    host,
	    domain,
	    ip,
	    port,
	    num_players);

  gi = new_game_info (uname, host, domain, ip, port, num_players);

  free (uname);
  free (host);
  free (domain);
  free (ip);

  return gi;
}
