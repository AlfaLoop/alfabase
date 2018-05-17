#include "alfabase.h"

#if defined(BOARD_PCA10040)
#define LED0                        17
#define LED1                        18
#define LED2                        19
#define LED3                        20
#endif

static bool timer_flag = false;
static uint32_t counter = 0;

static void
timer_event_handler(void)
{
  timer_flag = true;
}
/****************************************************************/
int main(void)
{
  Process *process = OSProcess();
  Logger *logger = OSLogger();
  Gpio *gpio = HWGpio();
  gpio->setup(LED0, GPIO_OUTPUT);
  gpio->setup(LED1, GPIO_OUTPUT);
  gpio->setup(LED2, GPIO_OUTPUT);
  gpio->setup(LED3, GPIO_OUTPUT);
  gpio->output(LED0, GPIO_OUTPUT_LOW);
  gpio->output(LED1, GPIO_OUTPUT_LOW);
  gpio->output(LED2, GPIO_OUTPUT_LOW);
  gpio->output(LED3, GPIO_OUTPUT_LOW);
  // Get the timer instance 0
  Timer *t = OSTimer();
  // Timer 0
  t->start(0, 1000, timer_event_handler);
  while (1) {
    // Power saving
    process->waitForEvent();
    if (timer_flag) {
      timer_flag = false;
      counter++;
      if (counter % 2 == 0) {
        gpio->output(LED0, GPIO_OUTPUT_LOW);
        gpio->output(LED1, GPIO_OUTPUT_LOW);
        gpio->output(LED2, GPIO_OUTPUT_LOW);
        gpio->output(LED3, GPIO_OUTPUT_LOW);
      } else {
        gpio->output(LED0, GPIO_OUTPUT_HIGH);
        gpio->output(LED1, GPIO_OUTPUT_HIGH);
        gpio->output(LED2, GPIO_OUTPUT_HIGH);
        gpio->output(LED3, GPIO_OUTPUT_HIGH);
      }
      logger->printf(LOG_RTT, "Hello Timer %d\n", counter);
    }
  }
  return 0;
}
