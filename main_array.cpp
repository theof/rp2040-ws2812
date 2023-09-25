#include "pico/sem.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "ws2812.pio.h"
#include "DmxInput.h"
#include "leds_array.h"

#define WS2812_PIN_BASE 4
#define DMA_CHANNEL 0

#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)

DmxInput dmxInput;
#define DMX_GPIO_PIN 8

volatile uint8_t dmx_buffer[DMXINPUT_BUFFER_SIZE(DMX_START_CHANNEL, DMX_NUM_CHANNELS)];
float dmx_buffer_smooth[DMXINPUT_BUFFER_SIZE(DMX_START_CHANNEL, DMX_NUM_CHANNELS)];

static void hexdump(void *data, size_t n) {
	unsigned char *str = (unsigned char*)data;

	for (size_t i = 0; i < n; i+=4) {
		printf("%u %u %u %u\n", str[i], str[i+1], str[i + 2], str[i + 3]);
	}
}

static uint32_t buffers[2][NUM_PIXELS];


// posted when it is safe to output a new set of values
static struct semaphore reset_delay_complete_sem;
// alarm handle for handling delay
alarm_id_t reset_delay_alarm_id;
u64 dma_start_time = 0;

int64_t reset_delay_complete(alarm_id_t id, void *user_data) {
	(void)id;
	(void)user_data;
    reset_delay_alarm_id = 0;
	printf("alarm released sem after %d us \n", (u32)(time_us_64() - dma_start_time));
    sem_release(&reset_delay_complete_sem);
    return 0;
}

void __isr dma_complete_handler() {
    if (dma_hw->ints1 & DMA_CHANNEL_MASK) {
        // clear IRQ
        dma_hw->ints1 = DMA_CHANNEL_MASK;
        // when the dma is complete we start the reset delay timer
        if (reset_delay_alarm_id)
			cancel_alarm(reset_delay_alarm_id);
		printf("dma send finished in %d us\n", (u32)(time_us_64() - dma_start_time));
        reset_delay_alarm_id = add_alarm_in_us(400, reset_delay_complete, NULL, true);
    }
}

void dma_init(PIO pio, uint sm) {
    dma_claim_mask(DMA_CHANNEL_MASK);

    // main DMA channel outputs 8 word fragments, and then chains back to the chain channel
    dma_channel_config channel_config = dma_channel_get_default_config(DMA_CHANNEL);
    channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
	channel_config_set_read_increment(&channel_config, true);
	channel_config_set_write_increment(&channel_config, false);
    dma_channel_configure(DMA_CHANNEL,
                          &channel_config,
                          &pio->txf[sm],
                          NULL, // set later
						  NUM_PIXELS,
                          false);

    irq_set_exclusive_handler(DMA_IRQ_1, dma_complete_handler);
    dma_channel_set_irq1_enabled(DMA_CHANNEL, true);
    irq_set_enabled(DMA_IRQ_1, true);
}

void output_dma(void *src) {
	if (dma_start_time)
		printf("new transmission after last one %d us ago\n", (u32)(time_us_64() - dma_start_time));
    dma_channel_hw_addr(DMA_CHANNEL)->al3_read_addr_trig = (uintptr_t) src;
	dma_start_time = time_us_64();
	dma_channel_start(DMA_CHANNEL);
}

static bool new_dmx_input = false;
void dmx_callback(DmxInput *input) {
	printf("new dmx frame received on pin %d! ts: %llu\n", input->pin(), time_us_64());
	hexdump((void*)dmx_buffer, sizeof(dmx_buffer));
	new_dmx_input = true;
}

#define SMOOTHING_STEPS 3

void smooth_dmx_buffer() {
	static int init = 1;
	static float chan_steps[DMX_NUM_CHANNELS];
	static int n_step = 0;
	float diff;

	irq_set_enabled(DMA_IRQ_0, false); //prevent concurrent access to new_dmx_input
	if (new_dmx_input) {
		new_dmx_input = false;
		if (init) {
			memcpy(dmx_buffer_smooth, (void*)dmx_buffer, sizeof(dmx_buffer));
			init = 0;
			return;
		}
		n_step = SMOOTHING_STEPS;
		for (int i = 0; i < DMX_NUM_CHANNELS; i++) {
			diff = dmx_buffer[DMX_START_CHANNEL + i] - dmx_buffer_smooth[DMX_START_CHANNEL + i];
			chan_steps[i] = diff / SMOOTHING_STEPS;
		}
	}
	irq_set_enabled(DMA_IRQ_0, true);
	if (n_step-- > 0) {
		for (int i = 0; i < DMX_NUM_CHANNELS; i++) {
			dmx_buffer_smooth[i + DMX_START_CHANNEL] += chan_steps[i];
		}
	} else if (n_step-- == 0) {
		//reach the target value
		for (int i = DMX_START_CHANNEL; i < DMX_START_CHANNEL + DMX_NUM_CHANNELS; i++) {
			dmx_buffer_smooth[i] = dmx_buffer[i];
		}
	}
}

int main() {
    stdio_init_all();
	puts("hello world");

    PIO pio = pio0;
    int sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN_BASE, 800000, false);

    sem_init(&reset_delay_complete_sem, 1, 1); // initially posted so we don't block first time
	gpio_pull_up(DMX_GPIO_PIN);
    dma_init(pio, sm);
	dmxInput.begin(DMX_GPIO_PIN, DMX_START_CHANNEL, DMX_NUM_CHANNELS, pio=pio1, false);
	dmxInput.read_async(dmx_buffer, dmx_callback);

    while (1) {
        uint current = 0;
		smooth_dmx_buffer();
		//TODO start rendering just before we should be able to acquire the semaphore
		if (dma_start_time)
			printf("acquiring semaphore %d \n", (u32)(time_us_64() - dma_start_time));

		sem_acquire_blocking(&reset_delay_complete_sem);
		for (int i = 0; i < NUM_PIXELS; i++) {
			buffers[current][i] = 0xFFFFFF00;
		}
			
		//draw((uint8_t *)buffers[current], dmx_buffer, dmx_buffer_smooth);
		if (dma_start_time)
			printf("acquired semaphore %d \n", (u32)(time_us_64() - dma_start_time));
		output_dma(buffers[current]);

		current ^= 1;
    }
}
