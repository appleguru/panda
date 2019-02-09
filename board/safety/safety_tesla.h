#include "../drivers/uja1023.h"

const int32_t STW_MENU_BTN_HOLD_INTERVAL = 750000; //75ms, how long before we recognize the user is  holding this steering wheel button down

uint32_t stw_menu_btn_pressed_ts = 0;
int stw_menu_current_output_state = 0; // 0 = front camera, 1 = rear camera, 2 = HDMI
int stw_menu_btn_state_last = 0;
int stw_menu_output_flag = 0;

static void tesla_rx_hook(CAN_FIFOMailBox_TypeDef *to_push) {
  uint32_t ts = TIM2->CNT;
  
  //Set UJA1023 outputs for camera swicther/etc.
  
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
    int brake_pressed = (to_push->RDLR & 0x8000) >> 15;
    int tesla_speed_mph = ((((((to_push->RDLR >> 24) & 0x0F) << 8) + (( to_push->RDLR >> 16) & 0xFF)) * 0.05 -25));

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
            if (stw_menu_current_output_state < 2) {
              stw_menu_current_output_state++;
            }
            else {
              //go back to state 0 if we're >=2
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
    if (addr == 0x45) {
      if (stw_menu_current_output_state >= 1) {
        //if we're on reverse (1) forward steering wheel messages to the HDMI switcher
        // This means HDMI switcher will switch inputs when going from rear cam to HDMI, and also from HDMI to front cam
        return 1;
        }
      else {
        //if the current output state is the forward camera (0), don't forward steering wheel button messages to the HDMI switcher
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

