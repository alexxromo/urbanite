/**
 * @file interr.c
 * @brief Interrupt service routines for the STM32F4 platform.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2025-01-01
 */
// Include HW dependencies:
#include "port_system.h"
#include "stm32f4_system.h"
#include "port_button.h"
#include "port_ultrasound.h"
#include "stm32f4_button.h"
#include "stm32f4_ultrasound.h"

// Include headers of different port elements:

//------------------------------------------------------
// INTERRUPT SERVICE ROUTINES
//------------------------------------------------------
/**
 * @brief Interrupt service routine for the System tick timer (SysTick).
 *
 * @note This ISR is called when the SysTick timer generates an interrupt.
 * The program flow jumps to this ISR and increments the tick counter by one millisecond.
 *
 * > **TODO alumnos:**
 * >
 * > ✅ 1. **Increment the System tick counter `msTicks` in 1 count.** To do so, use the function `port_system_get_millis()` and `port_system_set_millis()`.
 *
 * @warning **The variable `msTicks` must be declared volatile!** Just because it is modified by a call of an ISR, in order to avoid [*race conditions*](https://en.wikipedia.org/wiki/Race_condition). **Added to the definition** after *static*.
 *
 */
void SysTick_Handler(void)
{ // TODO v1

    // Declarar una variable local para almacenar el valor actual de msTicks
    uint32_t ticksAhora;

    // valor actual de msTicks
    ticksAhora = port_system_get_millis();

    // valor anterior incrementado en 1.
    ticksAhora++;

    // valor incrementado de msTicks
    port_system_set_millis(ticksAhora);
}

// TODO v1 Comprobar los ifs, quizas son al reves
/**
 * @brief This function handles Px10-Px15 global interrupts.

First, this function identifies the line/ pin which has raised the interruption. Then, perform the desired action. Before leaving it cleans the interrupt pending register.
 *
 */
void EXTI15_10_IRQHandler(void)
{
    port_system_systick_resume(); // TODO v4 reactivar el contador del sistema SysTick

    /* ISR parking button */
    // 1. Verificar si la interrupción proviene del botón de usuario
    if (port_button_get_pending_interrupt(PORT_PARKING_BUTTON_ID))
    {

        // 2. Obtener el estado actual del botón (si está presionado o no)
        bool button_pressed = port_button_get_value(PORT_PARKING_BUTTON_ID);

        // 3. Actualizar el estado de flag_pressed según el estado del botón
        if (button_pressed)
        {
            // Si el botón está liberado, marcar flag_pressed como falso
            port_button_set_pressed(PORT_PARKING_BUTTON_ID, false);
        }
        else
        {
            // Si el botón está presionado, marcar flag_pressed como verdadero
            port_button_set_pressed(PORT_PARKING_BUTTON_ID, true);
        }

        // 4. Limpiar la interrupción escribiendo '1' en el bit correspondiente del EXTI PR
        port_button_clear_pending_interrupt(PORT_PARKING_BUTTON_ID);
    }
    // hay que hacer un toggle con el valor que tenga el bool flag_pressed. Flag pressed solo se puede ver desde su propio c
}

/**
 * @brief Interrupt service routine for the TIM3 timer.

This timer controls the duration of the trigger signal of the ultrasound sensor. When the interrupt occurs it means that the time of the trigger signal has expired and must be lowered.
 *
 */
void TIM3_IRQHandler(void) // TODO v2
{

    /* Clear the TIM3 update interrupt flag */
    TIM3->SR &= ~TIM_SR_UIF;

    /* Set the trigger end flag to true */
    port_ultrasound_set_trigger_end(PORT_REAR_PARKING_SENSOR_ID, true);
}

/**
 * @brief Interrupt service routine for the TIM2 timer.

This timer controls the duration of the echo signal of the ultrasound sensor by means of the input capture mode.
 *
 */
void TIM2_IRQHandler(void)
{ // TODO v2

    port_system_systick_resume(); // TODO v4 reactivar el contador del sistema SysTick

    // Esta ISR puede saltar por 3 motivos: (i) por un desbordamiento (overflow) del temporizador, (ii) por un flanco de subida, o (iii) por un flanco de bajada.
    /* 1. Comprobar si la interrupción es por desbordamiento (UIF flag) */
    if (TIM2->SR & TIM_SR_UIF)
    {
        uint32_t ultrasound_id = PORT_REAR_PARKING_SENSOR_ID;
        uint32_t overflows = port_ultrasound_get_echo_overflows(ultrasound_id);
        port_ultrasound_set_echo_overflows(ultrasound_id, overflows + 1);
        TIM2->SR &= ~TIM_SR_UIF; // Limpiar flag UIF
    }

    /* 2. Comprobar si la interrupción es por captura de entrada (CC2IF flag) */
    if (TIM2->SR & TIM_SR_CC2IF)
    {
        uint32_t ultrasound_id = PORT_REAR_PARKING_SENSOR_ID;
        uint32_t tick = TIM2->CCR2; // Leer el valor del contador (también limpia el flag)

        // get echo init y end --> si los 2 0 --> inicio echo, si no, bajada

        if (port_ultrasound_get_echo_init_tick(ultrasound_id) == 0 && port_ultrasound_get_echo_end_tick(ultrasound_id) == 0)
        { // Flanco de subida detectado (CC2P = 0)
            port_ultrasound_set_echo_init_tick(ultrasound_id, tick);
            // port_ultrasound_set_echo_overflows(ultrasound_id, 0); // Resetear desbordamientos
            // TIM2->CCER |= TIM_CCER_CC2P; // Configurar para capturar el siguiente flanco de bajada
        }
        else
        { // Flanco de bajada detectado (CC2P = 1)
            port_ultrasound_set_echo_end_tick(ultrasound_id, tick);
            port_ultrasound_set_echo_received(ultrasound_id, true);
            // TIM2->CCER &= ~TIM_CCER_CC2P; // Configurar para capturar el siguiente flanco de subida
        }

        // TIM2->SR &= ~TIM_SR_CC2IF; // Limpiar flag de captura de entrada
    }
}

/**
 * @brief Interrupt service routine for the TIM5 timer.

This timer controls the duration of the measurements of the ultrasound sensor. When the interrupt occurs it means that the time of the a measurement has expired and a new measurement can be started.
 *
 */
void TIM5_IRQHandler(void)
{ // TODO
    // Clear the interrupt flag UIF in the status register SR.
    TIM5->SR &= ~TIM_SR_UIF;

    // Call the function port_ultrasound_set_trigger_ready() to set the flag that indicates that a new measurement can be started.
    uint32_t ultrasound_id = PORT_REAR_PARKING_SENSOR_ID; // Asigna el ID adecuado
    port_ultrasound_set_trigger_ready(ultrasound_id, true);
}
