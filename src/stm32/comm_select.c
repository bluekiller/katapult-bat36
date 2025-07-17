#include "autoconf.h"
#include "board/armcm_timer.h" // udelay
#include "board/gpio.h"        // gpio_out_setup
#include "sched.h"
#include "comm_select.h"
#include "internal.h"         // GPIO
#include "board/armcm_boot.h" // armcm_enable_irq
#include "board/misc.h" // console_sendf

// Selection pin (configure via CONFIG_BUTTON_PIN or hardcode)
#define COMM_SELECT_PIN GPIO('C', 14) // Change as needed

static int use_usb = 0;
extern void usb_init(void);
extern void can_init(void);
extern void USB_IRQHandler(void);
extern void CAN_IRQHandler(void);
extern void DefaultHandler(void);

void USB_OR_CAN_IRQHandler(void)
{
    if (use_usb)
    {
        USB_IRQHandler();
    }
    else
    {
        CAN_IRQHandler();
    }
}

void Default_OR_CAN_IRQHandler(void)
{
    if (use_usb)
    {
        DefaultHandler();
    }
    else
    {
        CAN_IRQHandler();
    }
}

// Check selection pin early in boot process
void comm_select_init(void)
{
    // Read pin state (high = USB, low = CAN)
    struct gpio_in sel_pin = gpio_in_setup(COMM_SELECT_PIN, 1); // pullup
    udelay(10);
    use_usb = gpio_in_read(sel_pin);
    if (use_usb)
    {
        usb_init();
        // Only enable the shared USB/CAN IRQ (IRQ 20)
        armcm_enable_irq(USB_OR_CAN_IRQHandler, 20, 1);
    }
    else
    {
        can_init();
        // Enable all CAN IRQs: IRQ 19 (CAN TX), IRQ 20 (CAN RX0), IRQ 21 (CAN RX1)
        armcm_enable_irq(USB_OR_CAN_IRQHandler, 20, 1);
        armcm_enable_irq(Default_OR_CAN_IRQHandler, 19, 1);
        armcm_enable_irq(Default_OR_CAN_IRQHandler, 21, 1);
    }
}

DECL_INIT(comm_select_init);

void comm_select_run_taskfuncs(void)
{
    if (use_usb)
    {
        extern void usb_ep0_task(void);
        usb_ep0_task();
        extern void usb_bulk_out_task(void);
        usb_bulk_out_task();
        extern void usb_bulk_in_task(void);
        usb_bulk_in_task();
    }
    else
    {
        extern void canserial_rx_task(void);
        canserial_rx_task();
        extern void canserial_tx_task(void);
        canserial_tx_task();
    }
}

DECL_TASK(comm_select_run_taskfuncs);

void comm_select_run_shutdownfuncs(void)
{
    if (use_usb)
    {
        extern void usb_shutdown(void);
        usb_shutdown();
    }
    else
    {
        extern void canserial_shutdown(void);
        canserial_shutdown();
    }
}
DECL_SHUTDOWN(comm_select_run_shutdownfuncs);

void console_sendf(const struct command_encoder *ce, va_list args)
{
    if (use_usb)
    {
        extern void console_sendf_usb(const struct command_encoder *ce, va_list args);
        console_sendf_usb(ce, args);
    }
    else
    {
        extern void console_sendf_can(const struct command_encoder *ce, va_list args);
        console_sendf_can(ce, args);
    }
}

// Optional: function to query current selection
int comm_use_usb(void)
{
    return use_usb;
}