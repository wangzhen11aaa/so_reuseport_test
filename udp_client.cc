#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
#define SERVER_PORT 8000
#define BUFFER_SIZE 1024

inline unsigned IpStringToU32(const char *pAddr) {
  in_addr in = {0};
  if (inet_pton(AF_INET, pAddr, &in) > 0) {
    return in.s_addr;
  }
  return 0;
}

inline unsigned IpStringToU32(const string &strAddr) {
  return IpStringToU32(strAddr.c_str());
}

inline string IpU32ToString(unsigned ipv4) {
  char buf[INET_ADDRSTRLEN] = {0};
  struct in_addr in;
  in.s_addr = ipv4;

  if (inet_ntop(AF_INET, &in, buf, sizeof(buf))) {
    return string(buf);
  } else {
    return string("");
  }
}
int main(int argc, char **argv) {
  if (argc < 4) {
    cout << "Usage: " << argv[0] << " <server ip> <server port> <send_times>" << endl;
    exit(0);
  }
  string server_ip = argv[1];
  unsigned short port = atoi(argv[2]);
  unsigned short send_cnt = atoi(argv[3]);
  cout << "server_ip: " << server_ip << " " << port << endl;

  struct sockaddr_in server_addr, client_addr;

  socklen_t client_addr_length = sizeof(client_addr);
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
  server_addr.sin_port = htons(port);

  int client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (client_socket_fd < 0) {
    perror("Create Socket Failed:");
    exit(1);
  }

  char buf[BUFFER_SIZE];
  int len = BUFFER_SIZE;
  int i = 1;
  int max = send_cnt;
  int cnt = 0;
  while (i++ < max)
  {
	  cout << " send to " << IpU32ToString(server_addr.sin_addr.s_addr)
		   << " port " << ntohs(server_addr.sin_port) << endl;
	  int res = sendto(client_socket_fd, buf, len, MSG_DONTWAIT,
					   (struct sockaddr *)&server_addr, sizeof(server_addr));
	  if (res < 0)
	  {
		  perror("send to relay fail:");
		  exit(1);
	  }
	  cout << "send " << res << " bytes to relay server "
		   << IpU32ToString(server_addr.sin_addr.s_addr) << " "
		   << ntohs(server_addr.sin_port) << "waiting for response..." << endl;

	  res = recvfrom(client_socket_fd, buf, BUFFER_SIZE, 0,
					 (struct sockaddr *)&client_addr, &client_addr_length);
	  if (res < 0)
	  {
		  perror("Receive Data Failed:");
		  exit(1);
	  }
	  cnt++;
	  cout << "recv form" << IpU32ToString(client_addr.sin_addr.s_addr) << " "
		   << client_addr.sin_port << ". data len: " << res << endl;
  }
  printf("cnt: %d \n", cnt);
  close(client_socket_fd);
  return 0;
}