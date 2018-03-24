static void tesla_rx_hook(CAN_FIFOMailBox_TypeDef *to_push) {
  // this should eventually read reverse state. For now track lock/unlock
  // 0x248 is MCU_lockRequest. "\x00\x00\x01\x00 \x00\x00\x00\x00" is locked. "\x00\x00\x02\x00 \x00\x00\x00\x00" is unlocked
  // RIR is CAN identity register
  if ((to_push->RIR>>21) == 0x248 {
    //RDHR is 32bit high register . RDLR is 32bit low register (rightmost 32bits of CAN message) 
    int locked_state = (to_push->RDHR & 0xF0) >> 8;
    if (locked_state == 2) {
      set_gpio_output(GPIOB, 10, 0);
    } else if (locked_state == 1) {
      set_gpio_output(GPIOB, 10, 1);
    }
  }
}

// *** no output safety mode ***

static void tesla_nooutput_init(int16_t param) {
  controls_allowed = 0;
}

static int tesla_nooutput_tx_hook(CAN_FIFOMailBox_TypeDef *to_send) {
  return false;
}

static int tesla_nooutput_tx_lin_hook(int lin_num, uint8_t *data, int len) {
  return false;
}

static int tesla_nooutput_fwd_hook(int bus_num, CAN_FIFOMailBox_TypeDef *to_fwd) {
  return -1;
}

const safety_hooks tesla_nooutput_hooks = {
  .init = tesla_nooutput_init,
  .rx = tesla_rx_hook,
  .tx = tesla_nooutput_tx_hook,
  .tx_lin = tesla_nooutput_tx_lin_hook,
  .fwd = tesla_nooutput_fwd_hook,
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

