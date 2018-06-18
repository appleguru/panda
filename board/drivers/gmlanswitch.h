#ifdef PANDA

int timeout_counter = 500; //We have a 33kbps timer to send data to the GMLAN transceiver, so every 500 loops is about 15ms (just under the transceiver's 17ms timeout)
int inverted_bit_to_send = 1; //0 or 1. 0 is high (dominant), 1 is low

void TIM4_IRQHandler(void) {
  if (TIM4->SR & TIM_SR_UIF) {
    if (timeout_counter == 0) {
      //Send a 1 (bus low) every 15ms to reset the GMLAN transceivers timeout
      timeout_counter = 500;
      set_gpio_output(GPIOB, 13, 1);
    }
    else {
      set_gpio_output(GPIOB, 13, inverted_bit_to_send);
      timeout_counter--;
    }
  }
  TIM4->SR = 0;
}

void gmlan_switch_init(void) {
  set_gpio_mode(GPIOB, 13, MODE_OUTPUT);
  
  // setup
  TIM4->PSC = 48-1;          // tick on 1 us
  TIM4->CR1 = TIM_CR1_CEN;   // enable
  TIM4->ARR = 30-1;          // 33.3 kbps

  // in case it's disabled
  NVIC_EnableIRQ(TIM4_IRQn);

  // run the interrupt
  TIM4->DIER = TIM_DIER_UIE; // update interrupt
  TIM4->SR = 0;
}

void set_gmlan_digital_output(int to_set) {
  inverted_bit_to_send = to_set;
  
  /*
  puts("Writing ");
  puth(inverted_bit_to_send);
  puts("\n");
  */
}

#endif

