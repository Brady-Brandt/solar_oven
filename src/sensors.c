#include "sensors.h"
#include "debug.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "pulsewidth.pio.h"
#include "hardware/adc.h"
#include "state.h"
#include <hardware/timer.h>
#include <pico/time.h>
#include <stdint.h>


#define ADC_INPUT_PIN 28 

#define TRIGGER_PIN 18
#define INPUT_PIN   19

#define sm 0
#define CAPACITANCE (1.1 * 100*1e-9)

/*
 * Right now debating if we should use a 555 monostable circuit to measure temp
 * Or use the ADCs. ADCS seem to be a bit more accurate, but less precise. Not using
 * them for temperature would also free them up for other needs. Either way I need to
 * calibrate them anyway so I don't think it will matter that much. Right Now I have the 
 * code and circuits built for both.
 *
 * ADC        -> Voltage divider into non inverting op amp with gain ~1.25
 * MONOStable -> Use PIO to trigger a 555 monostable circuit and measure the pulse width 
 */

#define CONVERSION_FACTOR (3.3f / (1 << 12))
#define OP_AMP_GAIN       (1 + 2200.0f / 8200.0f)
#define VOLTAGE_DIVIDER_R 8200.0f
#define ADC_TO_RESISTANCE(val) VOLTAGE_DIVIDER_R / (3.3f / ((CONVERSION_FACTOR * val) / OP_AMP_GAIN) - 1)

static void pio0_irq0_handler() {
    while (!pio_sm_is_rx_fifo_empty(pio0, sm)) {
        uint32_t raw = pio_sm_get(pio0, sm);
        uint32_t high_time = UINT32_MAX - raw;

        float seconds = (high_time * 2) / 125000000.0f;
        program_state.sensor1 = (seconds / CAPACITANCE);
    } 
    pio_interrupt_clear(pio0, 0);
}

static void adc_irq_handler(){
    uint16_t reading = adc_fifo_get();
    program_state.sensor2 = ADC_TO_RESISTANCE(reading);
}

void sensors_init(){
    PIO pio = pio0;

    uint offset = pio_add_program(pio, &pulsewidth_program);

    pio_sm_config c = pulsewidth_program_get_default_config(offset);

    pio_gpio_init(pio, TRIGGER_PIN);
    gpio_init(INPUT_PIN);
    gpio_set_dir(INPUT_PIN, GPIO_IN);

    // Set pin directions
    sm_config_set_out_pins(&c, TRIGGER_PIN, 1);
    sm_config_set_out_pin_count(&c, 1);
    sm_config_set_in_pins(&c, INPUT_PIN);
    sm_config_set_in_pin_count(&c, 1);
    sm_config_set_set_pins(&c, TRIGGER_PIN, 1);
    sm_config_set_set_pin_count(&c, 1);
    sm_config_set_jmp_pin(&c, INPUT_PIN);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_NONE);

    pio_sm_init(pio, sm, offset, &c);

    pio_sm_set_consecutive_pindirs(pio, sm, TRIGGER_PIN, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm, INPUT_PIN, 1, false);
 
    irq_set_exclusive_handler(PIO0_IRQ_0, pio0_irq0_handler);
    pio_set_irq0_source_enabled(pio0, pis_interrupt0, true);
    irq_set_enabled(PIO0_IRQ_0, true);
    pio_sm_set_enabled(pio, sm, true);

    adc_init();
    adc_gpio_init(ADC_INPUT_PIN);
    adc_fifo_setup(true, true, 1, false, false);
    adc_set_clkdiv(65535);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_irq_handler);
    irq_set_enabled(ADC_IRQ_FIFO, true);
    adc_irq_set_enabled(true);
    adc_select_input(2);
    adc_run(true);

    debug_info("Clock Speed: %d Hz\n", clock_get_hz(clk_sys));
    debug_info("PIO ENABLED\n");
}
