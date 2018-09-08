#include "SessUDP.h"
#include "KcpuvTest.h"
#include <cstring>

namespace kcpuv_test {

using namespace std;
using namespace kcpuv;

class SessUDPTest : public testing::Test {
protected:
  SessUDPTest(){};
  virtual ~SessUDPTest(){};
};

static uv_loop_t *loop;
static void emptyDgramCb(SessUDP *udp, const struct sockaddr *addr,
                         const char *data, int len) {
  Loop::CloseLoopHandles_(loop);
}
// static void emptyTimerCb(uv_timer_t *timer) {
//   SessUDP *udp = reinterpret_cast<SessUDP *>(timer->data);
//   udp->Unbind();
// }

// NOTE: Libuv may failed to trigger some callbacks if we don't actually send
// msgs.
TEST_F(SessUDPTest, DeleteSessUDPTest) {
  loop = new uv_loop_t;
  uv_loop_init(loop);

  SessUDP *udp = new SessUDP(loop);
  char addr[17];
  int namelength = 0;
  int port = 0;

  udp->Bind(0, emptyDgramCb);

  udp->GetAddressPort(&namelength, addr, &port);
  udp->SetSendAddr("127.0.0.1", port);
  udp->Send("Hello", 5);

  // NOTE: For valgrind, make uv loop exit automatically.
  uv_run(loop, UV_RUN_DEFAULT);
  fprintf(stderr, "alive: %d\n", uv_loop_alive(loop));
  Loop::CloseLoopHandles_(loop);
  int rval = uv_loop_close(loop);

  fprintf(stderr, "rval: %d\n", rval);

  delete udp;
  delete loop;
}

static void dgramCb(SessUDP *udp, const struct sockaddr *addr, const char *data,
                    int len) {

  if (len > 0) {
    ASSERT_STREQ(data, "Hello");
  } else {
    Loop::CloseLoopHandles_(loop);
  }
}

TEST_F(SessUDPTest, BindAndSend) {
  loop = new uv_loop_t;
  uv_loop_init(loop);

  SessUDP *udp = new SessUDP(loop);

  char addr[17];
  int port = 0;
  int namelength = 0;
  int rval;
  char localaddr[] = "0.0.0.0";
  char msg[] = "Hello";

  // Bind port A.
  rval = udp->Bind(0, dgramCb);

  if (rval) {
    fprintf(stderr, "%s\n", uv_strerror(rval));
  }

  EXPECT_GE(rval, 0);
  // Get sockname of A.

  udp->GetAddressPort(&namelength, addr, &port);
  EXPECT_GT(port, 0);

  // Create port b and send a message to port A.
  udp->SetSendAddr(localaddr, port);
  udp->Send(msg, sizeof(msg));

  uv_run(loop, UV_RUN_DEFAULT);
  Loop::CloseLoopHandles_(loop);
  rval = uv_loop_close(loop);

  assert(!rval);

  delete udp;
  delete loop;
}

TEST_F(SessUDPTest, HasSendAddr) {
  loop = new uv_loop_t;
  uv_loop_init(loop);

  SessUDP *sessHasAddr = new SessUDP(loop);
  SessUDP *sessDoesntHasAddr = new SessUDP(loop);

  char addr[] = "127.0.0.1";
  char data[] = "Hello";
  char addrname[17];
  int namelength = 0;
  int port = 0;

  sessHasAddr->Bind(0, emptyDgramCb);
  sessHasAddr->GetAddressPort(&namelength, addrname, &port);
  sessHasAddr->SetSendAddr("127.0.0.1", port);
  sessHasAddr->Send("Hello", 5);

  EXPECT_EQ(sessHasAddr->HasSendAddr(), 1);
  EXPECT_EQ(sessDoesntHasAddr->HasSendAddr(), 0);

  uv_run(loop, UV_RUN_DEFAULT);
  Loop::CloseLoopHandles_(loop);
  int rval = uv_loop_close(loop);

  delete sessHasAddr;
  delete sessDoesntHasAddr;
  delete loop;
}

TEST_F(SessUDPTest, SetSendAddrBySockaddr) {
  loop = new uv_loop_t;
  uv_loop_init(loop);

  SessUDP *udp = new SessUDP(loop);
  udp->data = loop;

  char addrstr[17];
  int port = 12345;
  int namelength = 0;
  int rval;
  char localaddr[] = "0.0.0.0";
  char msg[] = "Hello";
  struct sockaddr *addr = new struct sockaddr;

  // Bind port A.
  rval = udp->Bind(0, dgramCb);
  udp->GetAddressPort(&namelength, addrstr, &port);

  uv_ip4_addr(localaddr, port, reinterpret_cast<struct sockaddr_in *>(addr));

  if (rval) {
    fprintf(stderr, "%s\n", uv_strerror(rval));
  }
  EXPECT_GT(rval, -1);

  // Create port b and send a message to port A.
  udp->SetSendAddrBySockaddr(addr);
  udp->Send(msg, sizeof(msg));

  uv_run(loop, UV_RUN_DEFAULT);
  Loop::CloseLoopHandles_(loop);
  uv_loop_close(loop);

  delete udp;
  delete addr;
  delete loop;
}

} // namespace kcpuv_test
