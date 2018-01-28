#include "radio_decoder.h"
#include "radio_decoder_cfg.h"

#include "weapon.h"
#include "weapon_cfg.h"

#include "rc_mobot.h"
#include "rc_mobot_cfg.h"

#include "midget_spinner_cfg.h"

static const unsigned int  UI16_LOOP_CYCLE              = (unsigned int)K_LOOP_CYCLE;
static const unsigned int  UI16_NO_SIGNAL_TIMEOUT       = (unsigned int)K_NO_SIGNAL_TIMEOUT;

static unsigned long ui32_cycle_stamp;
static unsigned long ui32_current_protocol_stamp;
static unsigned long ui32_elapsed_protocol_idle;
static unsigned int  ui16_rx_pulses[NUM_OF_RX_CHANNELS];

void setup() 
{
  initRadioDecoder();
  Serial.begin(9600);
  initWeapon();
  initMobot();
  ui32_cycle_stamp = millis();
}

void loop() 
{
  if((millis()-ui32_cycle_stamp) > UI16_LOOP_CYCLE)
  {
    ui32_cycle_stamp = millis();
    ui32_elapsed_protocol_idle = micros() - getPreviousRadioStamp();
    getRxData(&ui16_rx_pulses[0]);
      
    /* failsafe mode */
    if(ui32_elapsed_protocol_idle > UI16_NO_SIGNAL_TIMEOUT)
    {
      Serial.println("No received data");
      initWeapon();
      initMobot();
    }
    /* active mode */
    else
    {
      Serial.print("Channel 1:");
      Serial.println(ui16_rx_pulses[RX_CHANNEL1]);
      
      Serial.print("Channel 2:");
      Serial.println(ui16_rx_pulses[RX_CHANNEL2]);
      
      Serial.print("Channel 3:");
      Serial.println(ui16_rx_pulses[RX_CHANNEL3]);
      
      Serial.print("Channel 4:");
      Serial.println(ui16_rx_pulses[RX_CHANNEL4]);    
  
      Serial.print("Channel 5:");
      Serial.println(ui16_rx_pulses[RX_CHANNEL5]);
      
      Serial.print("Channel 6:");
      Serial.println(ui16_rx_pulses[RX_CHANNEL6]);  
  
      weaponCycle(ui16_rx_pulses[RX_CHANNEL3], ui16_rx_pulses[RX_CHANNEL4], ui16_rx_pulses[RX_CHANNEL4]);
      mobotCycle(ui16_rx_pulses[RX_CHANNEL2], ui16_rx_pulses[RX_CHANNEL1]);
    }    
  }
}



