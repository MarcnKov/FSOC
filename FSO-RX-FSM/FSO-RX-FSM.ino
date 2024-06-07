#include "Timer.h"

enum State
{
  SYNCING,
  RECEIVING,
  WAITING
};

#define REF_VOLTAGE                 5.0
#define THRESHOLD_mV               0.05
#define LOST_SAMPLES_TH           10000
#define FREQUENCY                   500
#define PHOTO_DIODE                  A3
#define BYTE_LEN                      8
#define __vo                   volatile

char                             rxByte;
const char       frameStart = B00111100;
const char       frameEnd   = B00111110;
const char       sync0Seq   = B00000000;
const char       sync1Seq   = B10000000;

State currentState          =   WAITING;
__vo  boolean    rxState    =     false;
__vo  boolean    signalLost =     false;
int              lostSamples=        0;


void clock(void);
void RX_Data(void);
void sync(void);
char receiveByte(void);

Timer t = Timer(FREQUENCY, clock);

void setup()
{
  Serial.begin(115200);
  Serial.println("============= Receiver ============");

  pinMode(PHOTO_DIODE, INPUT);
  t.startClock();
  sync();
}


void loop()
{

  switch (currentState)
  {
    case WAITING:
      waitForSignal();
    break;

    case RECEIVING:
      RX_Data();    
    break;

    case SYNCING:
      sync();
    break;

  }
}

void RX_Data(void)
{
  rxByte = receiveByte();
  if (rxByte != frameStart && rxByte != frameEnd && rxByte != sync0Seq)
  {
    Serial.write(rxByte);
    currentState = RECEIVING;
  }
  else if (rxByte == sync0Seq || rxByte == sync1Seq)
  {
    currentState = SYNCING;
  }
}


char receiveByte(void)
{
  int bitcount = 0;
  char c;
  while (bitcount < BYTE_LEN)
  {
    if (rxState == true)
    {
      rxState = false;

      bitWrite(c, bitcount, getBit());
      bitcount++;
    }
  }
  return c;
}

int getBit()
{
  int             i;
  int             sampleBits;
  int             numberOfSamples;

  numberOfSamples = 4;
  sampleBits      = 0;
  i               = 0;

  while(i < numberOfSamples)
  {
    if (PD_Voltage() > THRESHOLD_mV)
    {
      sampleBits += 1;
      lostSamples  = 0;
    }
    else
    {
      lostSamples++;
    }
    i++;
  }

  if (sampleBits >= numberOfSamples/2)
    return 1;
  else
    return 0;
}

double PD_Voltage()
{
  return analogRead(PHOTO_DIODE) * (REF_VOLTAGE / 1023.0);
}

void sync(void)
{
  waitForSignal();
  t.resetClock();
  rxState = false;
}

void waitForSignal(void)
{
  while (PD_Voltage() <= THRESHOLD_mV)
  {
    lostSamples++;
    if (lostSamples >= LOST_SAMPLES_TH)
    {
      Serial.println("\n============= NO SIGNAL ============\n");
      lostSamples = 0;
    }
  }
  currentState = RECEIVING;
}

void clock(void)
{
  rxState = true;
}