#include <arduino.h>

#include "midget_spinner_cfg.h"

#include "radio_decoder.h"
#include "radio_decoder_cfg.h"

typedef enum
{
  RX_HISTORY_0
  ,RX_HISTORY_1
  ,NUM_OF_RX_PULSES_HISTORY 
}rx_channels;

static const unsigned int  UI16_PROTOCOL_IDLE_TIMEOUT  = (unsigned int)K_PROTOCOL_IDLE_TIMEOUT;
static const unsigned int  UI16_MIN_PULSE_WIDTH        = (unsigned int)K_MIN_PULSE_WIDTH;
static const unsigned int  UI16_MAX_PULSE_WIDTH        = (unsigned int)K_MAX_PULSE_WIDTH;
static const unsigned char UI8_INTERRUPT_PIN           = (unsigned char)K_INTERRUPT_PIN;

static unsigned char ui8_pulse_counter;
static unsigned long ui32_previous_protocol_stamp;
static unsigned long ui32_current_protocol_stamp;
static unsigned long ui32_elapsed_protocol_idle;
static unsigned long ui32_pulse_width_stamp;
static unsigned long ui32_pulse_width;
static unsigned int  ui16_rx_pulses[NUM_OF_RX_CHANNELS];
static unsigned int  ui16_rx_pulses_history[NUM_OF_RX_CHANNELS][NUM_OF_RX_PULSES_HISTORY];

static unsigned int threeTapMedianFilter(unsigned int ui16_value, unsigned char ui8_channel);

/* initialize radio receiver */
void initRadioDecoder(void)
{
  pinMode(UI8_INTERRUPT_PIN, INPUT);
  attachInterrupt(INT2, decodeRadio, CHANGE);
}

/* decode incoming rx pulses to individual channels */
void decodeRadio(void)
{
  /* check if timeout */
  ui32_current_protocol_stamp = micros();
  ui32_elapsed_protocol_idle = ui32_current_protocol_stamp - ui32_previous_protocol_stamp;
  
  if(ui32_elapsed_protocol_idle > UI16_PROTOCOL_IDLE_TIMEOUT)
  {
    ui8_pulse_counter = 1;                                    // reset to channel 1
  }

  /* check start of pulse */
  if(HIGH == digitalRead(UI8_INTERRUPT_PIN))
  {
    ui32_pulse_width_stamp = micros();
  }
  /* check if end of pulse */
  else
  {
    ui32_pulse_width = micros() - ui32_pulse_width_stamp;

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
    
    ui16_rx_pulses[ui8_pulse_counter-1] = threeTapMedianFilter(ui32_pulse_width, (ui8_pulse_counter-1));
    ui8_pulse_counter++;                                      // next channel
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
