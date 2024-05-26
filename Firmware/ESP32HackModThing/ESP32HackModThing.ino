/*
  // Simple DIY Electronic Music Projects
  //    diyelectromusic.wordpress.com
  //
  //  ESP32 Modular Hack Thing
  //  https://diyelectromusic.wordpress.com/2024/05/07/educational-diy-synth-thing/
  //
      MIT License

      Copyright (c) 2024 diyelectromusic (Kevin)

      Permission is hereby granted, free of charge, to any person obtaining a copy of
      this software and associated documentation files (the "Software"), to deal in
      the Software without restriction, including without limitation the rights to
      use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
      the Software, and to permit persons to whom the Software is furnished to do so,
      subject to the following conditions:

      The above copyright notice and this permission notice shall be included in all
      copies or substantial portions of the Software.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
      FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
      COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHERIN
      AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
      WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "task0.h"
#include "task1.h"

//#define S_DEBUG 1
#ifdef S_DEBUG
#warning Make sure GPIO 4 is not used as an oscillator to allow printing
#endif

void core0Entry (void *pvParameters);
TaskHandle_t core0Task;
void core1Entry (void *pvParameters);
TaskHandle_t core1Task;

void setup () {
#ifdef S_DEBUG
  Serial.begin(115200);
#endif
  delay(300);
  // NB: Need to start MIDI on Task 1 before PWM on Task 0 as TX is reused as a PWM output...
  xTaskCreatePinnedToCore(core1Entry, "Core 1 Task", 2048, NULL, 1, &core1Task, 1);
  delay(300);
  xTaskCreatePinnedToCore(core0Entry, "Core 0 Task", 4096, NULL, 1, &core0Task, 0);
}

void loop () {
  // No main loop here
}

void core0Entry (void *pvParameters) {
  Task0Setup();

  for (;;) {
    Task0Loop();
    vTaskDelay(1); // Allow FreeRTOS IDLE to maintain watchdog
  }
}

void core1Entry (void *pvParameters) {
  Task1Setup();

  for (;;) {
    Task1Loop();
    vTaskDelay(1); // Allow FreeRTOS IDLE to maintain watchdog
  }
}
