/**
 * @file stm32f4_button.c
 * @brief Portable functions to interact with the button FSM library. All portable functions must
 * be implemented in this file.
 * @author Alex Martin-Romo Gonzalez (alex.martin-romo@alumnos.upm.es)
 * @author Salvador García Oviedo   (salvador.garcia@alumnos.upm.es)
 * @date 28/03/2025
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

/* HW dependent includes */
#include "port_button.h" // Used to get general information about the buttons (ID, etc.)q
#include "port_system.h" // Used to get the system tick
#include <stdio.h>
#include "stm32f4_system.h"
#include "stm32f4_button.h"
//#include "stm32f4xx.h"


/* Microcontroller dependent includes */
// TO-DO alumnos: include the necessary files to interact with the GPIOs


/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Structure to define the HW dependencies of a button status.
 * 
 */
typedef struct
{
    GPIO_TypeDef *p_port; /*!<GPIO where the button is connected*/
    uint8_t pin;    /*!<Pin/line where the button is connected*/
    uint8_t pupd_mode; /*!<Pull-up/Pull-down mode*/
    bool flag_pressed; /*!<Flag to indicate that the button has been pressed*/
} stm32f4_button_hw_t;


/* Global variables ------------------------------------------------------------*/

/**
 * @brief Array of elements that represents the HW characteristics of the buttons connected to the STM32F4 platform.

This must be hidden from the user, so it is declared as static. To access the elements of this array, use the function
_stm32f4_button_get()
 * 
 * @hideinitializer
 */
static stm32f4_button_hw_t buttons_arr[] ={
    [PORT_PARKING_BUTTON_ID] = {.p_port = STM32F4_PARKING_BUTTON_GPIO, .pin = STM32F4_PARKING_BUTTON_PIN, .pupd_mode =
        STM32F4_GPIO_PUPDR_NOPULL},                                                                          
};

/* Private functions ----------------------------------------------------------*/


/**
 * @brief Get the button status struct with the given ID.
 *
 * @param button_id Button ID.
 *
 * @return Pointer to the button state struct.
 * @return NULL If the button ID is not valid.
 */ //TODO
stm32f4_button_hw_t *_stm32f4_button_get(uint32_t button_id)
{
    // Return the pointer to the button with the given ID. If the ID is not valid, return NULL.
    if (button_id < sizeof(buttons_arr) / sizeof(buttons_arr[0]))
    {
        return &buttons_arr[button_id];
    }
    else
    {
        return NULL;
    }
}

/* Public functions -----------------------------------------------------------*/

void port_button_init(uint32_t button_id)
{
    // Retrieve the button struct using the private function and the button ID
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);


    /* TODO alumnos */
    
    if (p_button == NULL) {
        return; // Manejo de error si el botón no existe
    }

    stm32f4_system_gpio_config(p_button->p_port, p_button->pin, STM32F4_GPIO_MODE_IN, STM32F4_GPIO_PUPDR_NOPULL); // 2. Configurar el botón como entrada sin pull-up ni pull-down

    stm32f4_system_gpio_config_exti(p_button->p_port, p_button->pin, STM32F4_TRIGGER_BOTH_EDGE); // 3. Configurar interrupciones en flanco de subida y bajada

    stm32f4_system_gpio_exti_enable(p_button->pin, 1, 0); // 4. Habilitar la línea de interrupción con prioridad 1 y subprioridad 0

    EXTI->IMR |= BIT_POS_TO_MASK(p_button->pin);

}



//TODO
bool 	port_button_get_value (uint32_t button_id){
    
    stm32f4_button_hw_t *p_button = (stm32f4_button_hw_t *)_stm32f4_button_get(button_id); // 1. Obtener la estructura de configuración del botón
    if (p_button == NULL) {
        return false; // Si el botón no existe, retornar false
    }

    return stm32f4_system_gpio_read(p_button->p_port, p_button->pin);// 2. Obtener el valor del GPIO conectado al botón
}




//TODO
bool port_button_get_pressed(uint32_t button_id){
    
    stm32f4_button_hw_t *p_button = (stm32f4_button_hw_t *)_stm32f4_button_get(button_id);
    if (p_button == NULL) {
        return false; // Si el botón no existe, retornar false
    }
   
    return p_button->flag_pressed;  // Retornar el estado del botón
}


//TODO
void 	port_button_set_pressed (uint32_t button_id, bool pressed){
    stm32f4_button_hw_t *p_button = (stm32f4_button_hw_t *)_stm32f4_button_get(button_id);
    if (p_button == NULL) {
        return; // Si el botón no existe, se para la ejecucion
    }
    p_button->flag_pressed =pressed;
}


//TODO
bool 	port_button_get_pending_interrupt (uint32_t button_id){
    
    stm32f4_button_hw_t *p_button = (stm32f4_button_hw_t *)_stm32f4_button_get(button_id);
    if (p_button == NULL) {
        return false; // Si el botón no existe, retornar false
    }
    uint8_t pin = p_button->pin; // 2. Obtener el pin del botón

    return (EXTI->PR & BIT_POS_TO_MASK(pin)) != 0; // 3. Leer el valor del registro PR del EXTI
}


void port_button_clear_pending_interrupt(uint32_t button_id){
     
    stm32f4_button_hw_t *p_button = (stm32f4_button_hw_t *)_stm32f4_button_get(button_id);
    if (p_button == NULL) {
        return; // Si el botón no existe, salir
    }
    uint8_t pin = p_button->pin; // 2. Obtener el pin del botón

    EXTI->PR |= BIT_POS_TO_MASK(pin); // 3. Limpiar la interrupción pendiente en el registro PR del EXTI
}


//TODO
void 	port_button_disable_interrupts (uint32_t button_id){
    
    stm32f4_button_hw_t *p_button = (stm32f4_button_hw_t *)_stm32f4_button_get(button_id);
    if (p_button == NULL) {
        return; // Si el botón no existe, salir
    }

    uint8_t pin = p_button->pin; // 2. Obtener el pin del botón

    stm32f4_system_gpio_exti_disable(pin); // 3. Deshabilitar la línea EXTI asociada al botón
}



void stm32f4_button_set_new_gpio(uint32_t button_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    if (p_button == NULL) return;
    p_button->p_port = p_port;
    p_button->pin = pin;
}

