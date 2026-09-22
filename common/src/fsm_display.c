/**
 * @file fsm_display.c
 * @brief Display system FSM main file.
 * @author Alex Martin-Romo Gonzalez (alex.martin-romo@alumnos.upm.es)
 * @date 23/05/2025
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>

/* HW dependent includes */
#include "port_display.h"
#include "port_system.h"
#include "fsm.h"
#include "fsm_display.h"

/* Project includes */

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Structure of the Display FSM.
 * 
 */
typedef struct fsm_display_t
{
    fsm_t       f; /*!<Display system FSM */
    uint32_t    distance_cm; /*!<Distance in cm to the object*/
    bool        new_color; /*!<Flag to indicate if a new color has to be set*/
    bool        status; /*!<Flag to indicate if the display is active*/
    bool        idle; /*!<Flag to indicate if the display being active is idle, or not*/
    uint32_t    display_id; /*!<Unique display system identifier number*/

} fsm_display_t;


/* Private functions -----------------------------------------------------------*/

/**
 * @brief Set color levels of the RGB LEDs according to the distance.

This function sets the levels of an RGB LED according to the distance measured by the ultrasound sensor. This RGB LED structure is later passed to the port_display_set_rgb() function to set the color of the RGB LED.
 * 
 * @param p_color Pointer to an rgb_color_t struct that will store the levels of the RGB LED.
 * @param distance_cm Distance measured by the ultrasound sensor in centimeters.
 */
void _compute_display_levels(rgb_color_t * p_color, int32_t distance_cm){
    if (!p_color) return;

    if (distance_cm >= DANGER_MIN_CM && distance_cm <= WARNING_MIN_CM) {

        p_color->r = 255; p_color->g = 0;   p_color->b = 0;
        return;
    }
    else if (distance_cm > WARNING_MIN_CM && distance_cm <= NO_PROBLEM_MIN_CM) {

        p_color->r = 237; p_color->g = 150; p_color->b = 0;
        return;
    }
    else if (distance_cm > NO_PROBLEM_MIN_CM && distance_cm <= INFO_MIN_CM) {

        p_color->r = 0; p_color->g = 255; p_color->b = 0;
        return;
    }
    else if (distance_cm > INFO_MIN_CM && distance_cm <= OK_MIN_CM) {

        p_color->r = 26;   p_color->g = 89;   p_color->b = 82;
        return;
    }
    else if (distance_cm > OK_MIN_CM && distance_cm <= OK_MAX_CM) {

        p_color->r = 0;   p_color->g = 0; p_color->b = 255;
        return;
    }
    else if (distance_cm >OK_MAX_CM){
        // Fuera de rango: apagado
        p_color->r = 0;   p_color->g = 0;   p_color->b = 0;
        return;
    }
}




/* State machine input or transition functions */

/**
 * @brief Check if the display is set to be active (ON), independently if it is idle or not.
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_display_t.
 * @return true If the display system has been indicated to be active independently if it is idle or not.
 * @return false If the display system has been indicated to be inactive.
 */
static bool check_active (fsm_t * p_this){
    fsm_display_t * p_fsm_display = (fsm_display_t *) p_this;//primero el cast
    return p_fsm_display->status;//devolvemos el flag
}

/**
 * @brief Check if a new color has to be set.
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_display_t.
 * @return true If a new color has to be set
 * @return false If a new color does not have to be set
 */
static bool check_set_new_color	(fsm_t * p_this){
    fsm_display_t * p_fsm_display = (fsm_display_t *) p_this;//primero el cast
    return p_fsm_display->new_color;//devolvemos el flag
}

/**
 * @brief Check if the display is set to be inactive (OFF).
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_display_t.
 * @return true If the display system has been indicated to be inactive.
 * @return false  If the display system has been indicated to be active.
 */
static bool check_off (fsm_t * p_this){
    fsm_display_t * p_fsm_display = (fsm_display_t *) p_this;//primero el cast
    return !p_fsm_display->status;//devolvemos el flag
}

/**
 * @brief Turn the display system ON for the first time.
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_display_t.
 */
static void do_set_on (fsm_t * p_this){
    fsm_display_t * p_fsm_display = (fsm_display_t *) p_this;//primero el cast
    port_display_set_rgb(p_fsm_display->display_id, COLOR_OFF); // Set the RGB LED to off
}

/**
 * @brief Set the color of the RGB LED according to the distance measured by the ultrasound sensor.
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_display_t.
 */
static void do_set_color (fsm_t * p_this){
    fsm_display_t * p_fsm_display = (fsm_display_t *) p_this;
    //Compute the levels of the RGB LEDs according to the distance and set the display level
    rgb_color_t color;//declaramos un color nuevo para pasarlo despues
    rgb_color_t *p_color = &color;
    _compute_display_levels(p_color, p_fsm_display->distance_cm);
    port_display_set_rgb(p_fsm_display->display_id, *p_color);
    p_fsm_display->new_color = false;
    p_fsm_display->idle = true;
}

/**
 * @brief Turn the display system OFF.
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_display_t.
 */
static void do_set_off (fsm_t * p_this){
    fsm_display_t * p_fsm_display = (fsm_display_t *) p_this;
    port_display_set_rgb(p_fsm_display->display_id, COLOR_OFF);
    p_fsm_display->idle=false;
}


/* State machine output or action functions */

static fsm_trans_t fsm_trans_display[] = {
    {WAIT_DISPLAY, check_active, SET_DISPLAY, do_set_on},
    {SET_DISPLAY, check_set_new_color, SET_DISPLAY, do_set_color},
    {SET_DISPLAY, check_off, WAIT_DISPLAY, do_set_off},
    {-1, NULL, -1, NULL}
}; /*!<Array representing the transitions table of the FSM display.*/


/**
 * @brief Initialize a display system FSM.

This function initializes the default values of the FSM struct and calls to the port to initialize the associated HW given the ID.

The FSM stores the display level of the display system. The user should set it using the function fsm_display_set_distance().
 * 
 * @param p_fsm_display Pointer to the display FSM.
 * @param display_id Unique display identifier number.
 */
static void fsm_display_init (fsm_display_t * p_fsm_display, uint32_t display_id ){
    
    /* 1) Inicializar la FSM indicando la tabla de transiciones */
    fsm_init(&p_fsm_display->f, fsm_trans_display);

    /* 2) Guardar el identificador del display */
    p_fsm_display->display_id = display_id;

    /* 3) Inicializar distance_cm a un valor inválido */
    p_fsm_display->distance_cm = (uint32_t)-1;

    //fsm_display_set_distance(p_fsm_display, -1); // Set the distance to -1cm (invalid)

    /* 4) Inicializar las banderas new_color, status e idle a false */
    p_fsm_display->new_color = false;
    p_fsm_display->status    = false;
    p_fsm_display->idle      = false;

    /* 5) Llamar al puerto para inicializar el HW del display */
    port_display_init(display_id);

    

    }

/* Other auxiliary functions */

/* Public functions -----------------------------------------------------------*/
fsm_display_t *fsm_display_new(uint32_t display_id)
{
    fsm_display_t *p_fsm_display = malloc(sizeof(fsm_display_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_display_init(p_fsm_display, display_id); /* Initialize the FSM */
    return p_fsm_display;
}


void fsm_display_fire (fsm_display_t * p_fsm){

    fsm_fire(&p_fsm->f); /* Call the fire function of the FSM */ 
}


void fsm_display_destroy (fsm_display_t * p_fsm){
    if (p_fsm != NULL) {
        free(&p_fsm->f); /* Free the memory of the FSM */
    }
}

fsm_t* fsm_display_get_inner_fsm (fsm_display_t * p_fsm){
    if (p_fsm != NULL) {
        return &p_fsm->f; /* Return the inner FSM */
    }
    return NULL; /* Return NULL if the FSM is NULL */
}

uint32_t fsm_display_get_state (fsm_display_t * p_fsm){
    if (p_fsm != NULL) {
        //return fsm_get_state(&p_fsm->f); /* Return the state of the FSM */
        return p_fsm->f.current_state; /* Return the state of the FSM */
    }
    return 0; /* Return 0 if the FSM is NULL */
}


uint32_t fsm_display_get_distance(fsm_display_t * p_fsm){//TODO esta no aparece en la api
    return p_fsm->distance_cm; /* Return the distance of the FSM */
}

void fsm_display_set_distance (fsm_display_t * p_fsm, uint32_t distance_cm ){
    p_fsm->distance_cm = distance_cm; /* Set the distance of the FSM */
    p_fsm->new_color = true; /* Set the new color flag to true */
}


bool fsm_display_get_status (fsm_display_t *p_fsm){
    return p_fsm->status; /* Return the status of the FSM */
}

void fsm_display_set_status (fsm_display_t *p_fsm, bool pause){
    p_fsm->status = pause;
}

void fsm_display_set_state(fsm_display_t *p_fsm_display, int8_t state) {//TODO esta es un pocok de v3
    return fsm_set_state(&p_fsm_display->f, state);
}

bool fsm_display_check_activity(fsm_display_t *p_fsm_display) {
    return (p_fsm_display->status && !p_fsm_display->idle);
}