#include "../drivers/uja1023.h"

const int32_t STW_MENU_BTN_HOLD_INTERVAL = 750000; //75ms, how long before we recognize the user is  holding this steering wheel button down
const int32_t HDMI_SWITCH_BUTTON_HOLD_DURATION = 500000; //50ms, how long to press the button on the hdmi switcher for when the input changes

uint32_t stw_menu_btn_pressed_ts = 0;
uint32_t hdmi_btn_pressed_ts = 0;
int stw_menu_current_output_state = 0; // 0 = rear camera, 1 = HDMI1, 2 = HDMI2, 3 = HDMI3, 4 = HDMI4 
int stw_menu_btn_state_last = 0;
int stw_menu_output_flag = 0;
int hdmi_input_num = 0;
int hdmi_input_change_in_progress = 0;

static int add_tesla_crc(uint32_t MLB, uint32_t MHB , int msg_len) {
  //"""Calculate CRC8 using 1D poly, FF start, FF end"""
  int crc_lookup[256] = { 0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53, 0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB, 
    0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E, 0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76, 
    0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4, 0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C, 
    0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19, 0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1, 
    0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40, 0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8, 
    0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D, 0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65, 
    0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7, 0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F, 
    0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A, 0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2, 
    0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75, 0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D, 
    0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8, 0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50, 
    0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2, 0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A, 
    0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F, 0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7, 
    0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66, 0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E, 
    0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB, 0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43, 
    0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1, 0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09, 
    0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C, 0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4 };
  int crc = 0xFF;
  for (int x = 0; x < msg_len; x++) {
    int v = 0;
    if (x <= 3) {
      v = (MLB >> (x * 8)) & 0xFF;
    } else {
      v = (MHB >> ( (x-4) * 8)) & 0xFF;
    }
    crc = crc_lookup[crc ^ v];
  }
  crc = crc ^ 0xFF;
  return crc;
}

static void send_fake_message(uint32_t RIR, uint32_t RDTR,int msg_len, int msg_addr, int bus_num, uint32_t data_lo, uint32_t data_hi) {
  CAN_FIFOMailBox_TypeDef to_send;
  uint32_t addr_mask = 0x001FFFFF;
  to_send.RIR = (msg_addr << 21) + (addr_mask & (RIR | 1));
  to_send.RDTR = (RDTR & 0xFFFFFFF0) | msg_len;
  to_send.RDLR = data_lo;
  to_send.RDHR = data_hi;
  can_send(&to_send, bus_num);
}

static void do_send_menu_btn_to_hdmi(CAN_FIFOMailBox_TypeDef *to_push) {
  const int hdmi_bus_num = 2;
  uint32_t MLB;
  uint32_t MHB; 
  MLB = (to_push->RDLR);
  MHB = (to_push->RDHR & 0xFFFFFFEF) + 0x10;
  int crc = add_tesla_crc(MLB, MHB,7);
  MHB = MHB + (crc << 16);
  send_fake_message(to_push->RIR,to_push->RDTR,8,0x45,hdmi_bus_num,MLB,MHB);
}

static void tesla_rx_hook(CAN_FIFOMailBox_TypeDef *to_push) {
  uint32_t ts = TIM2->CNT;
  
  if (hdmi_input_change_in_progress == 1 && ts > (hdmi_btn_pressed_ts + HDMI_SWITCH_BUTTON_HOLD_DURATION)) {
    hdmi_input_change_in_progress = 0;
    clear_uja1023_output_bits(1 << hdmi_input_num);
  }
  
  uint32_t addr;
  if (to_push->RIR & 4) {
    // Extended
    // Not looked at, but have to be separated
    // to avoid address collision
    addr = to_push->RIR >> 3;
  } else {
    // Normal
    addr = to_push->RIR >> 21;
  }
  
  //0x118 is DI_torque2
  if (addr == 0x118) {
    int drive_state = (to_push->RDLR >> 12) & 0x7; //DI_gear : 12|3@1+
    int tesla_speed_mph = ((((((to_push->RDLR >> 24) & 0x0F) << 8) + (( to_push->RDLR >> 16) & 0xFF)) * 0.05 -25));

    if (drive_state == 2 && stw_menu_current_output_state > 0) {
      //we're in reverse, show the rear camera
      //puts(" Got Reverse\n");
      //TODO: if we're in reverse and not already on the SERDES input, we need to generate a steering wheel button message and send to the HDMI switcher so that it switches from HDMI to SERDES. Then, once we leave reverse, we need to restore our state.  This is not trivial on stock panda.
      do_send_menu_btn_to_hdmi(to_push);
    } //in reverse
  }

  //0x45 is STW_ACTN_RQ
  if (addr == 0x45) {
    int stw_menu_button = (to_push->RDHR >> 5) & 0x1; //StW_Sw05_Psd : 37|1@1+
    if (stw_menu_button == 1) {
      //menu button is pushed, if it wasn't last time, set the initial timestamp
      if (stw_menu_btn_state_last == 0) {
        stw_menu_btn_state_last = 1;
        stw_menu_btn_pressed_ts = ts;  
      }
      else {
        uint32_t stw_ts_elapsed = get_ts_elapsed(ts, stw_menu_btn_pressed_ts);
        if (stw_ts_elapsed > STW_MENU_BTN_HOLD_INTERVAL) {
          //user held the button, do stuff!
          if (stw_menu_output_flag == 0) {
            stw_menu_output_flag = 1;
            if (stw_menu_current_output_state < 4) {
              stw_menu_current_output_state++;
              hdmi_input_num = stw_menu_current_output_state - 1;
              // change input on HDMI switcher
              set_uja1023_output_bits(1 << hdmi_input_num);
              hdmi_btn_pressed_ts = TIM2->CNT;
              hdmi_input_change_in_progress = 1;
            }
            else {
              //go back to state 0 if we're >=4
              stw_menu_current_output_state = 0;
            }
          }//only change state once per press! 
        } //held
      }
    } //stw menu button pressed
    else if (stw_menu_button == 0) {
      stw_menu_output_flag = 0;
      stw_menu_btn_state_last = 0;
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

  int32_t addr = to_fwd->RIR >> 21;

  if (bus_num == 0) {
    // intercept and squash some of the menu button commands
    //stw_menu_current_output_state: 0 = rear camera, 1 = HDMI1, 2 = HDMI2, 3 = HDMI3, 4 = HDMI4 
    // drive_state 2 is reverse
    if (addr == 0x45) {
      if (stw_menu_current_output_state == 0 || stw_menu_current_output_state == 4) {
        // if we're transitioning from an HDMI input to the rear cam or from the rear cam to the serdes, pass the menu button messages along to the HDMI switcher
        return 1;
      }
      else {
        // if we're just changing HDMI inputs, squash the button presses so the HDMI switcher stays on HDMI input
        return -1;
      }
    } //STW ACT REQ
    return 1; // Output all non-matched messages to HDMI switcher over bus 1 to look more like its connected to the actual car
  } //if message is from bus0
  return -1; //only forward messages from bus 0
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

