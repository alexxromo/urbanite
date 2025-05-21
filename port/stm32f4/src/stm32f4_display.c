/**
 * @file stm32f4_display.c
 * @brief Portable functions to interact with the display system FSM library. All portable functions must be implemented in this file.
 * @author Alex Martin-Romo Gonzalez (alex.martin-romo@alumnos.upm.es)
 * @date 23/05/2025
 */


 //TODO falta completar el test_port_display



/* Standard C includes */
#include <stdio.h>

/* HW dependent includes */
#include "port_display.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_display.h"
#include <math.h>

/* Microcontroller dependent includes */

/* Defines --------------------------------------------------------------------*/

/**
 * @brief Array of elements that represents the HW characteristics of the RGB LED of the display systems connected to the STM32F4 platform.

This must be hidden from the user, so it is declared as static. To access the elements of this array, use the function _stm32f4_display_get()
 * 
 */


/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Structure to define the HW dependencies of an RGB LED
 * 
 */
typedef struct
{
    GPIO_TypeDef * 	p_port_red; /*!<GPIO where the RED LED is connected*/
    uint8_t 	pin_red; /*!<Pin/line where the RED LED is connected*/
    GPIO_TypeDef * 	p_port_green; /*!<GPIO where the GREEN LED is connected*/
    uint8_t 	pin_green; /*!<Pin/line where the GREEN LED is connected*/
    GPIO_TypeDef * 	p_port_blue; /*!<GPIO where the BLUE LED is connected*/
    uint8_t 	pin_blue; /*!<Pin/line where the BLUE LED is connected*/

}stm32f4_display_hw_t;

/**
 * @brief Array of elements that represents the HW characteristics of the RGB LED of the display systems connected to the STM32F4 platform.
 * 
 */
static stm32f4_display_hw_t displays_arr[]={
    [PORT_REAR_PARKING_DISPLAY_ID] = {
        .p_port_red = STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO,
        .pin_red = STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN,
        .p_port_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO,
        .pin_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN,
        .p_port_blue = STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO,
        .pin_blue = STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN,
    
    },                                                                          
};

/* Global variables */

/* Private functions -----------------------------------------------------------*/
/**
 * @brief Get the display struct with the given ID.
 * 
 * @param display_id Button ID.
 * @return stm32f4_display_hw_t*  Pointer to the display struct. NULL If the display ID is not valid.
 */
stm32f4_display_hw_t *_stm32f4_display_get(uint32_t display_id)
{   // Return the pointer to the display with the given ID. If the ID is not valid, return NULL.
    // TODO alumnos

    if (display_id >= (sizeof(displays_arr) / sizeof(displays_arr[0]))) {
        return NULL;  // Retorna NULL si el ID es inválido
    }
    return &displays_arr[display_id];  //Devuelve el puntero a la estructura correspondiente
}

/* Public functions -----------------------------------------------------------*/

/**
 * @brief Configure the timer that controls the PWM of each one of the RGB LEDs of the display system.

This function is called by the port_display_init() public function to configure the timer that controls the PWM of the RGB LEDs of the display.
 * 
 * @param display_id Display system identifier number.
 */
void _timer_pwm_config	(uint32_t display_id){//TODO si da fallos cambiar el apb1 por apb2
    /* 1) Enable clock for TIM4 (APB1) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    /* 2) Disable counter and enable auto-reload preload */
    TIM4->CR1 &= ~TIM_CR1_CEN;
    TIM4->CR1 |= TIM_CR1_ARPE;

    /* 3) Reset counter */
    TIM4->CNT = 0;

    /* Compute PSC and ARR for 50 Hz (20 ms period) */
    double fclk = (double)SystemCoreClock;
    double t_pwm = 1.0 / 50.0;               // 50 Hz → 20 ms
    double arr_max = 65535.0;

    double psc_temp = (fclk * t_pwm) / (arr_max + 1.0) - 1.0;
    psc_temp = round(psc_temp);

    double arr_temp = (fclk * t_pwm) / (psc_temp + 1.0) - 1.0;
    arr_temp = round(arr_temp);

    while (arr_temp > arr_max) {
        psc_temp += 1.0;
        arr_temp = (fclk * t_pwm) / (psc_temp + 1.0) - 1.0;
        arr_temp = round(arr_temp);
    }

    TIM4->PSC = (uint32_t)psc_temp;
    TIM4->ARR = (uint32_t)arr_temp;

    /* 5) Disable output compare channels */
    TIM4->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);

    /* 6) Clear polarity bits */
    TIM4->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP |
                    TIM_CCER_CC2P | TIM_CCER_CC2NP |
                    TIM_CCER_CC3P | TIM_CCER_CC3NP |
                    TIM_CCER_CC4P | TIM_CCER_CC4NP);

    /* 7) Set PWM mode 1 and enable preload for each channel */
    TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
    TIM4->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE);
    TIM4->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE);

    TIM4->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M);
    TIM4->CCMR2 |= (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE);
    TIM4->CCMR2 |= (TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE);

    /* 8) Generate update event to load PSC and ARR */
    TIM4->EGR |= TIM_EGR_UG;

    /* 9) Disable the timer after configuration */
    TIM4->CR1 &= ~TIM_CR1_CEN;
    
}

/**
 * @brief Configure the HW specifications of a given display.

Assuming we are using an STM32F4-based platform, this function must call the following functions:
 * 
 * @param display_id This index is used to select the element of the displays_arr[] array
 */
void port_display_init	(uint32_t display_id){
    /* 1) Get the display struct */
    stm32f4_display_hw_t *p_display = _stm32f4_display_get(display_id);

    if (p_display == NULL) {
        return; // Exit if the display ID is invalid
    }

    /* 2) Configure the GPIO pins for the RGB LED */
    stm32f4_system_gpio_config(p_display->p_port_red, p_display->pin_red, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config(p_display->p_port_green, p_display->pin_green, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config(p_display->p_port_blue, p_display->pin_blue, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);

    /*Call function stm32f4_system_gpio_config_alternate() with the right arguments to configure the alternate function of the each RGB LED.*/
    stm32f4_system_gpio_config_alternate(p_display->p_port_red, p_display->pin_red, STM32F4_GPIO_MODE_AF);//STM32F4_GPIO_AF_TIM4
    stm32f4_system_gpio_config_alternate(p_display->p_port_green, p_display->pin_green, STM32F4_GPIO_MODE_AF);
    stm32f4_system_gpio_config_alternate(p_display->p_port_blue, p_display->pin_blue, STM32F4_GPIO_MODE_AF);

    /*Call function _timer_pwm_config() to configure the timer and the PWM signal of the display.*/
    _timer_pwm_config(display_id);

    /*Call function port_display_set_rgb() to set the RGB LED to off.*/
    port_display_set_rgb (display_id, COLOR_OFF);
}

/**
 * @brief Set the Capture/Compare register values for each channel of the RGB LED given a color.

This function disables the timer associated to the RGB LEDs, sets the Capture/Compare register values for each channel of the RGB LED, and enables the timer.
 * 
 * @param display_id Display system identifier number.
 * @param color RGB color to set.
 */
void port_display_set_rgb (uint32_t display_id, rgb_color_t color){
    
    /* 1) Leer componentes */
    uint8_t r = color.r;
    uint8_t g = color.g;
    uint8_t b = color.b;    

    /* 2) Comprobar display_id */
    if (display_id != PORT_REAR_PARKING_DISPLAY_ID) {
    return;
    }

    /*3) Desactivar el temporizador */
    TIM4->CR1 &= ~TIM_CR1_CEN;



    /* 4) Si todos son cero, desactivar todos los canales y saltar a FIN */
    if (r == 0 && g == 0 && b == 0) {
    TIM4->CCER &= ~(TIM_CCER_CC1E    /* Rojo */
                  | TIM_CCER_CC2E    /* Verde */
                  | TIM_CCER_CC4E);  /* Azul */
    /* 7) FIN: forzar actualización*/
        TIM4->EGR |= TIM_EGR_UG;
    return;
}

uint32_t arr = TIM4->ARR;        // <-- capturamos ARR
uint32_t max = arr + 1;          // total de pasos

    /* 5) Canal Rojo (CCR1 / CC1E) */
    if (r == 0) {
        TIM4->CCER &= ~TIM_CCER_CC1E;   // Deshabilitar el canal rojo si el valor es 0
    } else {
        TIM4->CCR1 = (uint32_t)r * max / 255;                // Asignar el valor de r al registro CCR1
        TIM4->CCER |= TIM_CCER_CC1E;    // Habilitar el canal rojo
    }

/* 6) Canal Verde (CCR3 / CC3E) */
if (g == 0) {
    TIM4->CCER &= ~TIM_CCER_CC3E;
} else {
    TIM4->CCR3 = (uint32_t)g * max / 255;
    TIM4->CCER |= TIM_CCER_CC3E;
}

/* 7) Canal Azul (CCR4 / CC4E) */
if (b == 0) {
    TIM4->CCER &= ~TIM_CCER_CC4E;
} else {
    TIM4->CCR4 = (uint32_t)b * max / 255;
    TIM4->CCER |= TIM_CCER_CC4E;
}

/* 8) Forzar carga de PSC/ARR e iniciar el timer */
TIM4->EGR |= TIM_EGR_UG;
TIM4->CR1 |= TIM_CR1_CEN;
}