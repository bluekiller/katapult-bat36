// Forward declarations for manual initialization
void usb_init(void);
void can_init(void);
void enable_can_irq(void);
void enable_usb_irq(void);

// Optional: functions to check if interfaces are enabled
int comm_use_usb(void);