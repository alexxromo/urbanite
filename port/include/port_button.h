/**
 * @file port_button.h
 * @brief Header for the portable functions to interact with the HW of the buttons. The functions must be implemented in the platform-specific code.
 * @author Alex Martin-Romo Gonzalez (alex.martin-romo@alumnos.upm.es)
 * @author Salvador García Oviedo   (salvador.garcia@alumnos.upm.es)
 * @date 28/03/2025
 */

#ifndef PORT_BUTTON_H_
#define PORT_BUTTON_H_

#define PORT_PARKING_BUTTON_ID 0 /*!<Button ID. In this case is 0 as its our only button*/
#define PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS 200/*!<Button debounce time in milliseconds*/

//#define PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS  50
//#define PORT_PARKING_BUTTON_ID                1


/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
// Define here all the button identifiers that are used in the system

/* Function prototypes and explanation -------------------------------------------------*/


/**
 * @brief Configure the HW specifications of a given button.
 * 
 * @param button_id This index is used to select the element of the buttons_arr[] array
 */
void 	port_button_init (uint32_t button_id);


/**
 * @brief 	Get the value of the GPIO connected to the button.
 * 
 * @param button_id This index is used to select the element of the buttons_arr[] array
 * @return true 
 * @return false 
 */
bool 	port_button_get_value (uint32_t button_id);



/**
 * @brief 	Return the status of the button (pressed or not).
 * 
 * @param button_id This index is used to get the correct button status struct.
 * @return true If the button has been pressed
 * @return false If the button has not been pressed
 */
bool    port_button_get_pressed (uint32_t button_id);



/**
 * @brief Set the status of the button (pressed or not).
 * 
 * @param button_id This index is used to get the correct button struct of the buttons_arr[] array.
 * @param pressed Status of the button.
 */

void 	port_button_set_pressed (uint32_t button_id, bool pressed);


/**
 * @brief Get the status of the interrupt line connected to the button.
 * 
 * @param button_id This index is used to get the correct button struct of the buttons_arr[] array.
 * @return true 
 * @return false 
 */
bool 	port_button_get_pending_interrupt (uint32_t button_id);


/**
 * @brief Clear the pending interrupt of the button.
 * 
 * @param button_id This index is used to get the correct button struct of the buttons_arr[] array.
 */
void 	port_button_clear_pending_interrupt (uint32_t button_id);


/**
 * @brief 	Disable the interrupts of the button.
 * 
 * @param button_id This index is used to get the correct button struct of the buttons_arr[] array.
 */
void 	port_button_disable_interrupts (uint32_t button_id);



#endif