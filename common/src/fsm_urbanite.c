/**
 * @file fsm_urbanite.c
 * @brief Urbanite FSM main file.
 * @author Alex Martin-Romo Gonzalez (alex.martin-romo@alumnos.upm.es)
 * @date 23/05/2025
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
 #include <stdlib.h>
 #include <stdio.h>

 /* HW dependent includes */
 #include "port_system.h"
 #include "fsm.h"
 #include "fsm_urbanite.h"

 /* Typedefs --------------------------------------------------------------------*/

 /**
  * @brief Structure to define the Urbanite FSM.
  * 
  */
typedef struct fsm_urbanite_t{
        fsm_t f; /*!< Urbanite FSM */
        bool is_paused; /*!< Flag to indicate if the system is paused*/
        uint32_t on_off_press_time_ms; /*!<Time in ms to consider ON/OFF*/
        fsm_button_t* p_fsm_button; /*!< Pointer to the button FSM*/
        fsm_display_t* p_fsm_display_rear; /*!< Pointer to the display FSM*/
        fsm_ultrasound_t* p_fsm_ultrasound_rear; /*!< Pointer to the ultrasound FSM*/
        uint32_t pause_display_time_ms; /*!< Time in ms to pause the display*/
 } fsm_urbanite_t;

 /* State machine input or transition functions */

/**
 * @brief Check if the button has been pressed for the required time to turn ON the Urbanite system.
 * 
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 * @return true if the duration is greater than 0 and greater than the required time to turn ON the system.
 * @return false Otherwise.
 */
static bool check_on (fsm_t * p_this){

    fsm_urbanite_t* p_fsm_urbanite_t = (fsm_urbanite_t *) p_this; //hacemos otro cast
    fsm_button_t* p_fsm_button_t = p_fsm_urbanite_t->p_fsm_button;//del urbanite accedemos al boton

    uint32_t duracion = fsm_button_get_duration(p_fsm_button_t);
    return (duracion>0 && duracion>p_fsm_urbanite_t->on_off_press_time_ms);
 }

 /**
  * @brief Check if the button has been pressed for the required time to turn OFF the system.
  * 
  * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
  * @return true 
  * @return false 
  */
static bool check_off (fsm_t * p_this){
    return check_on(p_this); //el tiempo de encendido es el mismo que el de apagado
 }

/**
 * @brief Check if a new measurement is ready.
 * 
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 * @return true 
 * @return false 
 */
 static bool check_new_measure (fsm_t * p_this){
    fsm_urbanite_t* p_fsm_urbanite_t = (fsm_urbanite_t *) p_this; //hacemos el cast
    return fsm_ultrasound_get_new_measurement_ready(p_fsm_urbanite_t->p_fsm_ultrasound_rear);
 }

 /**
  * @brief Check if it has been required to pause the display.
  * 
  * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
  * @return true If the duration is greater than 0, less than the required time to turn ON the system, and greater than the required time to pause the display.
  * @return false Otherwise.
  */
 static bool check_pause_display (fsm_t * p_this){
    fsm_urbanite_t* p_fsm_urbanite_t = (fsm_urbanite_t *) p_this; //hacemos un cast
    //fsm_button_t* p_fsm_button_t = p_fsm_urbanite_t->p_fsm_button; //del urbanite accedemos al boton

    uint32_t duracion = fsm_button_get_duration(p_fsm_urbanite_t->p_fsm_button);
    return (duracion>0
    && duracion<p_fsm_urbanite_t->on_off_press_time_ms
    && duracion>p_fsm_urbanite_t->pause_display_time_ms);
    //lo divido por lineas porque si no no se ve
 }

/**
 * @brief Check if any of the elements of the system is active.
 * 
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 * @return true  if any of the elements (button, ultrasound, or display) is active.
 * @return false otherwise
 */
 static bool check_activity (fsm_t * p_this){
    fsm_urbanite_t* p_fsm_urbanite_t =(fsm_urbanite_t *) p_this; //hacemos el cast

    return(//accedemos al boton, ultras y display desde el urbanite
        fsm_button_check_activity (p_fsm_urbanite_t->p_fsm_button)
        || fsm_ultrasound_check_activity (p_fsm_urbanite_t->p_fsm_ultrasound_rear)
        || fsm_display_check_activity (p_fsm_urbanite_t->p_fsm_display_rear)
    );
 }

 /**
  * @brief Check if all the elements of the system are inactive.
  * 
  * @param p_this Pointer to an fsm_t struct than contains an fsm_urbanite_t.
  * @return true  If all the elements of the system are inactive.
  * @return false Otherwise.
  */
 static bool check_no_activity (fsm_t * p_this){
    return !check_activity(p_this);
 }

 /**
  * @brief Check if any a new measurement is ready while the system is in low power mode.
  * 
  * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
  * @return true 
  * @return false 
  */
 static bool check_activity_in_measure (fsm_t * p_this){
    return check_new_measure(p_this);
 }



 /* State machine output or action functions */


 /**
  * @brief Turn the Urbanite system ON.
  * 
  * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
  */
 static void do_start_up_measure (fsm_t * p_this){
    fsm_urbanite_t* p_fsm_urbanite_t = (fsm_urbanite_t *) p_this; //hacemos el cast

    fsm_button_reset_duration(p_fsm_urbanite_t->p_fsm_button);
    fsm_ultrasound_start (p_fsm_urbanite_t->p_fsm_ultrasound_rear);

    //true if the display system is paused, false if the display system is active
    fsm_display_set_status (p_fsm_urbanite_t->p_fsm_display_rear, true);//true pues esta inactivo y lo queremos encender

    printf("[URBANITE][%ld] Urbanite system ON\n", port_system_get_millis());//TODO
 }

/**
 * @brief Turn the Urbanite system OFF.
 * 
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 */
 static void do_stop_urbanite (fsm_t * p_this){

    fsm_urbanite_t* p_fsm_urbanite_t = (fsm_urbanite_t *) p_this;

    fsm_button_reset_duration(p_fsm_urbanite_t->p_fsm_button);
    fsm_ultrasound_stop(p_fsm_urbanite_t->p_fsm_ultrasound_rear);
    fsm_display_set_status(p_fsm_urbanite_t->p_fsm_display_rear, false);//esta encendido y queremos apagarlo
        if (p_fsm_urbanite_t->is_paused){
            p_fsm_urbanite_t->is_paused = false;
        };

        printf("[URBANITE][%ld] Urbanite system OFF\n", port_system_get_millis());
 }


 /**
  * @brief Pause or resume the display system.
  * 
  * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
  */
 static void do_pause_display (fsm_t * p_this){

    fsm_urbanite_t* p_fsm_urbanite_t = (fsm_urbanite_t *) p_this;

    fsm_button_reset_duration(p_fsm_urbanite_t->p_fsm_button);
    p_fsm_urbanite_t->is_paused = !p_fsm_urbanite_t->is_paused;
    fsm_display_set_status(p_fsm_urbanite_t->p_fsm_display_rear, p_fsm_urbanite_t->is_paused);

    printf("[URBANITE][%ld] Urbanite system display PAUSE\n", port_system_get_millis());//TODO
    printf("[URBANITE][%ld] Urbanite system display RESUME\n", port_system_get_millis());   
 }

/**
 * @brief Display the distance measured by the ultrasound sensor.
 * 
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 */
 static void do_display_distance (fsm_t * p_this){

    fsm_urbanite_t* p_fsm_urbanite_t = (fsm_urbanite_t *) p_this;//hacemos el cast

    uint32_t distancia = fsm_ultrasound_get_distance(p_fsm_urbanite_t->p_fsm_ultrasound_rear);
    //lo guardo en una variable para no estar llamando a la funcion constantemente

    if (p_fsm_urbanite_t->is_paused){
        
        if (distancia < WARNING_MIN_CM / 2){
            fsm_display_set_distance (p_fsm_urbanite_t->p_fsm_display_rear, distancia);
            fsm_display_set_status(p_fsm_urbanite_t->p_fsm_display_rear, true);
        }
        fsm_display_set_status(p_fsm_urbanite_t->p_fsm_display_rear, false);
    }
    else {
        fsm_display_set_distance (p_fsm_urbanite_t->p_fsm_display_rear, distancia);
    }

    printf("[URBANITE][%ld] Distance: %ld cm\n", port_system_get_millis(), distancia);
 }

/**
 * @brief Start the low power mode while the Urbanite is OFF.
 * 
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 */
 static void do_sleep_off (fsm_t * p_this){
    port_system_sleep();
 }


 /**
  * @brief Start the low power mode while the Urbanite is measuring the distance and it is waiting for a new measurement.
  * 
  * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
  */
 static void do_sleep_while_measure (fsm_t * p_this){
    port_system_sleep();
 }

/**
 * @brief Start the low power mode while the Urbanite is awakened by a debug breakpoint or similar in the SLEEP_WHILE_OFF state.
 * 
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 */
 static void do_sleep_while_off (fsm_t * p_this){
    port_system_sleep();
 }

 /**
  * @brief Start the low power mode while the Urbanite is awakened by a debug breakpoint or similar in the SLEEP_WHILE_ON state.
  * 
  * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
  */
 static void do_sleep_while_on (fsm_t * p_this){
    port_system_sleep();
 }


 static fsm_trans_t fsm_trans_urbanite[] = {
    {OFF, check_on, MEASURE, do_start_up_measure},
    {OFF, check_no_activity, SLEEP_WHILE_OFF, do_sleep_off},
    {SLEEP_WHILE_OFF, check_activity, OFF, NULL},
    {SLEEP_WHILE_OFF, check_no_activity, SLEEP_WHILE_OFF, do_sleep_while_off},
    {MEASURE, check_new_measure, MEASURE, do_display_distance},
    {MEASURE, check_pause_display, MEASURE, do_pause_display},
    {MEASURE, check_off, OFF, do_stop_urbanite},
    {MEASURE, check_no_activity, SLEEP_WHILE_ON, do_sleep_while_measure},
    {SLEEP_WHILE_ON, check_activity_in_measure, MEASURE, NULL},
    {SLEEP_WHILE_ON, check_no_activity, SLEEP_WHILE_ON, do_sleep_while_on},

    {-1, NULL, -1, NULL}
 }; /*!< Array representing the transitions table of the FSM Urbanite.*/


 /**
  * @brief Create a new Urbanite FSM.
  * This function initializes the default values of the FSM struct and calls to the port to initialize the HWs associated to the devices.
  * 
  * @param p_fsm_urbanite Pointer to the Urbanite FSM.
  * @param p_fsm_button Pointer to the button FSM that activates the system and disables the display if it disturbs the driver.
  * @param on_off_press_time_ms Button press time in milliseconds to turn the system ON or OFF
  * @param pause_display_time_ms Time in milliseconds to pause the display after a short press of the button
  * @param p_fsm_ultrasound_rear Pointer to the ultrasound FSM that measures the distance to the rear obstacle.
  * @param p_fsm_display_rear Pointer to the display FSM that shows the distance to the rear obstacle.
  */
 static void fsm_urbanite_init (fsm_urbanite_t * p_fsm_urbanite, fsm_button_t * p_fsm_button, uint32_t on_off_press_time_ms, uint32_t pause_display_time_ms, fsm_ultrasound_t * p_fsm_ultrasound_rear, fsm_display_t * p_fsm_display_rear){
    
    fsm_init(&p_fsm_urbanite->f, fsm_trans_urbanite);
    
    p_fsm_urbanite->p_fsm_button             = p_fsm_button;
    p_fsm_urbanite->on_off_press_time_ms     = on_off_press_time_ms;
    p_fsm_urbanite->pause_display_time_ms    = pause_display_time_ms;
    p_fsm_urbanite->p_fsm_ultrasound_rear    = p_fsm_ultrasound_rear;
    p_fsm_urbanite->p_fsm_display_rear       = p_fsm_display_rear;

    p_fsm_urbanite->is_paused                = false; 
}

/**
 * @brief Create a new Urbanite FSM.

This function creates a new Urbanite FSM with the given button, ultrasound, display FSMs and the required times for configuration.
 * 
 * @param p_fsm_button Pointer to the button FSM to interact with the Urbanite.
 * @param on_off_press_time_ms Time in ms to consider ON/OFF of the Urbanite parking aid system.
 * @param pause_display_time_ms Time in ms to pause the display system.
 * @param p_fsm_ultrasound_rear Pointer to the rear ultrasound FSM.
 * @param p_fsm_display_rear Pointer to the rear display FSM.
 * @return fsm_urbanite_t* Pointer to the Urbanite FSM.
 */
fsm_urbanite_t* fsm_urbanite_new (fsm_button_t * p_fsm_button, uint32_t on_off_press_time_ms, uint32_t pause_display_time_ms, fsm_ultrasound_t * p_fsm_ultrasound_rear, fsm_display_t * p_fsm_display_rear){   

    fsm_urbanite_t *p_fsm_urbanite =malloc(sizeof(fsm_urbanite_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_urbanite_init(p_fsm_urbanite, p_fsm_button,on_off_press_time_ms,pause_display_time_ms,p_fsm_ultrasound_rear, p_fsm_display_rear);
    return p_fsm_urbanite;
}

/**
 * @brief Fire the Urbanite FSM.

This function is used to check the transitions and execute the actions of the Urbanite FSM.
 * 
 * @param p_fsm Pointer to the fsm_urbanite_t struct.
 */
void fsm_urbanite_fire (fsm_urbanite_t * p_fsm){
    fsm_fire(&p_fsm->f);
}

/**
 * @brief Destroy an Urbanite FSM.

This function destroys an Urbanite FSM and frees the memory.
 * 
 * @param p_fsm Pointer to an fsm_urbanite_t struct.
 */
void fsm_urbanite_destroy (fsm_urbanite_t * p_fsm){
    free(&p_fsm->f);
}