#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/sem.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "ws2812.pio.h"
#include "DmxInput.h"

#define PX_PIN_BASE 4
#define PX_NUM 50

#define PX_DMA_CHANNEL 0
#define PX_DMA_CHANNEL_MASK (1u << PX_DMA_CHANNEL)

DmxInput dmxInput;
#define DMX_GPIO_PIN 8
#define DMX_START_CHANNEL 1
#define DMX_START_CODE 0

//XXX: the number of channels sent by the dmx emitter MUST be exactly this
#define DMX_NUM_CHANNELS 400

typedef uint64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;


volatile uint8_t dmx_buffer[DMXINPUT_BUFFER_SIZE(DMX_START_CHANNEL, DMX_NUM_CHANNELS)];
uint8_t dmx_buffer_copy[sizeof(dmx_buffer) / sizeof(*dmx_buffer)];

static void hexdump(void *data, size_t n) {
	unsigned char *str = (unsigned char*)data;

	for (size_t i = 0; i < n; i+=4) {
		printf("%u %u %u %u\n", str[i], str[i+1], str[i + 2], str[i + 3]);
	}
}

static uint32_t px_buffer[PX_NUM];

// posted when it is safe to output a new set of values
static struct semaphore reset_delay_complete_sem;
// alarm handle for handling delay
alarm_id_t reset_delay_alarm_id;
u64 dma_start_time = 0;
u64 dmx_frame_count = 0;


int64_t reset_delay_complete(alarm_id_t id, void *user_data) {
	(void)id;
	(void)user_data;
    reset_delay_alarm_id = 0;
	printf("alarm released sem after %ld us \n", (u32)(time_us_64() - dma_start_time));
    sem_release(&reset_delay_complete_sem);
    return 0;
}

void __isr dma_complete_handler() {
    if (dma_hw->ints1 & PX_DMA_CHANNEL_MASK) {
        // clear IRQ
        dma_hw->ints1 = PX_DMA_CHANNEL_MASK;
        // when the dma is complete we start the reset delay timer
        if (reset_delay_alarm_id)
			cancel_alarm(reset_delay_alarm_id);
		printf("dma send finished in %ld us\n", (u32)(time_us_64() - dma_start_time));
        reset_delay_alarm_id = add_alarm_in_us(400, reset_delay_complete, NULL, true);
    }
}

void dma_init(PIO pio, uint sm) {
    dma_claim_mask(PX_DMA_CHANNEL_MASK);

    // main DMA channel outputs 8 word fragments, and then chains back to the chain channel
    dma_channel_config channel_config = dma_channel_get_default_config(PX_DMA_CHANNEL);
    channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
	channel_config_set_read_increment(&channel_config, true);
	channel_config_set_write_increment(&channel_config, false);
    dma_channel_configure(PX_DMA_CHANNEL,
                          &channel_config,
                          &pio->txf[sm],
                          NULL, // set later
						  PX_NUM,
                          false);

    irq_set_exclusive_handler(DMA_IRQ_1, dma_complete_handler);
    dma_channel_set_irq1_enabled(PX_DMA_CHANNEL, true);
    irq_set_enabled(DMA_IRQ_1, true);
}

void output_dma(void *src) {
	if (dma_start_time)
		printf("new transmission after last one %ld us ago\n", (u32)(time_us_64() - dma_start_time));
    dma_channel_hw_addr(PX_DMA_CHANNEL)->al3_read_addr_trig = (uintptr_t) src;
	dma_start_time = time_us_64();
	dma_channel_start(PX_DMA_CHANNEL);
}

static bool new_dmx_input = false;
void dmx_callback(DmxInput *input) {
	new_dmx_input = true;
}

static void fake_dmx_input() {
	static u64 last = 0;

	u64 now = time_us_64();
	if (now > last + 10000) {
		last = now;
		new_dmx_input = true;
		float dimm = (cos(((float)(now) / 1000000. * 2)) + 1) / 2;
		for (int i = 0; i < PX_NUM; i++) {
				dmx_buffer[i * 3] = 240 * dimm;
				dmx_buffer[i * 3 + 1] = 108 * dimm;
				dmx_buffer[i * 3 + 2] = 54 * dimm;
		}
	}
}


int main() {
    stdio_init_all();
	puts("hello world");

    PIO pio = pio0;
    int sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, PX_PIN_BASE, 800000, false);

    sem_init(&reset_delay_complete_sem, 1, 1); // initially posted so we don't block first time
	gpio_pull_up(DMX_GPIO_PIN);
    dma_init(pio, sm);

	dmxInput.begin(DMX_GPIO_PIN, DMX_START_CHANNEL, DMX_NUM_CHANNELS, pio=pio1, false);

    //inMode(LED_BUILTIN, OUTPUT);
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN, 0);

	/*
	dmxInput.read_async(dmx_buffer, dmx_callback);

    while (1) {
		if (new_dmx_input) {
			dmx_frame_count++;
			irq_set_enabled(DMA_IRQ_0, false); //prevent concurrent access to new_dmx_input
			new_dmx_input = false;
			if (dmx_buffer[0] == DMX_START_CODE) {
				printf("new dmx frame received on pin %d! ts: %llu\n", DMX_GPIO_PIN, time_us_64());
				memcpy(dmx_buffer_copy, (void*)dmx_buffer, sizeof(dmx_buffer_copy));
			}
			irq_set_enabled(DMA_IRQ_0, true);
		}

		//TODO start rendering just before we should be able to acquire the semaphore
		//need a second buffer in that case

		if (dma_start_time)
			printf("acquiring semaphore %ld \n", (u32)(time_us_64() - dma_start_time));

		sem_acquire_blocking(&reset_delay_complete_sem);

		if (!dmx_frame_count) {
			px_buffer[0] = 0;
		} else if (dmx_frame_count % 3 == 0) {
			px_buffer[0] = 0xff000000;
		} else if (dmx_frame_count % 3 == 1) {
			px_buffer[0] = 0x00ff0000;
		} else if (dmx_frame_count % 3 == 2) {
			px_buffer[0] = 0x0000ff00;
		}
		for (int i = 1; i < PX_NUM; i++) {
			px_buffer[i] =          // start at +1 to skip dmx start code
				dmx_buffer_copy[i * 3 + 1] << 24
				| dmx_buffer_copy[i * 3 + 2] << 16
				| dmx_buffer_copy[i * 3 + 3] << 8;
		}

		if (dma_start_time)
			printf("acquired semaphore %ld \n", (u32)(time_us_64() - dma_start_time));
		output_dma(px_buffer);
    }
	*/
	while (1) {
	    // Wait for next DMX packet
		dmxInput.read(dmx_buffer);
		dmx_frame_count++;
		gpio_put(PICO_DEFAULT_LED_PIN, dmx_frame_count % 2);

		sem_acquire_blocking(&reset_delay_complete_sem);
		// Blink the LED to indicate that a packet was received
		//digitalWrite(LED_BUILTIN, HIGH);
		//delay(10);
		//digitalWrite(LED_BUILTIN, LOW);
		for (int i = 1; i < PX_NUM; i++) {
			px_buffer[i] =          // start at +1 to skip dmx start code
				dmx_buffer[i * 3 + 1] << 24
				| dmx_buffer[i * 3 + 2] << 16
				| dmx_buffer[i * 3 + 3] << 8;
		}

		output_dma(px_buffer);
	}
}
