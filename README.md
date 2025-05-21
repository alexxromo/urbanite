# Urbanite Project

## Author

* **Alex Martin-Romo Gonzalez** - email: [alex.martin-romo@alumnos.upm.es](mailto:alex.martin-romo@alumnos.upm.es)

Urbanite es un proyecto que busca desarrollar un sensor de aparcamiento utilizando la placa STM32 y un sensor de ultrasonidos. Este sistema permite detectar la distancia a objetos cercanos, brindando asistencia al aparcar mediante un indicador visual o sonoro que advierte al conductor sobre posibles obstáculos.

Urbanite is a project focused on developing a parking sensor using the STM32 board and an ultrasonic sensor. The system detects the distance to nearby objects, providing parking assistance through visual or audio cues that alert the driver of potential obstacles.

## Version 1

In Version 1, the system works with the user button only. The user button is connected to the pin PC13. The code uses the EXTI13 interrupt to detect the button press.

## Version 2

In Version 2, the system adds the ultrasonic transceiver to measure the distance to an object. The trigger pin is connected to the pin PB0, and the echo pin is connected to the pin PA1. The code uses the TIM2, TIM3 and TIM5 timers to control the ultrasonic transceiver.

To measure the distance in centimeters with a timer resolution of 1 microseconds, we can say that 1 cm is equivalent to 58.3 microseconds. The speed of sound is 343 m/s at 20ºC. The ultrasonic transceiver is the HC-SR04.

## Version 3

## Version 4






**Los fallos y bugs no detectados serán corregidos en las proximas versiones**