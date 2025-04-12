/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <stdio.h>
 #include <zephyr/kernel.h>
 #include <zephyr/drivers/gpio.h>
 
 /* 1000 msec = 1 sec */
 #define SLEEP_TIME_MS   1000
 
 /* Use GPIO0 controller and pin 2 which is typically connected to the built-in LED */
 #define LED_GPIO_NODE DT_NODELABEL(gpio0)
 #define LED_PIN 2
 
 static const struct gpio_dt_spec led = {
	 .port = DEVICE_DT_GET(LED_GPIO_NODE),
	 .pin = LED_PIN,
	 .dt_flags = GPIO_ACTIVE_LOW  /* Most ESP32 DevKitC boards use active low for the LED */
 };
 
 int main(void)
 {
	 int ret;
	 bool led_state = true;
 
	 if (!gpio_is_ready_dt(&led)) {
		 printf("Error: GPIO device %s is not ready\n", led.port->name);
		 return 0;
	 }
 
	 ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	 if (ret < 0) {
		 printf("Error %d: Failed to configure GPIO pin %d\n", ret, LED_PIN);
		 return 0;
	 }
 
	 printf("ESP32 Blinky Example\n");
 
	 while (1) {
		 ret = gpio_pin_toggle_dt(&led);
		 if (ret < 0) {
			 printf("Error %d: Failed to toggle GPIO pin %d\n", ret, LED_PIN);
			 return 0;
		 }
 
		 led_state = !led_state;
		 printf("LED state: %s\n", led_state ? "ON" : "OFF");
		 k_msleep(SLEEP_TIME_MS);
	 }
	 return 0;
 }