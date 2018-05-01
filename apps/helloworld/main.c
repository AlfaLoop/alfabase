#include "alfabase.h"

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
      logger->printf(LOG_RTT, "Hello Timer %d\n", counter);
    }
  }
  return 0;
}
