
#include <time.h>

typedef struct
{
  char *uname;
  char *host;
  char *domain;
  char *ip;
  int port;
  int num_players;
  time_t last_report;
} game_info;


game_info *new_game_info (char *uname,
			  char *host,
			  char *domain,
			  char *ip,
			  int port,
			  int num_players);
void delete_game_info (game_info *gi);
char *encode_game_info (int pc, game_info *gi, int *len);
game_info *decode_game_info (char *buf);
