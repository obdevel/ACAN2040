//

/// this library header

#include <ACAN2040.h>

/// context variables for up to 3 concurrent buses, numbered according to PIO instance -- 0, 1 or 2

static struct can2040 _contexts[3];

/// PIO IRQ handlers for up to 3 concurrent buses
/// these must be static non-class functions as we cannot pass a class member to an IRQ handler

static void PIO0_IRQHandler(void) {
	can2040_pio_irq_handler(&_contexts[0]);
}

static void PIO1_IRQHandler(void) {
	can2040_pio_irq_handler(&_contexts[1]);
}

#if PICO_RP2350
static void PIO2_IRQHandler(void) {
	can2040_pio_irq_handler(&_contexts[2]);
}
#endif

/// constructor

ACAN2040::ACAN2040(uint32_t pio_num, uint32_t gpio_tx, uint32_t gpio_rx, uint32_t bitrate, uint32_t sys_clock, void (*callback)(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg))
	: _pio_num(pio_num), _gpio_tx(gpio_tx), _gpio_rx(gpio_rx), _bitrate(bitrate), _sys_clock(sys_clock), _callback(callback) {
}

/// begin CAN processing
/// https://github.com/KevinOConnor/can2040/blob/master/docs/API.md#startup
/// https://github.com/KevinOConnor/can2040/blob/master/example/canexample.c

void ACAN2040::begin() {

	// Setup canbus

	can2040_setup(&_contexts[_pio_num], _pio_num);
	can2040_callback_config(&_contexts[_pio_num], _callback);

	// Enable irqs

	switch (_pio_num) {

	case 0:
		irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIO0_IRQHandler);
		irq_set_priority(PIO0_IRQ_0, 1);
		irq_set_enabled(PIO0_IRQ_0, 1);
		break;

	case 1:
		irq_set_exclusive_handler(PIO1_IRQ_0_IRQn, PIO1_IRQHandler);
		irq_set_priority(PIO1_IRQ_0, 1);
		irq_set_enabled(PIO1_IRQ_0, 1);
		break;

#if PICO_RP2350
	case 2:
		irq_set_priority(PIO2_IRQ_0, 1);
		irq_set_enabled(PIO2_IRQ_0, 1);
		irq_set_exclusive_handler(PIO2_IRQ_0_IRQn, PIO2_IRQHandler);
		break;
#endif
	}

	// start canbus message processing

	can2040_start(&_contexts[_pio_num], _sys_clock, _bitrate, _gpio_rx, _gpio_tx);
	return;
}

/// check that driver can accept a message to transmit
/// failure indicates that the internal transmit queue is full due to a bus error

bool ACAN2040::ok_to_send(void) {

	return can2040_check_transmit(&_contexts[_pio_num]);
}

/// transmit a CAN message
/// call ok_to_send() first

bool ACAN2040::send_message(struct can2040_msg *msg) {

	return (can2040_transmit(&_contexts[_pio_num], msg) == 0);
}

/// stop the CAN driver

void ACAN2040::stop(void) {

	can2040_stop(&_contexts[_pio_num]);
}

/// retrieve CAN statistics -- see can2040 API docs

void ACAN2040::get_statistics(struct can2040_stats *can_stats) {

	can2040_get_statistics(&_contexts[_pio_num], can_stats);
	return;
}

///

