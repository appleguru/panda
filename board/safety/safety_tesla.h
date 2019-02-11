#include "../drivers/uja1023.h"

const int32_t STW_MENU_BTN_HOLD_INTERVAL = 750000; //750ms, how long before we recognize the user is  holding this steering wheel button down
const int32_t HDMI_SWITCH_BUTTON_HOLD_DURATION = 500000; //500ms, how long to press the button on the hdmi switcher for when the input changes

uint32_t stw_menu_btn_pressed_ts = 0;
uint32_t hdmi_btn_pressed_ts = 0;
uint32_t hdmi_fake_menu_btn_pressed_ts = 0;

int stw_menu_current_output_state = 1; // 0 = rear camera, 1 = HDMI1, 2 = HDMI2, 3 = HDMI3, 4 = HDMI4 
int stw_menu_btn_state_last = 0;
int stw_menu_output_flag = 0;
int hdmi_input_num = 0;
int hdmi_switcher_input_change_in_progress = 0;

static void tesla_rx_hook(CAN_FIFOMailBox_TypeDef *to_push) {
  uint32_t ts = TIM2->CNT;
  
  if (hdmi_switcher_input_change_in_progress == 1 && ts > (hdmi_btn_pressed_ts + HDMI_SWITCH_BUTTON_HOLD_DURATION)) {
    hdmi_switcher_input_change_in_progress = 0;
    clear_uja1023_output_bits(1 << hdmi_input_num);
    puts("Unpush HDMI button\n");
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
  }

  //0x45 is STW_ACTN_RQ
  if (addr == 0x45) {
    int stw_menu_button = (to_push->RDHR >> 5) & 0x1; //StW_Sw05_Psd : 37|1@1+
    int stw_rt_scroll_wheel = (to_push->RDHR >> 4) & 0x1; //StW_Sw04_Psd : 36|1@1+
    
    if (stw_rt_scroll_wheel == 1) {
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
              puts("Switching to HDMI ");
              puth(hdmi_input_num + 1);
              puts("\n");
              // change input on HDMI switcher
              set_uja1023_output_bits(1 << hdmi_input_num);
              hdmi_btn_pressed_ts = TIM2->CNT;
              hdmi_switcher_input_change_in_progress = 1;
            }
            else {
              //go back to state 0 if we're >=4
              stw_menu_current_output_state = 0;
              puts("Switching to Rear Camera\n");
            }
          }//only change state once per press! 
        } //held
      }
    } //stw menu button pressed
    else if (stw_rt_scroll_wheel == 0) {
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

/*

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

*/
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

