#include "../drivers/uja1023.h"

uint32_t tesla_ts_brakelight_on_last = 0;
const int32_t BRAKELIGHT_CLEAR_INTERVAL = 250000; //25ms; needs to be slower than the framerate difference between the DI_torque2 (~100Hz) and DI_state messages (~10hz).
const int32_t STW_MENU_BTN_HOLD_INTERVAL = 750000; //75ms, how long before we recognize the user is  holding this steering wheel button down

uint32_t stw_menu_btn_pressed_ts = 0;
int stw_menu_current_output_state = 0;
int stw_menu_btn_state_last = 0;
int stw_menu_output_flag = 0;
int high_beam_lever_state = 0;



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

    //if the car goes into reverse, set UJA1023 output pin 0 to high. If Drive, set pin 1 high.
    //DI_gear 7 "DI_GEAR_SNA" 4 "DI_GEAR_D" 3 "DI_GEAR_N" 2 "DI_GEAR_R" 1 "DI_GEAR_P" 0 "DI_GEAR_INVALID" ;
    if (drive_state == 2) {
      set_uja1023_output_bits(1 << 0);
      //puts(" Got Reverse\n");
    } else {
      clear_uja1023_output_bits(1 << 0);
    }
    if (drive_state == 4) {
      set_uja1023_output_bits(1 << 1);
      //puts(" Got Drive\n");
    } else {
      clear_uja1023_output_bits(1 << 1);
    }

    //if the car's brake is pressed, set pin 2 to high
    if (brake_pressed == 1) {
      set_uja1023_output_bits(1 << 2);
      //puts(" Brake on!\n");
      tesla_ts_brakelight_on_last = ts;
    } else {
      uint32_t ts_elapsed = get_ts_elapsed(ts, tesla_ts_brakelight_on_last);
      if (ts_elapsed > BRAKELIGHT_CLEAR_INTERVAL) {
        clear_uja1023_output_bits(1 << 2);
        //puts(" Brakelight off!\n");
      } 
    }
  }
  
  //BO_ 872 DI_state: 8 DI
  if (addr == 0x368) {
    int regen_brake_light = (to_push->RDLR >> 8) & 0x1; //DI_regenLight : 8|1@1+
    //if the car's brake lights are on, set pin 2 to high
    if (regen_brake_light == 1) {
      set_uja1023_output_bits(1 << 2);
      //puts(" Regen Brake Light on!\n");
      tesla_ts_brakelight_on_last = ts;
    } else {
      uint32_t ts_elapsed = get_ts_elapsed(ts, tesla_ts_brakelight_on_last);
      if (ts_elapsed > BRAKELIGHT_CLEAR_INTERVAL) {
        clear_uja1023_output_bits(1 << 2);
        //puts(" Brakelight off!\n");
      }
    }
  }

  //0x45 is STW_ACTN_RQ
  if (addr == 0x45) {
    int turn_signal_lever = (to_push->RDLR >> 16) & 0x3; //TurnIndLvr_Stat : 16|2@1+
    int stw_menu_button = (to_push->RDHR >> 5) & 0x1; //StW_Sw05_Psd : 37|1@1+
    high_beam_lever_state = (to_push->RDLR >> 18) & 0x3; //SG_ HiBmLvr_Stat : 18|2@1+ 
    
    //TurnIndLvr_Stat 3 "SNA" 2 "RIGHT" 1 "LEFT" 0 "IDLE" ;
    if (turn_signal_lever == 1) {
      //Left turn signal is on, turn on output pin 3
      set_uja1023_output_bits(1 << 3);
      //puts(" Left turn on!\n");
    }
    else {
      clear_uja1023_output_bits(1 << 3);
    }
    if (turn_signal_lever == 2) {
      //Right turn signal is on, turn on output pin 4
      set_uja1023_output_bits(1 << 4);
      //puts(" Right turn on!\n");
    }
    else {
      clear_uja1023_output_bits(1 << 4);
    }
    
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
          if (stw_menu_current_output_state == 0 && stw_menu_output_flag == 0) {
            stw_menu_output_flag = 1;
            stw_menu_current_output_state = 1;
            set_uja1023_output_bits(1 << 5);
            puts("Menu Button held, setting output 5 HIGH\n");
          }
          else if (stw_menu_current_output_state == 1 && stw_menu_output_flag == 0) {
            stw_menu_output_flag = 1;
            stw_menu_current_output_state = 0;
            clear_uja1023_output_bits(1 << 5);
            puts("Menu Button held, setting output 5 LOW\n");
          }
        } //held
      }
    } //stw menu button pressed
    else if (stw_menu_button == 0) {
      stw_menu_output_flag = 0;
      stw_menu_btn_state_last = 0;
    }
  }
  
  //BO_ 1001 DAS_bodyControls: 8 XXX
  if (addr == 0x3e9) {
    int high_beam_decision = (to_push->RDLR >> 10) & 0x3; //DAS_highLowBeamDecision : 10|2@1+
    // highLowBeamDecision:
    //0: Undecided (Car off)
    //1: Off
    //2: On
    //3: Auto High Beam is disabled
    //VAL_ 69 HiBmLvr_Stat 3 "SNA" 2 "HIBM_FLSH_ON_PSD" 1 "HIBM_ON_PSD" 0 "IDLE" ;

    //If the lever is in either high beam position and auto high beam is off or indicates highs should be on
    if ((high_beam_decision == 3 && (high_beam_lever_state == 2 || high_beam_lever_state == 1))
    || (high_beam_decision == 2 && (high_beam_lever_state == 2 || high_beam_lever_state == 1))) {
      //high beams are on. Set the output 6 high
      set_uja1023_output_bits(1 << 6);
      //puts("High Beam on!\n");
    } //high beams on!
    else {
      //high beams are off. Set the output 6 low
      clear_uja1023_output_bits(1 << 6);
      //puts("High Beam off!\n");
    } //high beams off
  } //DAS_bodyControls
    
    /*
    Set UJA1023 output pins. Bits = pins
    P0 = bit 0
    P1 = bit 1
    ...
    P7 = bit 7
    0b10101010 = P0 off, P1 on, P2 off, P3 on, P4 off, P5 on, P6 off, P7 on
    
    Examples:
    set bit 5:
    set_uja1023_output_bits(1 << 5); //turn on any pins that = 1, leave all other pins alone
    
    clear bit 5:
    clear_uja1023_output_bits(1 << 5); //turn off any pins that = 1, leave all other pins alone
    
    Or set the whole buffer at once:
    set_uja1023_output_buffer(0xFF); //turn all output pins on
    set_uja1023_output_buffer(0x00); //turn all output pins off
    */
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

