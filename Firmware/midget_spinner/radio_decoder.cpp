#include <arduino.h>

#include "midget_spinner_cfg.h"

#include "radio_decoder.h"
#include "radio_decoder_cfg.h"

typedef enum
{
  RX_HISTORY_0
  ,RX_HISTORY_1
  ,NUM_OF_RX_PULSES_HISTORY 
}rx_history_et;

static const unsigned char UI8_RX_PIN_CHANNEL1         = (unsigned char)K_RX_PIN_CHANNEL1;
static const unsigned char UI8_RX_PIN_CHANNEL2         = (unsigned char)K_RX_PIN_CHANNEL2;
static const unsigned char UI8_RX_PIN_CHANNEL3         = (unsigned char)K_RX_PIN_CHANNEL3;
static const unsigned char UI8_RX_PIN_CHANNEL4         = (unsigned char)K_RX_PIN_CHANNEL4;
static const unsigned char UI8_RX_PIN_CHANNEL5         = (unsigned char)K_RX_PIN_CHANNEL5;
static const unsigned char UI8_RX_PIN_CHANNEL6         = (unsigned char)K_RX_PIN_CHANNEL6;

static const unsigned int  UI16_PROTOCOL_IDLE_TIMEOUT  = (unsigned int)K_PROTOCOL_IDLE_TIMEOUT;
static const unsigned int  UI16_MIN_PULSE_WIDTH        = (unsigned int)K_MIN_PULSE_WIDTH;
static const unsigned int  UI16_MAX_PULSE_WIDTH        = (unsigned int)K_MAX_PULSE_WIDTH;

static unsigned char ui8_pulse_counter;
static unsigned char ui8_rx_port_state;
static unsigned long ui32_previous_protocol_stamp;
static unsigned long ui32_current_protocol_stamp;
static unsigned long ui32_elapsed_protocol_idle;
static unsigned long ui32_pulse_width;
static unsigned int  ui16_rx_pulses[NUM_OF_RX_CHANNELS];
static unsigned int  ui16_rx_pulses_history[NUM_OF_RX_CHANNELS][NUM_OF_RX_PULSES_HISTORY];

static void setupPinChangeInterrupt(unsigned char ui8_pin);
static unsigned int threeTapMedianFilter(unsigned int ui16_value, unsigned char ui8_channel);

/* ISR vector for pin change interrupt for RX_PIN_CHANNELS */
ISR(PCINT2_vect)    // handle pin change interrupt for D24-D31
{
  decodeRadio();
}

/* initialize radio receiver */
void initRadioDecoder(void)
{
  /* setup pin mode for RX_PIN_CHANNELS */
  pinMode(UI8_RX_PIN_CHANNEL1, INPUT);
  pinMode(UI8_RX_PIN_CHANNEL2, INPUT);
  pinMode(UI8_RX_PIN_CHANNEL3, INPUT);
  pinMode(UI8_RX_PIN_CHANNEL4, INPUT);
  pinMode(UI8_RX_PIN_CHANNEL5, INPUT);
  pinMode(UI8_RX_PIN_CHANNEL6, INPUT);
  
  /* setup pin change interrupt for RX_PIN_CHANNELS */
  setupPinChangeInterrupt(UI8_RX_PIN_CHANNEL1);
  setupPinChangeInterrupt(UI8_RX_PIN_CHANNEL2);
  setupPinChangeInterrupt(UI8_RX_PIN_CHANNEL3);
  setupPinChangeInterrupt(UI8_RX_PIN_CHANNEL4);
  setupPinChangeInterrupt(UI8_RX_PIN_CHANNEL5);
  setupPinChangeInterrupt(UI8_RX_PIN_CHANNEL6);
}

/* decode incoming rx pulses to individual channels */
void decodeRadio(void)
{
  ui32_current_protocol_stamp = micros();
  ui32_elapsed_protocol_idle  = ui32_current_protocol_stamp - ui32_previous_protocol_stamp;
  
  /* direct access to port input pins register to optimize for speed */
  ui8_rx_port_state = (PINC & 0xFC);
  
  switch(ui8_rx_port_state)
  {
    case 0x80:
    {
      ui8_pulse_counter = RX_CHANNEL1;
      break;
    }
    
    case 0x40:
    {
      ui8_pulse_counter = RX_CHANNEL2;
      break;
    }
    
    case 0x20:
    {
      ui8_pulse_counter = RX_CHANNEL3;
      break;
    }
    
    case 0x10:
    {
      ui8_pulse_counter = RX_CHANNEL4;
      break;
    }
    
    case 0x08:
    {
      ui8_pulse_counter = RX_CHANNEL5;
      break;
    }
    
    case 0x04:
    {
      ui8_pulse_counter = RX_CHANNEL6;
      break;
    }
    
    case 0x00:
    {
      /* all channels LOW, falling edge detected */
      ui32_pulse_width = ui32_elapsed_protocol_idle;
      
      /* check for saturation */
      if(ui32_pulse_width > UI16_MAX_PULSE_WIDTH)
      {
        ui32_pulse_width = UI16_MAX_PULSE_WIDTH;
      }
      else if(ui32_pulse_width < UI16_MIN_PULSE_WIDTH)
      {
        ui32_pulse_width = UI16_MIN_PULSE_WIDTH;
      }
      else
      {
        /* do nothing */
      }
      
      ui16_rx_pulses[ui8_pulse_counter] = threeTapMedianFilter(ui32_pulse_width, ui8_pulse_counter);
      break;
    }
    
    default:
    {
      break;
    }
  }
  
  ui32_previous_protocol_stamp = ui32_current_protocol_stamp;
}

unsigned long getPreviousRadioStamp(void)
{
  return ui32_previous_protocol_stamp;
}

void getRxData(unsigned int* ui16_rx_data_destination)
{
  unsigned char ui8_temp;
  
  for(ui8_temp = NUM_OF_RX_CHANNELS; ui8_temp == 0; ui8_temp--)
  {
    ui16_rx_data_destination[ui8_temp-1] = ui16_rx_pulses[ui8_temp-1];
  }
}

static void setupPinChangeInterrupt(unsigned char ui8_pin)
{
  *digitalPinToPCMSK(ui8_pin) |= bit(digitalPinToPCMSKbit(ui8_pin));
  PCIFR |= bit(digitalPinToPCICRbit(ui8_pin));
  PCICR |= bit(digitalPinToPCICRbit(ui8_pin));
}

static unsigned int threeTapMedianFilter(unsigned int ui16_value, unsigned char ui8_channel)
{
  unsigned int ui16_return_value;

  if(ui16_value > ui16_rx_pulses_history[ui8_channel][RX_HISTORY_0])
  {
    if(ui16_value > ui16_rx_pulses_history[ui8_channel][RX_HISTORY_1])
    {
      if(ui16_rx_pulses_history[ui8_channel][RX_HISTORY_1] > ui16_rx_pulses_history[ui8_channel][RX_HISTORY_0])
      {
        ui16_return_value = ui16_rx_pulses_history[ui8_channel][RX_HISTORY_0];
      }
      else
      {
        ui16_return_value = ui16_rx_pulses_history[ui8_channel][RX_HISTORY_1];
      }
    }
    else
    {
      ui16_return_value = ui16_value;
    }
  }
  else 
  {
    if(ui16_value < ui16_rx_pulses_history[ui8_channel][RX_HISTORY_1])
    {
      if(ui16_rx_pulses_history[ui8_channel][RX_HISTORY_0] < ui16_rx_pulses_history[ui8_channel][RX_HISTORY_1])
      {
        ui16_return_value = ui16_rx_pulses_history[ui8_channel][RX_HISTORY_0];
      }
      else
      {
        ui16_return_value = ui16_rx_pulses_history[ui8_channel][RX_HISTORY_1];
      }
    }
    else
    {
      ui16_return_value = ui16_value;
    }    
  }

  /* update history buffers */
  ui16_rx_pulses_history[ui8_channel][RX_HISTORY_1] = ui16_rx_pulses_history[ui8_channel][RX_HISTORY_0];
  ui16_rx_pulses_history[ui8_channel][RX_HISTORY_0] = ui16_value;
  return ui16_return_value;
}
