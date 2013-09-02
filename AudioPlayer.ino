/*
  Do stuff.
 */

#include <TimerOne.h>

#define SAMPLE_US (1000/11)

// DAC pins, MSB first
static const int pins[] = {12, 13, 14, 15, 16, -1};
// Underflow LED
static const int led_pin = 6;

// At 11kHz, 64 bytes should be more than enough to accomodate USB latency.
#define SAMPLE_BUF_SIZE 64
static volatile unsigned char sample_buf[SAMPLE_BUF_SIZE];
static int sample_head;
static int sample_tail;
static bool mute;
static volatile int num_samples;

static inline int sample_next(int n)
{
  return (n + 1) & (SAMPLE_BUF_SIZE - 1);
}

static void sample_push(unsigned char val)
{
  sample_buf[sample_head] = val;
  sample_head = sample_next(sample_head);
  // Disable interrupts to prevent race condition with the interrupt handler
  noInterrupts();
  num_samples++;
  interrupts();
}

static void
really_play(int count)
{
  unsigned char val;
  int i;

  while (count--) {
      if (mute) {
	  if (num_samples < SAMPLE_BUF_SIZE / 2)
	    return;
	  digitalWrite(led_pin, HIGH);
	  mute = false;
      } else if (num_samples == 0) {
	  digitalWrite(led_pin, LOW);
	  mute = true;
	  return;
      }
      num_samples--;
      val = sample_buf[sample_tail];
      sample_tail = sample_next(sample_tail);
  }
  i = 0;
  while (pins[i] >= 0){ 
      digitalWrite(pins[i], (val & 0x80) ? HIGH : LOW);
      i++;
      val <<= 1;
  }
}

static void
play_sample(void)
{
  static long tick;
  int n = 0;

  if (tick == 0)
    tick = micros();
  // Compensate for ticks missed when interrupts are disabled for a long time
  while ((long)(micros() - tick) > 0) {
      n++;
      tick += SAMPLE_US;
  }
  if (n > 0)
    really_play(n);
}

void loop()
{
  while (1) {
      // We can compare num_samples here because it will only ever be
      // decemented by the interrupt handler
      if (Serial.available() && num_samples < SAMPLE_BUF_SIZE) {
	  sample_push(Serial.read());
      }
  }
}

void setup()
{
  int n;
  for (n = 0; pins[n] >= 0; n++)
    pinMode(pins[n], OUTPUT);     
  pinMode(led_pin, OUTPUT);     
  digitalWrite(led_pin, HIGH);
  Serial.begin(115200);
  Timer1.initialize();
  Timer1.attachInterrupt(play_sample, SAMPLE_US);
}

