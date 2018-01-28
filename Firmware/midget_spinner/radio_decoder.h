#ifndef RADIO_DECODER
#define RADIO_DECODER

typedef enum
{
  RX_CHANNEL1             // Roll
  ,RX_CHANNEL2            // Pitch
  ,RX_CHANNEL3            // Throttle
  ,RX_CHANNEL4            // Yaw
  ,RX_CHANNEL5            // Arm/Disarm
  ,RX_CHANNEL6
  ,NUM_OF_RX_CHANNELS  
}rx_channels_et;

void initRadioDecoder(void);
void decodeRadio(void);
void getRxData(unsigned int* ui16_rx_data_destination);
unsigned long getPreviousRadioStamp(void);

#endif

