
/*!
 * @file DDS_Public.h
 *
 * @brief Contains a Direct Digital Synthesizer for sine wave generation.
 *
 * The output frequency of the DDS is calculated as:
 *
 *
 *                          PIR
 *          f_out = ----------------------
 *                    (2^32)  * T_ISR
 *
 *
 *
 *          PIR = f_out * (2^32) * T_ISR
 *
 *
 * The sampled sine wave output is 12 bits.  This was done so that the DDS
 * output could be directly fed to one of the PIC's PWM modules.
 *
 * @TODO Flesh out the description of the DDS including theory, equations,
 *       and perhaps a few external links.
 */

   #include <stdint.h>
 
#ifndef DDS_H
    #define DDS_H

    uint16_t DDS_service (void);
    uint16_t DDS_service_1 (void);
    uint16_t DDS_service_2 (uint32_t phase_shift);

    void DDS_on(void);

    void DDS_off(void);

    void DDS_set_PIR(uint32_t X);

#endif
