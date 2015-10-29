#include <iostream>

#include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <sys/ioctl.h>
// #include <net/if.h>
// #include <ifaddrs.h>
// #include <netdb.h>
// #include <unistd.h>
// #include <rdma/rdma_cma.h>
// #include <sys/time.h>
// #include <poll.h>

// #include <stdint.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/stat.h>

// #include <rdma/rdma_cma.h>
// #include <rdma/rdma_verbs.h>

#include <deque>

// #include "mpi.h"

// char* ip_search(void)
// {
//   struct sockaddr_in address;
//   memset(&address, 0, sizeof(address));

//   struct ifaddrs *addrs;
//   getifaddrs(&addrs);
//   struct ifaddrs *head = addrs;

//   int count =0;

//   for (; head != NULL; head = head->ifa_next) {
//           if (head->ifa_name == NULL) {
//                   break;
//     }
//     count++;
//   }

//   int sfd, intr;
//   struct ifreq buf[16];
//   struct ifconf ifc;
//   sfd = socket(AF_INET, SOCK_DGRAM, 0);
//   if(sfd < 0)
//     return NULL;
//   ifc.ifc_len = sizeof(buf);
//   ifc.ifc_buf = (caddr_t) buf;
//   if(ioctl(sfd, SIOCGIFCONF, (char *) &ifc))
//     return NULL;
//   intr = ifc.ifc_len / sizeof(struct ifreq);
//   while(intr-- > 0 && ioctl(sfd, SIOCGIFADDR, (char *) &buf[intr]));
//   close(sfd);
  
//   int _count = 0;
//   int i = 0;
//   for (; i < count; i++) {
//     if (!strcmp(buf[i].ifr_name, "ib0") )
//       _count = i;
//     std::cout << "intf= " << buf[i].ifr_name << "\n"
//               << "ip=" << inet_ntoa(((struct sockaddr_in *) (&buf[i].ifr_addr))->sin_addr) << "\n\n";
//   }
  
//   return inet_ntoa(((struct sockaddr_in *) (&buf[_count].ifr_addr))->sin_addr);
// }

int main()
{
  // char* ip = ip_search();
  // std::cout << "main:: ip= " << ip << "\n";
  
  std::deque<std::string> dq;
  dq.push_back("dummy_00");
  dq.push_back("dummy_10");
  dq.push_back("dummy_01");
  dq.push_back("dummy_11");
  
  std::cout << "dq= \n";
  for (std::deque<std::string>::iterator it = dq.begin(); it != dq.end(); it++)
    std::cout << *it << "\n";
  
  return 0;
}
