#include "Timer.h"
#include "Message.h"


enum State
{
  SYNCING,
  SENDING
};

#define BYTE_LEN    8
#define LASER       4
#define PAYLOAD_LEN 100

State       currentState      = SYNCING;
volatile    boolean txState   = false;
const char  frameStart        = B00111100;
const char  frameEnd          = B00111110;

void clock();

Timer t = Timer(500, clock);

void setup()
{
  Serial.begin(115200);

  pinMode(LASER, OUTPUT);
  digitalWrite(LASER, LOW);
  
  delay(5000);
  t.startClock();
}

void loop()
{
  switch (currentState)
  {
    case SYNCING:
      Serial.println("----- SYNCING ------");
      sync();
      break;

    case SENDING:
      Serial.println("----- SENDING ------");
      char * message = getInput();
      send(message);
      // send(testMsg2);
      break;
  }
}

char * getInput()
{
  char buffer[100]="";
  int len = Serial.available();
  if (len > 0)
  {
    Serial.readBytes(buffer, len);
    buffer[len] = '\0';
    return buffer;
  }
  return testMsg2;
}

void send(char * message)
{
  enframe(message);
}

void enframe(char *input)
{
  int     end;
  int     bitIdx;
  size_t  offset;
  size_t  inputLen;
  char    txBuffer[PAYLOAD_LEN+3];

  inputLen  = strlen(input);
  offset    = 0;

  while (offset < inputLen)
  {
    txBuffer[0] = frameStart;
    strncpy(txBuffer+1, input+offset, PAYLOAD_LEN);
    
    end = min(PAYLOAD_LEN+1, strlen(txBuffer));
    
    txBuffer[end]     = frameEnd;
    txBuffer[end + 1] = '\0';

    offset += PAYLOAD_LEN;

    sync();

    bitIdx = 0;
    while (bitIdx < PAYLOAD_LEN+2)
    {
      sendChar(txBuffer[bitIdx]);
      bitIdx++;
    }
  }
}

void sendChar(char byte)
{
  int bitCount;

  bitCount = 0;
  while (bitCount < BYTE_LEN)
  {
    if (txState == true)
    {
      txState = false;
      digitalWrite(LASER, bitRead(byte, bitCount));
      bitCount++;
    }
  }
}


void sync()
{ 
  sendChar(B00000000);
  sendChar(B10000000);

  currentState = SENDING;
}

void clock()
{
  txState = true;
}