/**
 * @file stm32f4_ultrasound.c
 * @brief Portable functions to interact with the ultrasound FSM library. All portable functions must be implemented in this file.
 * @author Alex Martin-Romo Gonzalez (alex.martin-romo@alumnos.upm.es)
 * @date 28/03/2025
 */

/* Standard C includes */
#include <stdio.h>
#include <math.h>
/* HW dependent includes */
#include "port_ultrasound.h"
#include "port_system.h"
/* Microcontroller dependent includes */
#include "stm32f4_system.h"
#include "stm32f4_ultrasound.h"
/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Structure to define the HW dependencies of an ultrasound sensor.
 * 
 */
typedef struct
{
    GPIO_TypeDef * 	p_trigger_port; /*!<GPIO where the trigger signal is connected*/
    GPIO_TypeDef * 	p_echo_port; /*!<GPIO where the echo signal is connected*/
    uint8_t trigger_pin; /*!<Pin/line where the trigger signal is connected*/
    uint8_t echo_pin; /*!<Pin/line where the echo signal is connected*/
    uint8_t echo_alt_fun; /*!<Alternate function for the echo signal*/
    bool 	trigger_ready; /*!<Flag to indicate that a new measurement can be started*/
    bool 	trigger_end; /*!<Flag to indicate that the trigger signal has been sent*/
    bool 	echo_received; /*!<Flag to indicate that the echo signal has been received*/
    uint32_t 	echo_init_tick; /*!<Tick time when the echo signal was received*/
    uint32_t 	echo_end_tick; /*!<Tick time when the echo signal was received*/
    uint32_t 	echo_overflows; /*!<Number of overflows of the timer during the echo signal */
}stm32f4_ultrasound_hw_t ;

/* Global variables */
/**
 * @brief Array of elements that represents the HW characteristics of the ultrasounds connected to the STM32F4 platform.

This must be hidden from the user, so it is declared as static. To access the elements of this array, use the function _stm32f4_ultrasound_get()
 * 
 */
static stm32f4_ultrasound_hw_t 	ultrasounds_arr [] ={
    [PORT_REAR_PARKING_SENSOR_ID]={
        .p_trigger_port= STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO,
        .p_echo_port=STM32F4_REAR_PARKING_SENSOR_ECHO_GPIO,
        .trigger_pin= STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN,
        .echo_pin = STM32F4_REAR_PARKING_SENSOR_ECHO_PIN,
        .echo_alt_fun = STM32F4_AF1,
        
    },  
};

/* Private functions ----------------------------------------------------------*/
//TODO
/**
 * @brief Get the ultrasound struct with the given ID.
 * 
 * @param ultrasound_id Ultrasound ID.
 * @return stm32f4_ultrasound_hw_t* Pointer to the ultrasound struct.
NULL If the ultrasound ID is not valid.
 */
stm32f4_ultrasound_hw_t* _stm32f4_ultrasound_get(uint32_t ultrasound_id){

    if (ultrasound_id >= (sizeof(ultrasounds_arr) / sizeof(ultrasounds_arr[0]))) {
        return NULL;  // Retorna NULL si el ID es inválido
    }
    return &ultrasounds_arr[ultrasound_id];  //Devuelve el puntero a la estructura correspondiente
}
/* Public functions -----------------------------------------------------------*/


// Getters and setters functions

bool port_ultrasound_get_trigger_ready(uint32_t ultrasound_id){//TODO
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL) {
        return false; // Handle invalid ultrasound_id gracefully
    }
    return p_ultrasound->trigger_ready;
}

bool port_ultrasound_get_trigger_end (uint32_t ultrasound_id){//TODO
    /* Get the ultrasound sensor */
  stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);

  /* Return the trigger_end field, default to false if invalid ID */
  if (p_ultrasound != NULL) {
      return p_ultrasound->trigger_end;
  } else {
      return false;
  }
}

uint32_t port_ultrasound_get_echo_overflows	(uint32_t ultrasound_id){//TODO
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return (p_ultrasound != NULL) ? p_ultrasound->echo_overflows : 0;
}

uint32_t port_ultrasound_get_echo_init_tick	(uint32_t ultrasound_id){//TODO
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return (p_ultrasound != NULL) ? p_ultrasound->echo_init_tick : 0;
}

uint32_t port_ultrasound_get_echo_end_tick(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return (p_ultrasound != NULL) ? p_ultrasound->echo_end_tick : 0;
}

bool port_ultrasound_get_echo_received(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return (p_ultrasound != NULL) ? p_ultrasound->echo_received : false;
}

void stm32f4_ultrasound_set_new_trigger_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL) {
        if (p_ultrasound != NULL) {
            p_ultrasound->p_trigger_port = p_port;
            p_ultrasound->trigger_pin = pin;
        }
    }
        p_ultrasound->trigger_pin = pin;
}


void stm32f4_ultrasound_set_new_echo_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL) {
        p_ultrasound->p_echo_port = p_port;
        p_ultrasound->echo_pin = pin;
    }
}

void port_ultrasound_set_trigger_ready (uint32_t ultrasound_id, bool trigger_ready){//TODO
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL) {
        p_ultrasound->trigger_ready = trigger_ready;
    }
}

void port_ultrasound_set_trigger_end (uint32_t ultrasound_id, bool trigger_end){//TODO

    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL) {
        p_ultrasound->trigger_end = trigger_end;
    }
}

void port_ultrasound_set_echo_init_tick(uint32_t ultrasound_id, uint32_t echo_init_tick){//TODO
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL){
        p_ultrasound->echo_init_tick = echo_init_tick;
    }
}

void port_ultrasound_set_echo_end_tick(uint32_t ultrasound_id, uint32_t echo_end_tick)
{ // TODO
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL){
        p_ultrasound->echo_end_tick = echo_end_tick;
    }
}

void port_ultrasound_set_echo_received(uint32_t ultrasound_id, bool echo_received){//TODO
    if (ultrasound_id >= (sizeof(ultrasounds_arr) / sizeof(ultrasounds_arr[0]))) {
        return; // Invalid ultrasound_id, exit the function
    }
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL) {
        p_ultrasound->echo_received = echo_received;
    }
}

void port_ultrasound_set_echo_overflows(uint32_t ultrasound_id, uint32_t echo_overflows)
{
    //TODO
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL) {
        p_ultrasound->echo_overflows = echo_overflows;
    }
}

// Util

void port_ultrasound_stop_trigger_timer (uint32_t ultrasound_id){//TODO
        //Set the trigger pin to low. You can use the BSRR or call the stm32f4_system_gpio_write() function.
        stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
        if (p_ultrasound == NULL) {
            return; // Exit if the ultrasound ID is invalid
        }
        stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, false);
        //Disable the trigger timer (register CR1 of the timer).
        TIM3->CR1 &= ~TIM_CR1_CEN;
        //poner el flag de trigger end a true??
}       


/**
 * @brief Configure the timer that controls the duration of the trigger signal.
 * 
 */
static void _timer_trigger_setup(){ //TODO
    /* Enalbe clock */
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* Disable counter CR1*/
    TIM3->CR1 &= ~TIM_CR1_CEN;

    /* Enable autoreload preload */
    TIM3->CR1 |= TIM_CR1_ARPE;

    /*Set the counter to 0*/ 
    TIM3->CNT = 0;

    /* Compute PSC and ARR */
    double fclk = (double)SystemCoreClock; /*APB1 = SYSCLK/2*/ 
    double t_interr = (double)PORT_PARKING_SENSOR_TRIGGER_UP_US * 1.0e-6;  /*Convert µs to seconds*/ 
    double arr_max = 65535.0;

    /*Compute initial PSC with ARR max*/ 
    double psc_temp = (fclk * t_interr) / (arr_max + 1.0) - 1.0;
    psc_temp = round(psc_temp);  /*Round to Integer*/

    /*Re-compute ARR with the PSC value*/ 
    double arr_temp = (fclk * t_interr) / (psc_temp + 1.0) - 1.0;
    arr_temp = round(arr_temp);

    /* Adjust if ARR exceeds ARR max*/ 

    while (arr_temp > 65535.0)
    {
        psc_temp += 1.0;
        arr_temp = (fclk * t_interr) / (psc_temp + 1.0) - 1.0;
        arr_temp = round(arr_temp);
    }
    
    /*Load PSC and ARR into registers*/
    TIM3->PSC = (uint32_t)psc_temp;
    TIM3->ARR = (uint32_t)arr_temp;

    /*Generate an update event to load preload registers*/ 
    TIM3->EGR = TIM_EGR_UG;

    /*Clear the interrupt flag*/
    TIM3->SR = ~TIM_SR_UIF;

    /*Enable timer interrupts*/
    TIM3->DIER |= TIM_DIER_UIE;

    /* Set NVIC priority (4, 0) and enable IRQ, but do not start the timer yet*/
    NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));
    //NVIC_EnableIRQ(TIM3_IRQn);
    
}

//TODO no tengo ni puta idea de esto
/**
 * @brief Configure the timer that controls the duration of the echo signal.

This function configures the timer as input capture to measure the duration of the echo signal. port_ultrasound_init() public function to configure the timer.
 * 
 * @param ultrasound_id This ID is used to configure the timer that controls the echo signal of the ultrasound sensor.
 */
static void _timer_echo_setup(uint32_t ultrasound_id) {
     // Enable clock for TIM2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Disable the timer for configuration
    TIM2->CR1 &= ~TIM_CR1_CEN;

    // Disable all interrupts for the timer
    TIM2->DIER &= ~(TIM_DIER_UIE | TIM_DIER_CC2IE);

    // Configure prescaler and auto-reload for 1 µs resolution
    TIM2->PSC = (SystemCoreClock / 1000000) - 1; // Prescaler for 1 MHz
    TIM2->ARR = 0xFFFF; // Maximum value for 16-bit timer

    // Enable auto-reload preload
    TIM2->CR1 |= TIM_CR1_ARPE;

    // Generate an update event to load the registers
    TIM2->EGR |= TIM_EGR_UG;

    // Configure capture/compare channel as input on TI2
    TIM2->CCMR1 &= ~TIM_CCMR1_CC2S;
    TIM2->CCMR1 |= (0x1 << TIM_CCMR1_CC2S_Pos);

    // Disable input capture filter
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2F;

    // Configure edge detection as both edges
    TIM2->CCER |= TIM_CCER_CC2P | TIM_CCER_CC2NP;

    // Enable input capture on channel 2
    TIM2->CCER |= TIM_CCER_CC2E;

    // Clear all interrupt flags
    TIM2->SR &= ~(TIM_SR_UIF | TIM_SR_CC2IF);

    // Enable update and capture/compare interrupts
    TIM2->DIER |= TIM_DIER_UIE | TIM_DIER_CC2IE;

    // Set NVIC priority and enable IRQ
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
    //NVIC_EnableIRQ(TIM2_IRQn);
}

/**
 * @brief Configure the timer that controls the duration of the new measurement.
 * 
 */
static void _timer_new_measurement_setup(){ //TODO
    // Enable the required interrupts for TIM5


    /* Enable clock (TIM5 on APB1) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;

    /* Disable counter */
    TIM5->CR1 &= ~TIM_CR1_CEN;

    /* Enable autoreload preload */
    TIM5->CR1 |= TIM_CR1_ARPE;

    /* Set counter to 0 */
    TIM5->CNT = 0;

    /* Compute PSC and ARR using the efficient algorithm */
    double fclk = (double)SystemCoreClock;  
    double t_interr = (double)(PORT_PARKING_SENSOR_TIMEOUT_MS * 1.0e-3);  
    double arr_max = 65535.0;

    double psc_d = round((fclk * t_interr - 1.0) / (arr_max + 1.0)) - 1.0;
    psc_d = round(psc_d);

    double arr_d = round((fclk * t_interr) / ((psc_d + 1.0))-1.0);
    arr_d = round(arr_d);

    if (arr_d > 65535.0){
        psc_d += 1.0;
        arr_d = round((fclk * t_interr) / (psc_d + 1.0)-1.0);
    }

    uint32_t psc = (uint32_t)(round(psc_d));
    uint32_t arr = (uint32_t)(round(arr_d));

    TIM5->PSC = psc;
    TIM5->ARR = arr;

    /* Generate update event */
    TIM5->EGR |= TIM_EGR_UG;

    /* Clear interrupt flag */
    TIM5->SR &= ~TIM_SR_UIF;

    /*Enable the interrupts of the timer by setting the UIE and CC2IE bits of the DIER register.*/
    TIM5->DIER |= TIM_DIER_UIE;

    /* Set NVIC priority (5, 0) but do not enable timer or interrupt yet */
    NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
}

void port_ultrasound_start_measurement(uint32_t ultrasound_id) {//TODO
     stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);

    if (p_ultrasound == NULL) {
        return; // Exit if the ultrasound ID is invalid
    }

    /* Reset trigger flag */
    p_ultrasound->trigger_ready = false;

    /* Reset timers */
    bool valid_ultrasound_id = (ultrasound_id < sizeof(ultrasounds_arr) / sizeof(ultrasounds_arr[0]));

    if (valid_ultrasound_id) {
        TIM_TypeDef *trigger_timer = TIM3;
        TIM_TypeDef *echo_timer = TIM2;
        TIM_TypeDef *measu_timer = TIM5;

        // Clear all interrupt flags
        trigger_timer->SR &= ~(TIM_SR_UIF | TIM_SR_CC2IF);

        // Reset the counter
        trigger_timer->CNT = 0;
        echo_timer->CNT = 0;//TODO esto es nuevo
        measu_timer->CNT = 0;

        // Enable the timer
        trigger_timer->CR1 |= TIM_CR1_CEN; // Habilita el temporizador
        echo_timer->CR1 |= TIM_CR1_CEN;
        measu_timer->CR1 |= TIM_CR1_CEN;

    }

    TIM5->SR &= ~TIM_SR_UIF; // Clear global timer interrupt flag
    TIM5->CNT = 0; // Reset global timer counter

    /* Set Trigger Pin High */
    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, true);

    /* Enable interrupts */
    if (valid_ultrasound_id) {
        NVIC_EnableIRQ(TIM3_IRQn); // Timer Trigger
    }
    NVIC_EnableIRQ(TIM5_IRQn); // Timer Global
    NVIC_EnableIRQ(TIM2_IRQn); // Timer Measurement

    //falta lo correspondiente para el TIM2

}

void port_ultrasound_start_new_measurement_timer (void){
//TODO
    /* Enable NVIC interrupt */
    NVIC_EnableIRQ(TIM5_IRQn);
    
    /* Enable new measurement timer */
    TIM5->CR1 |= TIM_CR1_CEN;
}

void port_ultrasound_stop_new_measurement_timer	(void){
    //TODO
    /* Disable new measurement timer */
    TIM5->CR1 &= ~TIM_CR1_CEN;
}

void port_ultrasound_stop_ultrasound (uint32_t ultrasound_id){
    //TODO
    if (ultrasound_id < sizeof(ultrasounds_arr) / sizeof(ultrasounds_arr[0])) {
        TIM2->CR1 &= ~TIM_CR1_CEN;
        TIM5->CR1 &= ~TIM_CR1_CEN;
    }
    TIM5->CR1 &= ~TIM_CR1_CEN;
}

void port_ultrasound_stop_echo_timer(uint32_t ultrasound_id) {
    // Implementación para detener el temporizador de eco
    TIM2->CR1 &= ~TIM_CR1_CEN; // Deshabilitar el temporizador
}

void port_ultrasound_reset_echo_ticks(uint32_t ultrasound_id) {
    // Implementación para reiniciar los ticks de eco
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL) {
        p_ultrasound->echo_init_tick = 0;
        p_ultrasound->echo_end_tick = 0;
        p_ultrasound->echo_overflows = 0;
    }
}

void port_ultrasound_init(uint32_t ultrasound_id) {
    /* Get the ultrasound sensor */
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);

    if (p_ultrasound == NULL) {
        return; // Exit if the ultrasound ID is invalid
    }

    // Initialize the fields of the ultrasound struct
    p_ultrasound->trigger_ready = true;
    p_ultrasound->trigger_end = false;
    p_ultrasound->echo_received = false;
    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_end_tick = 0;
    p_ultrasound->echo_overflows = 0;

    // Configure the trigger pin
    if (p_ultrasound->p_trigger_port != NULL) {
        stm32f4_system_gpio_config(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, STM32F4_GPIO_MODE_OUT, STM32F4_GPIO_PUPDR_NOPULL);
    }

    // Configure the echo pin
    if (p_ultrasound->p_echo_port != NULL && p_ultrasound->echo_pin < 16 && p_ultrasound->echo_alt_fun != 0) {
        // Configure the echo pin as alternate function mode
        p_ultrasound->p_echo_port->MODER &= ~(0x3 << (p_ultrasound->echo_pin * 2)); // Clear mode bits
        p_ultrasound->p_echo_port->MODER |= (STM32F4_GPIO_MODE_AF << (p_ultrasound->echo_pin * 2)); // Set alternate function mode

        // Configure the pull-up/pull-down resistor as no pull-up/pull-down
        p_ultrasound->p_echo_port->PUPDR &= ~(0x3 << (p_ultrasound->echo_pin * 2)); // Clear PUPDR bits
        p_ultrasound->p_echo_port->PUPDR |= (STM32F4_GPIO_PUPDR_NOPULL << (p_ultrasound->echo_pin * 2)); // Set no pull-up/pull-down

        // Configure the alternate function for the echo pin
        uint32_t afr_index = p_ultrasound->echo_pin / 8; // Determine AFR register index (0 or 1)
        uint32_t afr_pos = (p_ultrasound->echo_pin % 8) * 4; // Determine bit position in AFR register
        p_ultrasound->p_echo_port->AFR[afr_index] &= ~(0xF << afr_pos); // Clear existing alternate function bits
        p_ultrasound->p_echo_port->AFR[afr_index] |= (p_ultrasound->echo_alt_fun << afr_pos); // Set alternate function
    }

    // Call the private functions to configure the timers
    _timer_trigger_setup();
    _timer_echo_setup(ultrasound_id);
    _timer_new_measurement_setup();
}