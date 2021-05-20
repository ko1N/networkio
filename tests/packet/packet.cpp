// test/packet

#include "../tests.h"

#include <networkio/networkio.h>

using namespace networkio;

BEGIN_TESTS("networkio", "packet")

ADD_TEST("uint8_t", {
  uint8_t a = 1;
  uint8_t b = 200;
  proto::packet pkt;
  pkt.write(a);
  pkt.write(b);
  pkt.write(a);
  TEST_EQUAL(a, pkt.read<uint8_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(b, pkt.read<uint8_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(a, pkt.read<uint8_t>());
  TEST_EQUAL(pkt.read_bytesleft(), 0);
});

ADD_TEST("uint32_t", {
  uint32_t a = 1;
  uint32_t b = 200;
  proto::packet pkt;
  pkt.write(a);
  pkt.write(b);
  pkt.write(a);
  TEST_EQUAL(a, pkt.read<uint32_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(b, pkt.read<uint32_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(a, pkt.read<uint32_t>());
  TEST_EQUAL(pkt.read_bytesleft(), 0);
});

ADD_TEST("float", {
  float a = 1.0f;
  float b = -100.0f;
  proto::packet pkt;
  pkt.write(a);
  pkt.write(b);
  pkt.write(a);
  TEST_EQUAL(a, pkt.read<float>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(b, pkt.read<float>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(a, pkt.read<float>());
  TEST_EQUAL(pkt.read_bytesleft(), 0);
});

ADD_TEST("strings", {
  std::string instr = "this is a test";
  proto::packet pkt;
  pkt.write_string(instr);
  std::string outstr = pkt.read_string();
  TEST_EQUAL(instr, outstr);
  TEST_EQUAL(pkt.read_bytesleft(), 0);
});

ADD_TEST("strings_with_null", {
  std::string instr = "this is\0a test";
  proto::packet pkt;
  pkt.write_string(instr);
  std::string outstr = pkt.read_string();
  TEST_EQUAL(instr, outstr);
  TEST_EQUAL(instr.length(), outstr.length());
  TEST_EQUAL(pkt.read_bytesleft(), 0);
});

ADD_TEST("begin_read", {
  uint32_t a = 100;
  uint32_t b = 200;
  proto::packet pkt;
  pkt.write(a);
  pkt.write(b);
  TEST_EQUAL(a, pkt.read<uint32_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(b, pkt.read<uint32_t>());
  TEST_EQUAL(pkt.read_bytesleft(), 0);
  pkt.begin_read();
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(a, pkt.read<uint32_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(b, pkt.read<uint32_t>());
  TEST_EQUAL(pkt.read_bytesleft(), 0);
});

ADD_TEST("set_start", {
  uint32_t a = 100;
  uint32_t b = 200;
  uint32_t c = 300;
  proto::packet pkt;
  pkt.write(a);
  pkt.write(b);
  pkt.write(c);
  TEST_EQUAL(a, pkt.read<uint32_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  pkt.set_start();
  TEST_EQUAL(b, pkt.read<uint32_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(c, pkt.read<uint32_t>());
  TEST_EQUAL(pkt.read_bytesleft(), 0);
  pkt.begin_read();
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(b, pkt.read<uint32_t>());
  TEST_NOT_EQUAL(pkt.read_bytesleft(), 0);
  TEST_EQUAL(c, pkt.read<uint32_t>());
  TEST_EQUAL(pkt.read_bytesleft(), 0);
});

END_TESTS("networkio")
