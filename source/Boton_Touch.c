/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*  Standard C Included Files */
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "fsl_debug_console.h"
#include "emwin_support.h"

#include "GUI.h"
#include "GUIDRV_Lin.h"
#include "BUTTON.h"

#include "pin_mux.h"
#include "fsl_sctimer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
//#define EXAMPLE_I2C_MASTER_BASE (I2C2_BASE)
//#define I2C_MASTER_CLOCK_FREQUENCY (12000000)

#define SDRAM_BASE_ADDR 0xa0000000
#define SDRAM_SIZE_BYTES (8 * 1024 * 1024)

//#define APP_LCD LCD
//#define APP_LCD_IRQHandler LCD_IRQHandler
//#define APP_LCD_IRQn LCD_IRQn

//#define LCD_PANEL_CLK 9000000
#define LCD_PPL 480
#define LCD_HSW 2
#define LCD_HFP 8
#define LCD_HBP 43
#define LCD_LPP 272
#define LCD_VSW 10
#define LCD_VFP 4
#define LCD_VBP 12
#define LCD_POL_FLAGS kLCDC_InvertVsyncPolarity | kLCDC_InvertHsyncPolarity
#define LCD_INPUT_CLK_FREQ CLOCK_GetFreq(kCLOCK_LCD)
#define LCD_WIDTH 480
#define LCD_HEIGHT 272
#define LCD_BITS_PER_PIXEL 16

/* Work memory for emWin */
#define GUI_NUMBYTES 0x20000
#define GUI_MEMORY_ADDR (SDRAM_BASE_ADDR)

/* Display framebuffer */
#define GUI_BUFFERS 2
#define VRAM_ADDR (GUI_MEMORY_ADDR + GUI_NUMBYTES)
#define VRAM_SIZE (LCD_HEIGHT * LCD_WIDTH * LCD_BITS_PER_PIXEL / 8)

#define LED3_PORT 2U
#define LED2_PORT 3U
#define LED1_PORT 3U

#define LED3_PIN 2U
#define LED2_PIN 3U
#define LED1_PIN 14U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define UP_BUTTON_ID (GUI_ID_BUTTON0)
#define RESET_BUTTON_ID   (GUI_ID_BUTTON1)
#define DOWN_BUTTON_ID   (GUI_ID_BUTTON2)

/*******************************************************************************
 * Code
 ******************************************************************************/

int valor = 0;
void InitPWM(void)
{
	sctimer_config_t config;
	sctimer_pwm_signal_param_t pwmParam;
	uint32_t evento;
	CLOCK_AttachClk(kMAIN_CLK_to_SCT_CLK);
	CLOCK_SetClkDiv(kCLOCK_DivSctClk,2,true);
	SCTIMER_GetDefaultConfig(&config);
	SCTIMER_Init (SCT0,&config);
	pwmParam.output = kSCTIMER_Out_5;
	pwmParam.level = kSCTIMER_HighTrue;
	pwmParam.dutyCyclePercent = 5;
	SCTIMER_SetupPwm(SCT0,&pwmParam,kSCTIMER_CenterAlignedPwm,1000U,CLOCK_GetSctClkFreq(),&evento);
}

void LED_config()
{
	gpio_pin_config_t led_config = {
				kGPIO_DigitalOutput,
				1
	};
	GPIO_PortInit(GPIO,LED3_PORT);
	GPIO_PinInit(GPIO,LED3_PORT,LED3_PIN,&led_config);
	GPIO_PortInit(GPIO,LED2_PORT);
	GPIO_PinInit(GPIO,LED2_PORT,LED2_PIN,&led_config);
	GPIO_PinInit(GPIO,LED1_PORT,LED1_PIN,&led_config);
	GPIO_PortMaskedSet(GPIO,LED2_PORT,0x0000FFFF);
	GPIO_PortMaskedWrite(GPIO,LED2_PORT,0xFFFFFFFF);
	GPIO_PortMaskedSet(GPIO,LED3_PORT,0x0000FFFF);
	GPIO_PortMaskedWrite(GPIO,LED3_PORT,0xFFFFFFFF);
}

void LSBtoLED(char x)
{
   	GPIO_PinWrite(GPIO,LED1_PORT,LED1_PIN,~(x&0x01));  //Sets BIT0 in LED0
   	GPIO_PinWrite(GPIO,LED2_PORT,LED2_PIN,~((x&0x02)>>1U)); //Sets BIT1 in LED1
   	GPIO_PinWrite(GPIO,LED3_PORT,LED3_PIN,~((x&0x04)>>2U)); //Sets BIT2 in LED2
}

static void cbBackgroundWin(WM_MESSAGE *pMsg)
{
    int widget_id;

    switch (pMsg->MsgId)
    {
        case WM_NOTIFY_PARENT:
            widget_id = WM_GetId(pMsg->hWinSrc);

			if (widget_id == UP_BUTTON_ID && pMsg->Data.v == WM_NOTIFICATION_CLICKED)
            {
				if(valor<7)
				{
					valor++;
				}
                GUI_Clear();
                GUI_SetColor(GUI_BLUE);
                GUI_DispStringHCenterAt("Lectura de \nconteo de boton",
                    		LCD_WIDTH / 2, LCD_HEIGHT / 2);
                GUI_SetColor(GUI_RED);
                GUI_DispDecAt (valor, LCD_WIDTH / 2,LCD_HEIGHT / 2 + 40,4);
            }
			if (widget_id == DOWN_BUTTON_ID && pMsg->Data.v == WM_NOTIFICATION_CLICKED)
            {
				if(valor>0)
				{
					valor--;
				}
                GUI_Clear();
                GUI_SetColor(GUI_BLUE);
                GUI_DispStringHCenterAt("Lectura de \nconteo de boton",
                    		LCD_WIDTH / 2, LCD_HEIGHT / 2);
                GUI_SetColor(GUI_RED);
                GUI_DispDecAt (valor, LCD_WIDTH / 2,LCD_HEIGHT / 2 + 40,4);
            }
			if (widget_id == RESET_BUTTON_ID && pMsg->Data.v == WM_NOTIFICATION_CLICKED)
            {
				valor=0;
                GUI_Clear();
                GUI_SetColor(GUI_BLUE);
                GUI_DispStringHCenterAt("Lectura de \nconteo de boton",
                    		LCD_WIDTH / 2, LCD_HEIGHT / 2);
                GUI_SetColor(GUI_RED);
                GUI_DispDecAt (valor, LCD_WIDTH / 2,LCD_HEIGHT / 2 + 40,4);
            }
            break;
        default:
            WM_DefaultProc(pMsg);
    }
}

int main(void)
{
    //int i;


    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
	CLOCK_AttachClk(kMAIN_CLK_to_LCD_CLK);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
    CLOCK_EnableClock(kCLOCK_Gpio2);
    CLOCK_SetClkDiv(kCLOCK_DivLcdClk, 1, true);

    BOARD_InitPins();
    BOARD_BootClockPLL180M();
    BOARD_InitDebugConsole();
    BOARD_InitSDRAM();

    LED_config();

    InitPWM();

    GUI_Init();

    WM_SetSize(WM_HBKWIN, LCD_WIDTH, LCD_HEIGHT);
    WM_SetDesktopColor(GUI_GRAY);

    WM_SetCallback(WM_HBKWIN, cbBackgroundWin);

    GUI_SetBkColor(GUI_GRAY);
    GUI_Clear();


    BUTTON_SetReactOnLevel();

    GUI_SetColor(GUI_BLUE);

    BUTTON_Handle hButtonUP;
    hButtonUP = BUTTON_CreateEx(20, 20, 100, 50, 0, WM_CF_SHOW, 0, UP_BUTTON_ID);
    BUTTON_SetText(hButtonUP, "Arriba");

    BUTTON_Handle hButtonDOWN;
    hButtonDOWN = BUTTON_CreateEx(140, 20, 100, 50, 0, WM_CF_SHOW, 0, DOWN_BUTTON_ID);
    BUTTON_SetText(hButtonDOWN, "Abajo");

    BUTTON_Handle hButtonReset;
    hButtonReset = BUTTON_CreateEx(260, 20, 100, 50, 0, WM_CF_SHOW, 0, RESET_BUTTON_ID);
    BUTTON_SetText(hButtonReset, "Reinicio");

    GUI_DispStringHCenterAt("Lectura de \nconteo de boton",
                       		LCD_WIDTH / 2, LCD_HEIGHT / 2);
    GUI_SetColor(GUI_RED);
    GUI_DispDecAt (valor, LCD_WIDTH / 2,LCD_HEIGHT / 2 + 40,4);

    WM_Exec();

    while (1)
    {
        if (BOARD_Touch_Poll())
        {
            GUI_MULTIBUF_Begin();
            WM_Exec();
            WM_Paint(WM_HBKWIN);
            GUI_MULTIBUF_End();
            LSBtoLED(valor);
        }
    }
}
