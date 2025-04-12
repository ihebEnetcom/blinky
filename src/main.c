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
 #define DEBOUNCE_MS     50
 
 /* Original LED configuration */
 #define LED_GPIO_NODE DT_NODELABEL(gpio0)
 #define LED_PIN 2
 
 /* New switch and LED configuration */
 #define SWITCH_PIN 5
 #define LED2_PIN 4
 
 static const struct gpio_dt_spec led = {
	 .port = DEVICE_DT_GET(LED_GPIO_NODE),
	 .pin = LED_PIN,
	 .dt_flags = GPIO_ACTIVE_LOW
 };
 
 static const struct gpio_dt_spec led2 = {
	 .port = DEVICE_DT_GET(LED_GPIO_NODE),
	 .pin = LED2_PIN,
	 .dt_flags = GPIO_ACTIVE_LOW
 };
 
 static const struct gpio_dt_spec switch_input = {
	 .port = DEVICE_DT_GET(LED_GPIO_NODE),
	 .pin = SWITCH_PIN,
	 .dt_flags = GPIO_ACTIVE_LOW | GPIO_PULL_UP
 };
 
 /* Thread definitions */
 #define SWITCH_THREAD_STACK_SIZE 1024
 #define SWITCH_THREAD_PRIORITY 7
 K_THREAD_STACK_DEFINE(switch_thread_stack, SWITCH_THREAD_STACK_SIZE);
 static struct k_thread switch_thread_data;
 
 /* Thread function for the switch and second LED */
 void switch_led_thread(void *arg1, void *arg2, void *arg3)
 {
	 int ret;
	 int switch_state;
	 int prev_state = -1;
 
	 ARG_UNUSED(arg1);
	 ARG_UNUSED(arg2);
	 ARG_UNUSED(arg3);
 
	 /* Configure second LED */
	 ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
	 if (ret < 0) {
		 printf("Error %d: Failed to configure LED2 pin %d\n", ret, LED2_PIN);
		 return;
	 }
 
	 /* Configure switch input */
	 ret = gpio_pin_configure_dt(&switch_input, GPIO_INPUT);
	 if (ret < 0) {
		 printf("Error %d: Failed to configure switch pin %d\n", ret, SWITCH_PIN);
		 return;
	 }
 
	 printf("Switch-LED control thread started\n");
 
	 while (1) {
		 /* Read the switch state */
		 switch_state = gpio_pin_get_dt(&switch_input);
		 
		 if (switch_state < 0) {
			 printf("Error %d: Failed to read switch\n", switch_state);
		 } else if (switch_state != prev_state) {
			 /* Update LED to match switch state */
			 printf("Switch changed to: %s\n", switch_state ? "ON" : "OFF");
			 ret = gpio_pin_set_dt(&led2, switch_state);
			 if (ret < 0) {
				 printf("Error %d: Failed to set LED2\n", ret);
			 }
			 prev_state = switch_state;
		 }
		 
		 /* Small delay for debouncing and CPU relief */
		 k_msleep(DEBOUNCE_MS);
	 }
 }
 
 int main(void)
 {
	 int ret;
	 bool led_state = true;
 
	 /* Check if GPIO devices are ready */
	 if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&switch_input)) {
		 printf("Error: GPIO devices not ready\n");
		 return 0;
	 }
 
	 /* Configure the original LED */
	 ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	 if (ret < 0) {
		 printf("Error %d: Failed to configure GPIO pin %d\n", ret, LED_PIN);
		 return 0;
	 }
 
	 /* Start the switch-LED thread */
	 k_thread_create(&switch_thread_data, switch_thread_stack,
					K_THREAD_STACK_SIZEOF(switch_thread_stack),
					switch_led_thread, NULL, NULL, NULL,
					SWITCH_THREAD_PRIORITY, 0, K_NO_WAIT);
 
	 printf("ESP32 Blinky Example with Switch Control\n");
	 printf("Main thread: blinking LED1, Second thread: switch controlling LED2\n");
 
	 /* Original blinky functionality in main thread */
	 while (1) {
		 ret = gpio_pin_toggle_dt(&led);
		 if (ret < 0) {
			 printf("Error %d: Failed to toggle LED pin %d\n", ret, LED_PIN);
			 return 0;
		 }
 
		 led_state = !led_state;
		 printf("LED1 state: %s\n", led_state ? "ON" : "OFF");
		 k_msleep(SLEEP_TIME_MS);
	 }
	 return 0;
 }