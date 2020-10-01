/**
 * Universidad de la Salle Bajío
 * BE. Francisco Rafael Flores de Maria y Campos
 * BE. Luis Manuel Rico Chávez
 * Electronic and Telecommunications Engineering
 * Monitoring and Diagnosis System for Industrial Machines
 * __________
 *           |
 *   		 |=> LCD
 *           |
 * __________|
 * This is the interface for the system that will communicate the sensors with
 * the DB developed through a LabView Application. This will show the measurements
 * in real time as well as the graphics for a selected time.
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
#include "fsl_usart.h"
#include "fsl_ctimer.h"
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

/*******************************************************************************
 * CTIMER Handler
 ******************************************************************************/
/*Definitions for ctimer handler*/
#define CTIMER CTIMER3
#define CTIMER_CLK_FREQ CLOCK_GetFreq(kCLOCK_AsyncApbClk)
#define CTIMER_MAT0_OUT kCTIMER_Match_0
/*Variables for ctimer handler*/
static ctimer_match_config_t matchConfig0; //config variable for the matching value
unsigned int cont = 0; //aux counter for interrupt
void ctimer_match0_callback(uint32_t flags); //callback function
ctimer_callback_t ctimer_callback_table[] = {ctimer_match0_callback, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; //array of callback functions
void init_timer(); //ctimer initialization

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

    init_timer();
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
/* Inicialización de la ventada de inicio de la HMI
 * */
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

/*******************************************************************************
 * CTIMER Handler
 ******************************************************************************/
void init_timer()
{

    ctimer_config_t config;
    /* Enable the asynchronous bridge */
    SYSCON->ASYNCAPBCTRL = 1;
    /*Attaches 12MHZ clock to CTIMER source*/
    CLOCK_AttachClk(kFRO12M_to_ASYNC_APB);
    /*CTIMER configuration*/
    CTIMER_GetDefaultConfig(&config);
    CTIMER_Init(CTIMER, &config);
    /* Configuration for channel 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = CTIMER_CLK_FREQ;
    matchConfig0.outControl         = kCTIMER_Output_NoAction;
    matchConfig0.outPinInitState    = false;
    matchConfig0.enableInterrupt    = true;
    /*Callback registers*/
    CTIMER_RegisterCallBack(CTIMER, &ctimer_callback_table[0], kCTIMER_MultipleCallback);
    CTIMER_SetupMatch(CTIMER, CTIMER_MAT0_OUT, &matchConfig0);
    CTIMER_StartTimer(CTIMER);
	PRINTF("\r\nContador inicializado");
}
/* Función de callback para timer.
 * El sistema genera una interrupción cada segundo
 * entrando en la función de callback
 * */
void ctimer_match0_callback(uint32_t flags)
{
/*#TODO poner la llamada por software de los canales de ADC*/
	cont++;
	PRINTF("\r\nHAN PASADO %d segundos",cont);
}
