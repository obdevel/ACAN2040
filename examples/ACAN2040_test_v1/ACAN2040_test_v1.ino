// Pico CAN test

#include <ACAN2040.h>

// constants
const uint8_t PIONUM = 0;
const uint8_t TXPIN = 1;
const uint8_t RXPIN = 2;
const uint32_t BITRATE = 125000UL;
const uint32_t SYSCLK = F_CPU;

// forward function definitions
void my_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg);
char *msg_to_str(struct can2040_msg *msg);

// global variables
ACAN2040 can2040(PIONUM, TXPIN, RXPIN, BITRATE, SYSCLK, my_cb);
volatile bool cb_called = false, got_msg = false, msg_sent_ok = false, got_error = false, unknown_cb = false;
struct can2040_msg tx_msg, rx_msg;

///

void setup() {

  Serial.begin(115200);
  while (!Serial);
  Serial.printf("\n\nACAN2040 test, bitrate = %lu kbps, syscl = %lu mhz\n", BITRATE, SYSCLK);

  // create a test message
  tx_msg.id = 12345;
  tx_msg.dlc = 8;
  for (uint8_t i = 0; i < 8; i++) {
    tx_msg.data[i] = i + 90;
  }

  // start CAN
  Serial.printf("starting CAN bus\n");
  can2040.begin();

  Serial.printf("setup complete, free memory = %lu bytes\n\n", rp2040.getFreeHeap());
  Serial.printf("type 's' to send a test message\n\n");
}

///

void loop() {

  // callback called
  if (cb_called) {
    cb_called = false;
    Serial.printf("callback called\n");

    // received message
    if (got_msg) {
      got_msg = false;
      Serial.printf("cb: received msg: %s\n", msg_to_str(&rx_msg));
    }

    // message sent ok
    if (msg_sent_ok) {
      msg_sent_ok = false;
      Serial.printf("cb: message sent ok\n");
    }

    // error
    if (got_error) {
      got_error = false;
      Serial.printf("cb: an error occurred\n");
    }

    // unknown
    if (unknown_cb) {
      unknown_cb = false;
      Serial.printf("cb: unknown notification\n");
    }
  }

  // send a test message
  if (Serial.available()) {
    char c = Serial.read();

    if (c == 's') {
      Serial.printf("sending message: %s ... ", msg_to_str(&tx_msg));

      if (can2040.ok_to_send()) {
        if (can2040.send_message(&tx_msg)) {
          Serial.printf("ok\n");
        } else {
          Serial.printf("error sending message\n");
        }
      } else {
        Serial.printf("no space available to send message\n");
      }
    }
  }

}

/// notify callback - runs in interrupt context so must be very short !

void my_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg) {

  (void)(cd);
  cb_called = true;

  switch (notify) {
    case CAN2040_NOTIFY_RX:
      rx_msg = *msg;
      got_msg = true;
      break;
    case CAN2040_NOTIFY_TX:
      msg_sent_ok = true;
      break;
    case CAN2040_NOTIFY_ERROR:
      got_error = true;
      break;
    default:
      unknown_cb = true;
      break;
  }

  return;
}

/// format CAN message as a string

char *msg_to_str(struct can2040_msg *msg) {

  static char buf[64], buf2[8];

  sprintf(buf, "[ %lu ] [ %lu ] [ ", msg->id, msg->dlc);
  for (uint32_t i = 0; i < msg->dlc && i < 8; i++) {
    sprintf(buf2, "%u ", msg->data[i]);
    strcat(buf, buf2);
  }
  strcat(buf, " ] ");

  if (msg->id & CAN2040_ID_RTR) {
    strcat(buf, "R");
  }

  if (msg->id & CAN2040_ID_EFF) {
    strcat(buf, "X");
  }

  return buf;
}
