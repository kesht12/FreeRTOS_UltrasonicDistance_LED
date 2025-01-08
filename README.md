# **Distance Monitoring with LED Control System**

A simple FreeRTOS-based project that reads distance values from a HCSR04 ultrasonic sensor and controls ESP32 built-in RGB LED with proportional brightness, while also keeping track of the average distance values. 

## **Code Overview**

1. We first read new distance given by HCSR04 sensor every 500 ms and put that value into a FreeRTOS Queue
2. We then calculate the running average distance over the last five values, which is protected using a mutex.
3. Using the distance received from the queue, we calculate a proportional brightness for the LED.
4. When the distance exceeds an upper bound, the LED changes from green to red with a set brightness.   

## **Flow Chart**

![alt text](https://github.com/kesht12/FreeRTOS_UltrasonicDistance_LED/blob/main/FlowChartHCSR04.jpeg)

## **Concepts Used**
1. Queues: FreeRTOS queues to store and read distance values
2. Synchronization Primitives: Used a mutex to protect average distance value, which was calculated in task 1 and used by software timer callback function
3. Task Management: Concurrently ran multiple FreeRTOS tasks

## **Hardware**

1. ESP32-S3
2. HCSR04 Temperature Sensor
3. Wires

## **Software**

1. Arduino IDE









 
