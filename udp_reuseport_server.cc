#include <arpa/inet.h>
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <stdarg.h> //?
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::thread;
using std::vector;
#define BUFFER_SIZE 1024
string ip;
unsigned short port;

auto getTid() { return std::this_thread::get_id(); }

inline string ip2String(unsigned int ipv4) {
  char buf[INET_ADDRSTRLEN] = {0};
  struct in_addr in;
  in.s_addr = ipv4;
  if (inet_ntop(AF_INET, &in, buf, sizeof(buf)))
    return string(buf);
  return string{""};
}

void startUdpServer(string &ip, unsigned int port) {
  cout << "ip : " << ip << " ,port:  " << port << ", thread_id: " << getTid()
       << endl;

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
  server_addr.sin_port = htons(port);

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    cerr << "Create Socket Failed: " << strerror(errno) << endl;
    exit(1);
  }

  int opt_val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(opt_val));
  if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
    cerr << "Server Bind Failed, " << strerror(errno) << endl;
    exit(1);
  }

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);

    cout << "begin to receive data... " << getTid() << endl;

    int res = recvfrom(fd, buffer, BUFFER_SIZE, 0,
                       (struct sockaddr *)&client_addr, &client_addr_len);
    if (res < 0) {
      cerr << "Server Receive Faild" << strerror(errno) << endl;
      exit(1);
    }
    cout << getTid() << " recv " << res << " bytes from"
         << ip2String(client_addr.sin_addr.s_addr) << ": "
         << server_addr.sin_port << " " << getTid() << endl;
    res = sendto(fd, buffer, res, 0, (struct sockaddr *)&client_addr,
                 sizeof(client_addr));
    if (res < 0) {
      cerr << "Server SendTo Faild" << strerror(errno) << endl;
      exit(1);
    }
    cout << "response: " << res << "bytes to client"
         << " " << getTid() << endl;
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    cout << "Usage: " << argv[0] << " <local_ip> <udp_port> <thread_count>"
         << endl;
    exit(0);
  }
  ip = argv[1];
  port = atoi(argv[2]);
  int thread_count = atoi(argv[3]);
  cout << "ip: " << ip << " port: " << port << " thread_cout: " << thread_count
       << endl;
  vector<thread> vt;
  for (auto i = 0; i < thread_count; i++) {
    vt.push_back(
        std::move(thread(startUdpServer, std::ref(ip), std::ref(port))));
  }
  cout << "thread vector size: " << vt.size() << endl;
  for (auto i = 0; i < thread_count; i++) {
    vt[i].join();
  }
  return 0;
}