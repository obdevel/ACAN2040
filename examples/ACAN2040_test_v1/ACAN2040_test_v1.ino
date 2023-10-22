//
/// Pico CAN test using can2040
//

#include <ACAN2040.h>

const uint8_t PIONUM0 = 0;
const uint8_t TXPIN0 = 1;
const uint8_t RXPIN0 = 2;
const uint32_t BITRATE0 = 125000UL;
const uint32_t SYSCLK = F_CPU;

void my_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg);
char *msg_to_str(struct can2040_msg *msg);

ACAN2040 can2040(PIONUM0, TXPIN0, RXPIN0, BITRATE0, SYSCLK, my_cb);
bool got_msg = false;
struct can2040_msg tx_msg, rx_msg;
struct can2040_stats can_stats;

///

void setup() {

  // start serial and wait for connection to serial monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.printf("\n\nACAN2040 test, bitrate = %lu kbps, syscl = %lu mhz\n", BITRATE0, SYSCLK);

  // create a test CAN message
  tx_msg.id = 12345;
  tx_msg.dlc = 8;
  for (uint8_t i = 0; i < 8; i++) {
    tx_msg.data[i] = i + 90;
  }

  // start CAN
  Serial.printf("starting CAN bus\n");
  can2040.begin();

  Serial.printf("setup complete, free memory = %u bytes\n\n", rp2040.getFreeHeap());
  Serial.printf("type 's' to send a test message\n");
  Serial.printf("type 't' to stop CAN\n");
  Serial.printf("type 'b' to restart CAN\n");
  Serial.printf("type 't' for CAN statistics\n\n");
}

///

void loop() {

  // print received message
  if (got_msg) {
    got_msg = false;
    Serial.printf("received msg: %s\n", msg_to_str(&rx_msg));
  }

  // send a test message
  if (Serial.available()) {
    char c = Serial.read();

    switch (c) {
    case 's':
      Serial.printf("sending message: %s ... ", msg_to_str(&tx_msg));

      if (can2040.ok_to_send()) {
        if (can2040.send_message(&tx_msg)) {
          Serial.printf("ok\n");
        } else {
          Serial.printf("** error sending message\n");
        }
      } else {
        Serial.printf("** no space available to send\n");
      }

      break;

    case 't':
      can2040.get_statistics(&can_stats);
      Serial.printf("rx_total = %lu, tx_total = %lu, tx_attempt = %lu, parse_error = %lu\n",
                    can_stats.rx_total, can_stats.tx_total, can_stats.tx_attempt, can_stats.parse_error);
      break;

    case 'p':
      Serial.printf("stopping CAN\n");
      can2040.stop();
      break;

    case 'b':
      Serial.printf("restarting CAN\n");
      can2040.begin();
      break;
    }
  }
}

/// notify callback

void my_cb(struct can2040 * cd, uint32_t notify, struct can2040_msg * msg) {

  (void)(cd);
  Serial.printf("cb: notify event type = %lu\n", notify);

  switch (notify) {
  case CAN2040_NOTIFY_RX:
    Serial.printf("cb: message received\n");
    rx_msg = *msg;
    got_msg = true;
    break;
  case CAN2040_NOTIFY_TX:
    Serial.printf("cb: message sent ok\n");
    break;
  case CAN2040_NOTIFY_ERROR:
    Serial.printf("cb: an error occurred\n");
    break;
  default:
    Serial.printf("cb: unknown event type\n");
    break;
  }
}

/// format CAN message as a string

char *msg_to_str(struct can2040_msg * msg) {

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
