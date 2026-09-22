/**
 * @file main.c
 * @brief 
 * 
 * 
 * 
 * @author Alex Martin-Romo (alex.martin-romo@alumnos.upm.es)
 * @date 23/05/2025
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C libraries */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h> // printf

/* HW libraries */

#include "port_system.h"
#include "port_button.h"
#include "port_ultrasound.h"
#include "port_display.h"
#include "fsm.h"
#include "fsm_button.h"
#include "fsm_ultrasound.h"
#include "fsm_display.h"
#include "fsm_urbanite.h"

/* Defines ------------------------------------------------------------------*/

#define 	URBANITE_ON_OFF_PRESS_TIME_MS 1000 /*!<Time in ms to activate the Urbanite system, started mainly due to a parking maneuver (long press)*/
#define 	URBANITE_PAUSE_DISPLAY_TIME_MS 500 /*!<Time in ms to pause the display*/


/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* Init board */
    port_system_init();

    fsm_button_t* p_fsm_button = fsm_button_new(PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS, PORT_PARKING_BUTTON_ID);
    fsm_ultrasound_t * p_fsm_ultrasound = fsm_ultrasound_new(PORT_REAR_PARKING_SENSOR_ID);
    fsm_display_t * p_fsm_display = fsm_display_new(PORT_REAR_PARKING_DISPLAY_ID);
    fsm_urbanite_t * p_fsm_urbanite = fsm_urbanite_new(p_fsm_button, URBANITE_ON_OFF_PRESS_TIME_MS, URBANITE_PAUSE_DISPLAY_TIME_MS, p_fsm_ultrasound, p_fsm_display);  

   
    /* Infinite loop */
    while (1)
    {
        fsm_button_fire(p_fsm_button);
        fsm_ultrasound_fire(p_fsm_ultrasound);
        fsm_display_fire(p_fsm_display);
        fsm_urbanite_fire(p_fsm_urbanite);
        
    } // End of while(1)

    //para liberar la memoria qe se guarda de cada una con el malloc en el fsm_xxx_new usamos el destroy
    // el malloc guarda memoria dinamica
    //pero esto nunca se ejecutara porque nunca se saldra del bucle while
    fsm_button_destroy(p_fsm_button);
    fsm_ultrasound_destroy(p_fsm_ultrasound);
    fsm_display_destroy(p_fsm_display);
    fsm_urbanite_destroy(p_fsm_urbanite);
    



    return 0;
}
