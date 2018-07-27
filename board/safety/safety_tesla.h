#include "../drivers/uja1023.h"

static void tesla_rx_hook(CAN_FIFOMailBox_TypeDef *to_push) {
  // 0x118 on bus 2 is DI_torque2. Example message on bus2, id 0x118: FF AF F4 21 91 6B
  // 9F is park, AF is reverse, CF is drive, BF is neutral (with brake on)
  // specifically, bits 20-22 are drive state: 001 (1) is park, 010 (2) is reverse, 011 (3) is Neutral, 100 (4) is drive
  
  // RIR is CAN identity register
  if ((to_push->RIR >> 21) == 0x118) {
    //RDHR is 32bit high register. RDLR is 32bit low register
    //if whole CAN message is FF AF F4 21 91 6B, RDLR contains 21 F4 AF FF
    int drive_state = (to_push->RDLR >> 12) & 0x7;
    
    //if the car goes into reverse, set GMLAN to 1 (low.. inverted via circuit)...
    if (drive_state == 2) {
      set_uja1023_output(0x01);
      //puts("Got Reverse\n");
    } else {
      set_uja1023_output(0x00);
      //puts("Got Drive\n");
    }
  }
}

// *** no output safety mode ***

static void tesla_init(int16_t param) {
  controls_allowed = 0;
  uja1023_init();
}

static int tesla_tx_hook(CAN_FIFOMailBox_TypeDef *to_send) {
  return false;
}

static int tesla_tx_lin_hook(int lin_num, uint8_t *data, int len) {
  return false;
}

static int tesla_fwd_hook(int bus_num, CAN_FIFOMailBox_TypeDef *to_fwd) {
  return -1;
}

const safety_hooks tesla_hooks = {
  .init = tesla_init,
  .rx = tesla_rx_hook,
  .tx = tesla_tx_hook,
  .tx_lin = tesla_tx_lin_hook,
  .fwd = tesla_fwd_hook,
};

// *** all output safety mode ***

static void tesla_alloutput_init(int16_t param) {
  controls_allowed = 1;
}

static int tesla_alloutput_tx_hook(CAN_FIFOMailBox_TypeDef *to_send) {
  return true;
}

static int tesla_alloutput_tx_lin_hook(int lin_num, uint8_t *data, int len) {
  return true;
}

static int tesla_alloutput_fwd_hook(int bus_num, CAN_FIFOMailBox_TypeDef *to_fwd) {
  return -1;
}

const safety_hooks tesla_alloutput_hooks = {
  .init = tesla_alloutput_init,
  .rx = tesla_rx_hook,
  .tx = tesla_alloutput_tx_hook,
  .tx_lin = tesla_alloutput_tx_lin_hook,
  .fwd = tesla_alloutput_fwd_hook,
};

