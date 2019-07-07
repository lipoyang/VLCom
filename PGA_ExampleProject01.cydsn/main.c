/*******************************************************************************
* File: main.c
*
* Version: 1.2
*
* Description: 
*  This is a source code for basic functionality of PGA.
*  Main function performs following functions:
*  1. Initializes the LCD and clears the display
*  2. Starts PGA component
*  3. Sets PGA gain to 1
*  4. Sets PGA power to medium power
*  5. Print test name and Gain on the LCD
*
********************************************************************************
* Copyright 2012, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include <project.h>

int main()
{
    /* Start the LCD component */
    LCD_Char_1_Start();

    /* Clear LCD Display */
    LCD_Char_1_ClearDisplay();

    /* Starts PGA component */
    PGA_1_Start();
    
    /* Sets the PGA gain to 1 */
    PGA_1_SetGain(PGA_1_GAIN_01);
    
    /* Sets the power mode to medium power */
    PGA_1_SetPower(PGA_1_MEDPOWER);
    
    LCD_Char_1_Position(0u, 0u);
    LCD_Char_1_PrintString("    PGA DEMO");

    LCD_Char_1_Position(1u, 0u);
    LCD_Char_1_PrintString("    Gain = 1");

    for(;;)
    {
    }
}


/* [] END OF FILE */
