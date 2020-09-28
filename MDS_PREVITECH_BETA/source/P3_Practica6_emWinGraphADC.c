/**
 * Universidad de la Salle Bajío
 * BE. Francisco Rafael Flores de Maria y Campos
 * Electronic and Telecommunications Engineering
 * Practice 6: Analog Signals Display.
 * __________
 *           |
 *   		 |=> LCD
 *           |
 *      ACH4 |<- J12[2] (Potenciometer)
 *      ACH5 |<- J12[4] (Potenciometer)
 *      ACH6 |<- J12[5] (Potenciometer)
 *    ACH0 <-| TempSensor
 * __________|
 * Displays a selected function with selectid cycles in a graph as well as
 * the readings of the 3 ADC external Channels plus the internal temperature
 * sensor at the same time.
 */
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "fsl_debug_console.h"
#include "emwin_support.h"
/* TODO: insert other include files here. */
#include "GUI.h"
#include "GUIDRV_Lin.h"
#include "BUTTON.h"
#include "GRAPH.h"
#include "LISTBOX.h"
#include "math.h"
#include "pin_mux.h"
#include "fsl_sctimer.h"
#include "fsl_adc.h"
#include "fsl_clock.h"
#include "fsl_power.h"
/* TODO: insert other definitions and declarations here. */
/*** General LCD Configuration ***/
/* SDRAM Config */
#define SDRAM_BASE_ADDR 0xa0000000
#define SDRAM_SIZE_BYTES (8 * 1024 * 1024)
/* Display Config */
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
/* Work Memory for LCD */
#define GUI_NUMBYTES 0x20000
#define GUI_MEMORY_ADDR (SDRAM_BASE_ADDR)
/* Display framebuffer */
#define GUI_BUFFERS 2
#define VRAM_ADDR (GUI_MEMORY_ADDR + GUI_NUMBYTES)
#define VRAM_SIZE (LCD_HEIGHT * LCD_WIDTH * LCD_BITS_PER_PIXEL / 8)
/*******************************************************************************
 * General LCD Handler
 ******************************************************************************/
void InitPWM(void);
static void cbBackgroundWin(WM_MESSAGE *pMsg);
void InitWindow(void);
void DisplayTitle(void);
/*******************************************************************************
 * Button Handler
 ******************************************************************************/

/*******************************************************************************
 * Graph Handler
 ******************************************************************************/

/*******************************************************************************
 * Data Handler
 ******************************************************************************/

/*******************************************************************************
 * LISTBOX Handler
 ******************************************************************************/

/*******************************************************************************
 * ADC Handler
 ******************************************************************************/

int main(void) {

    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
	CLOCK_AttachClk(kMAIN_CLK_to_LCD_CLK);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
    CLOCK_EnableClock(kCLOCK_Gpio2);
    CLOCK_SetClkDiv(kCLOCK_DivLcdClk, 1, true);

    BOARD_InitPins();
    BOARD_BootClockPLL180M();
    BOARD_InitDebugConsole();
    BOARD_InitSDRAM();

    InitWindow();
    WM_Exec();

    while(1) {
        if (BOARD_Touch_Poll())
        {
            GUI_MULTIBUF_Begin();
            WM_Exec();
            WM_Paint(WM_HBKWIN);
            GUI_MULTIBUF_End();
        }
    }
    return 0 ;
}
/*******************************************************************************
 * General LCD Handler
 ******************************************************************************/
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

static void cbBackgroundWin(WM_MESSAGE *pMsg)
{
    int widget_id;

    switch (pMsg->MsgId)
    {
        case WM_NOTIFY_PARENT:
            widget_id = WM_GetId(pMsg->hWinSrc);
            break;
        default:
            WM_DefaultProc(pMsg);
    }
}

void InitWindow(void)
{
	InitPWM();

    GUI_Init();

    WM_SetSize(WM_HBKWIN, LCD_WIDTH, LCD_HEIGHT);
    WM_SetDesktopColor(GUI_BLUE_98);
    WM_SetCallback(WM_HBKWIN, cbBackgroundWin);
    GUI_SetBkColor(GUI_BLUE_98);
    GUI_Clear();
}
