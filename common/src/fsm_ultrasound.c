/**
 * @file fsm_ultrasound.c
 * @brief Ultrasound sensor FSM main file.
 * @author Alex Martin-Romo Gonzalez (alex.martin-romo@alumnos.upm.es)
 * @author Salvador García Oviedo   (salvador.garcia@alumnos.upm.es)
 * @date 28/03/2025
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "port_ultrasound.h"
#include "port_system.h"
#include "fsm.h"
#include "fsm_ultrasound.h"

/* Standard C includes */

/* HW dependent includes */

/* Project includes */

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Structure to define the Ultrasound FSM.
 *
 */
struct fsm_ultrasound_t
{
    fsm_t f;                                                /*!<Ultrasound FSM*/
    uint32_t distance_cm;                                   /*!<How much time the ultrasound has been pressed*/
    bool status;                                            /*!<Indicate if the ultrasound sensor is active or not*/
    bool new_measurement;                                   /*!<Flag to indicate if a new measurement has been completed*/
    uint32_t ultrasound_id;                                 /*!<Ultrasound ID. Must be unique.*/
    uint32_t distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS]; /*!<Array to store the last distance measurements*/
    uint32_t distance_idx;                                  /*!<Index to store the last distance measurement*/
};

/* Private functions -----------------------------------------------------------*/

/**
 * @brief Compare function to sort arrays.

This function is used to compare two elements. It will be used for the qsort() function to sort the array of distances.
 *
 * @param a Pointer to the first element to compare.
 * @param b Pointer to the second element to compare.
 * @return int Result of the comparison.
 */
int _compare(const void *a, const void *b)
{
    return (*(uint32_t *)a - *(uint32_t *)b);
}

/* State machine input or transition functions */

/**
 * @brief Check if the ultrasound sensor is active and ready to start a new measurement.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 * @return true
 * @return false
 */
static bool check_on(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;                                               // primero se hace el cast
    return fsm_ultrasound_get_status(p_fsm) && port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id); // luego se comrpueba la flag
}

/**
 * @brief Check if the ultrasound sensor has been set to be inactive (OFF).
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 * @return true
 * @return false
 */
static bool check_off(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this; // primero se hace el cast
    return !p_fsm->status;
}
/**
 * @brief Check if the ultrasound sensor has finished the trigger signal.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 * @return true if the time to trigger the ultrasound sensor has finished.
 * @return otherwise
 */
static bool check_trigger_end(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;         // primero se hace el cast
    return port_ultrasound_get_trigger_end(p_fsm->ultrasound_id); // luego se comrpueba la flag
}

/**
 * @brief Check if the ultrasound sensor has received the init (rising edge in the input capture) of the echo signal.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 * @return true
 * @return false
 */
static bool check_echo_init(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;                               // primero se hace el cast
    uint32_t echo_init_tick = port_ultrasound_get_echo_init_tick(p_fsm->ultrasound_id); // se obtiene el tiempo del tick
    return (echo_init_tick > 0);                                                        // true si el tick es mayor que 0
}

/**
 * @brief Check if the ultrasound sensor has received the end (falling edge in the input capture) of the echo signal.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 * @return true if both the init and end ticks have been received.
 * @return false otherwise
 */
static bool check_echo_received(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;           // primero se hace el cast
    return port_ultrasound_get_echo_received(p_fsm->ultrasound_id); // se obtiene el tiempo del tick
}

/**
 * @brief Check if a new measurement is ready.
 *
 * @param p_this
 * @return true if the ultrasound sensor is ready to start a new measurement.
 * @return false otherwise
 */
static bool check_new_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;           // primero se hace el cast
    return port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id); // true if the ultrasound sensor is ready to start a new measurement.
}

/**
 * @brief Start a measurement of the ultrasound transceiver for the first time after the FSM is started.
 *
 * @param p_this Pointer to an fsm_t struct than contains an fsm_ultrasound_t.
 */
static void do_start_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;    // primero se hace el cast
    port_ultrasound_start_measurement(p_fsm->ultrasound_id); // se inicia la medición con el ID correcto
}

/**
 * @brief Stop the trigger signal of the ultrasound sensor.

This function is called when the time to trigger the ultrasound sensor has finished. It stops the trigger signal and the trigger timer.
 *
 * @param p_this Pointer to an fsm_t struct than contains an fsm_ultrasound_t.
 */
static void do_stop_trigger(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;     // primero se hace el cast
    port_ultrasound_stop_trigger_timer(p_fsm->ultrasound_id); // se para el trigger timer
    port_ultrasound_set_trigger_end(p_fsm->ultrasound_id, false);
}

/**
 * @brief Set the distance measured by the ultrasound sensor.

This function is called when the ultrasound sensor has received the echo signal. It calculates the distance in cm and stores it in the array of distances.

When the array is full, it computes the median of the array and resets the index of the array.
 *
 * @param p_this
 */
static void do_set_distance(fsm_t *p_this)
{ // TODO AQUI VIENE LA GORDA
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;

    // 1. Retrieve echo init tick, echo end tick, and echo overflows
    uint32_t echo_init_tick = port_ultrasound_get_echo_init_tick(p_fsm->ultrasound_id);
    uint32_t echo_end_tick = port_ultrasound_get_echo_end_tick(p_fsm->ultrasound_id);
    uint32_t echo_overflows = port_ultrasound_get_echo_overflows(p_fsm->ultrasound_id);

    // 2. Calculate the total time (ticks)
    uint32_t time_us; // tiempo en microsegundos
    if (echo_end_tick >= echo_init_tick)
    {
        time_us = (echo_end_tick - echo_init_tick) + (echo_overflows * 0xFFFFFFFF);
    }
    else
    {
        time_us = ((0xFFFFFFFF - echo_init_tick) + echo_end_tick + 1) + (echo_overflows * 0xFFFFFFFF);
    }

    // 3. Calculate the distance in cm

    uint32_t distance_cm = (time_us * 343) / (2 * 10000);

    // 4. Store the distance in the array
    p_fsm->distance_arr[p_fsm->distance_idx] = distance_cm;

    // 5. If the array is full, sort it
    if (p_fsm->distance_idx == FSM_ULTRASOUND_NUM_MEASUREMENTS - 1)
    {
        qsort(p_fsm->distance_arr, FSM_ULTRASOUND_NUM_MEASUREMENTS, sizeof(uint32_t), _compare);

        // 6. Compute the median
        if (FSM_ULTRASOUND_NUM_MEASUREMENTS % 2 == 0)
        {
            p_fsm->distance_cm = (p_fsm->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2 - 1] +
                                  p_fsm->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2]) /
                                 2;
        }
        else
        {
            p_fsm->distance_cm = p_fsm->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2];
        }

        // 7. Set the new_measurement flag
        p_fsm->new_measurement = true;
    }

    // 8. Increase the index and reset if necessary
    p_fsm->distance_idx = (p_fsm->distance_idx + 1) % FSM_ULTRASOUND_NUM_MEASUREMENTS;

    // 9. Stop the echo timer
    port_ultrasound_stop_echo_timer(p_fsm->ultrasound_id);

    // 10. Reset the echo ticks
    port_ultrasound_reset_echo_ticks(p_fsm->ultrasound_id);
}

/**
 * @brief Stop the ultrasound sensor.

This function is called when the ultrasound sensor is stopped. It stops the ultrasound sensor and resets the echo ticks.
 *
 * @param p_this Pointer to an fsm_t struct than contains an fsm_ultrasound_t.
 */
static void do_stop_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)p_this;
    port_ultrasound_stop_ultrasound(p_fsm->ultrasound_id); // se para el ultrasound
}

/**
 * @brief Start a new measurement of the ultrasound transceiver.

This function is called when the ultrasound sensor has finished a measurement and is ready to start a new one.
 *
 * @param p_this Pointer to an fsm_t struct than contains an fsm_ultrasound_t.
 */
static void do_start_new_measurement(fsm_t *p_this)
{
    do_start_measurement(p_this); // llama a la funcion de start measurement
}

/* State machine output or action functions */

/**
 * @brief Array representing the transitions table of the FSM ultrasound.
 *
 */
static fsm_trans_t fsm_trans_ultrasound[] = {
    {WAIT_START, check_on, TRIGGER_START, do_start_measurement},
    {TRIGGER_START, check_trigger_end, WAIT_ECHO_START, do_stop_trigger},
    {WAIT_ECHO_START, check_echo_init, WAIT_ECHO_END, NULL},
    {WAIT_ECHO_END, check_echo_received, SET_DISTANCE, do_set_distance},
    {SET_DISTANCE, check_new_measurement, TRIGGER_START, do_start_new_measurement},
    {SET_DISTANCE, check_off, WAIT_START, do_stop_measurement},
    {-1, NULL, -1, NULL},
};

void fsm_ultrasound_fire(fsm_ultrasound_t *p_fsm)
{

    fsm_fire(&p_fsm->f); // llama a la funcion fire de la FSM
}

void fsm_ultrasound_destroy(fsm_ultrasound_t *p_fsm)
{
    free(p_fsm); // libera la memoria
}

fsm_t *fsm_ultrasound_get_inner_fsm(fsm_ultrasound_t *p_fsm)
{
    return &p_fsm->f; // devuelve la FSM
}

uint32_t fsm_ultrasound_get_state(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->f.current_state; // devuelve el estado actual de la FSM
}

uint32_t fsm_ultrasound_get_distance(fsm_ultrasound_t *p_fsm)
{
    p_fsm->new_measurement = false; // se pone la flag de new measurement a false
    return p_fsm->distance_cm;      // se guarda la distancia
}

void fsm_ultrasound_stop(fsm_ultrasound_t *p_fsm)
{
    // p_fsm->status = false; //se pone la flag de status a false
    fsm_ultrasound_set_status(p_fsm, false);
    port_ultrasound_stop_ultrasound(p_fsm->ultrasound_id); // se para el ultrasound
}

void fsm_ultrasound_start(fsm_ultrasound_t *p_fsm)
{
    // p_fsm->status = true; //se pone la flag de status a true
    // cambiarlo a fsm_ultrasound_set_status (p_fsm, true);
    fsm_ultrasound_set_status(p_fsm, true);                        // se pone la flag de status a true
    p_fsm->distance_idx = 0;                                       // se pone el index a 0
    p_fsm->distance_cm = 0;                                        // se pone la distancia a 0
    port_ultrasound_reset_echo_ticks(p_fsm->ultrasound_id);        // se reinician los ticks
    port_ultrasound_set_trigger_ready(p_fsm->ultrasound_id, true); // se pone la flag de trigger ready a true
    port_ultrasound_start_new_measurement_timer();                 // se inicia el timer
}

bool fsm_ultrasound_get_status(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->status; // devuelve el estado de la FSM
}

void fsm_ultrasound_set_status(fsm_ultrasound_t *p_fsm, bool status)
{
    p_fsm->status = status; // se pone la flag de status a true
}

bool fsm_ultrasound_get_ready(fsm_ultrasound_t *p_fsm)
{
    return port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id); // devuelve el estado de la FSM
}

bool fsm_ultrasound_get_new_measurement_ready(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->new_measurement; // devuelve el estado de la FSM
}

/* Other auxiliary functions */

/**
 * @brief Initialize a ultrasound FSM.

This function initializes the default values of the FSM struct and calls to the port to initialize the associated HW given the ID.

The FSM stores the distance of the last ultrasound trigger. The user should ask for it using the function fsm_ultrasound_get_distance().

The FSM contains information of the ultrasound ID. This ID is a unique identifier that is managed by the user in the port. That is where the user provides identifiers and HW information for all the ultrasounds on his system. The FSM does not have to know anything of the underlying HW.
 *
 * @param p_fsm_ultrasound Pointer to the ultrasound FSM.
 * @param ultrasound_id Unique ultrasound identifier number.
 */
void fsm_ultrasound_init(fsm_ultrasound_t *p_fsm_ultrasound, uint32_t ultrasound_id)
{
    // Initialize the FSM
    fsm_init(&p_fsm_ultrasound->f, fsm_trans_ultrasound);

    /* TODO alumnos: */
    // Initialize the fields of the FSM structure

    p_fsm_ultrasound->distance_cm = 0;
    p_fsm_ultrasound->distance_idx = 0;
    p_fsm_ultrasound->ultrasound_id = ultrasound_id;
    memset(p_fsm_ultrasound->distance_arr, 0, sizeof(p_fsm_ultrasound->distance_arr)); // Initialize the distance array to 0

    // p_fsm_ultrasound->status = false;
    fsm_ultrasound_set_status(p_fsm_ultrasound, false);
    p_fsm_ultrasound->new_measurement = false;
    port_ultrasound_init(ultrasound_id); // Initialize the port with the ultrasound ID
}

/* Public functions -----------------------------------------------------------*/

fsm_ultrasound_t *fsm_ultrasound_new(uint32_t ultrasound_id)
{
    fsm_ultrasound_t *p_fsm_ultrasound = malloc(sizeof(fsm_ultrasound_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_ultrasound_init(p_fsm_ultrasound, ultrasound_id);                  /* Initialize the FSM */
    return p_fsm_ultrasound;
}

// Other auxiliary functions

void fsm_ultrasound_set_state(fsm_ultrasound_t *p_fsm, int8_t state)
{
    p_fsm->f.current_state = state;
}

bool fsm_ultrasound_check_activity(fsm_ultrasound_t *p_fsm)
{
    return false; // Always false because all the transitions are due to HW interrupts.
}