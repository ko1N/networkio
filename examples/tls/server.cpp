
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <networkio/networkio.h>
#include <thread>
#include <time.h>

#include "hostinfo.h"

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------

#define HOST_PORT 14447
const std::string certificate = HOST_CLIENT_CERT;
const std::string privatekey = HOST_PRIVATE_KEY;

//----------------------------------------------------------------------------
// client thread
//----------------------------------------------------------------------------

void client_thread(std::shared_ptr<networkio::interfaces::client> client) {
  clock_t start = clock();
  uint64_t rcvd = 0;
  uint64_t s = 0;

  auto cl_proto = std::make_shared<networkio::proto::packet_proto>(client);

  while (true) {
    cl_proto->process();

    while (true) {
      auto packet = cl_proto->read_packet();
      if (packet == nullptr) {
        break;
      }

      // add rcvd length
      rcvd += (uint64_t)packet->size();

      // print out stats
      if (s % 5000 == 0) {
        float mbps = (float)(rcvd / 1000000) /
                     ((double)(clock() - start) / CLOCKS_PER_SEC);
        printf("received packet #%d with length %d. received %dmb with %2.2f "
               "mb/s\n",
               (uint32_t)s, packet->size(), (uint32_t)(rcvd / 1000000), mbps);
      }

      cl_proto->write_packet(packet);
      s++;
    }

    // Sleep(0);
  }

  // g_cLog.Printf(CLog::LOG_VERBOSE, "client from %s disconnected.",
  // inet_ntoa(sClient.sAddress.sin_addr));
}

//----------------------------------------------------------------------------
// entry point
//----------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  networkio::tls::server server;

  server.set_certificate(certificate, privatekey, HOST_PRIVATE_KEY_PASSWORD);
  server.set_buffer_size(DEFAULT_PACKET_SIZE);
  server.set_blocking(false);

  while (!server.bind(HOST_PORT)) {
    networkio::sleep(5000);
  }
  printf("server socket bound to port %d.\n", HOST_PORT);

  while (true) {
    struct sockaddr_in clientaddr;
    auto client = server.accept(&clientaddr);
    if (client != nullptr) {
      std::thread thread(client_thread, client);
      thread.detach();
    }

    networkio::sleep(100);
  }

  getchar();
  return 0;
}
