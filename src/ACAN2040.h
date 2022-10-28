
#pragma once

// Pico SDK headers - supplied with arduino-pico core
#include "RP2040.h" // hw_set_bits
#include "hardware/regs/dreq.h" // DREQ_PIO0_RX1
#include "hardware/structs/dma.h" // dma_hw
#include "hardware/structs/iobank0.h" // iobank0_hw
#include "hardware/structs/padsbank0.h" // padsbank0_hw
#include "hardware/structs/pio.h" // pio0_hw
#include "hardware/structs/resets.h" // RESETS_RESET_PIO0_BITS

// additional SDK header required for successful compilation
#include <hardware/irq.h>

// can2040 header
#include "can2040.h"


/// Arduino library class that wraps the acan2040 Kevin's code

class ACAN2040 {

public:
	ACAN2040(uint32_t pio_num, uint32_t gpio_tx, uint32_t gpio_rx, uint32_t bitrate, uint32_t sys_clock, void (*callback)(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg));
	void begin();
	bool send_message(struct can2040_msg *msg);
	bool ok_to_send(void);

private:
	uint32_t _pio_num;
	uint32_t _bitrate;
	uint32_t _gpio_tx, _gpio_rx;
	uint32_t _sys_clock;
	struct can2040 _cbus;
	void (*_callback)(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg);
};
