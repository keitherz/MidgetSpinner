const unsigned char UI8_INTERRUPT_PIN           = 2;
const unsigned int  UI16_NO_SIGNAL_TIMEOUT      = 20000;
const unsigned int  UI16_PROTOCOL_IDLE_TIMEOUT  = 5000;
const unsigned int  UI16_MIN_PULSE_WIDTH        = 1000;
const unsigned int  UI16_MAX_PULSE_WIDTH        = 2000;

typedef enum
{
  RX_CHANNEL1
  ,RX_CHANNEL2
  ,RX_CHANNEL3
  ,RX_CHANNEL4
  ,RX_CHANNEL5
  ,RX_CHANNEL6
  ,NUM_OF_RX_CHANNELS  
}rx_channels;

unsigned char ui8_pulse_counter;
unsigned long ui32_previous_protocol_stamp;
unsigned long ui32_current_protocol_stamp;
unsigned long ui32_elapsed_protocol_idle;
unsigned long ui32_pulse_width_stamp;
unsigned long ui32_pulse_width;
unsigned int  ui16_rx_pulses[NUM_OF_RX_CHANNELS];

void setup() 
{
  pinMode(UI8_INTERRUPT_PIN, INPUT);
  attachInterrupt(INT2, decodeReceiver, CHANGE);
  Serial.begin(9600);
}

void loop() 
{
  ui32_elapsed_protocol_idle = micros() - ui32_previous_protocol_stamp;

  /* transmitter is off */
  if(ui32_elapsed_protocol_idle > UI16_NO_SIGNAL_TIMEOUT)
  {
    Serial.println("No received data");
  }
  /* transmitter is on */
  else
  {
    // put your main code here, to run repeatedly:
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
  }
  delay(1000);
}

/* decode incoming rx pulses to individual channels */
void decodeReceiver()
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
    
    ui16_rx_pulses[ui8_pulse_counter-1] = ui32_pulse_width;
    ui8_pulse_counter++;                                      // next channel
  }
    
  ui32_previous_protocol_stamp = ui32_current_protocol_stamp;
}

void threeTapMedianFilter()
{
  
}

