/**
  ******************************************************************************
  * @file    Based on Audio/Audio_playback_and_record/Src/waverecorder.c
  *
  * @brief   This file provides the loopback interface API
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "soundloop.h"
#include "reverb.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TOUCH_RECORD_XMIN       300
#define TOUCH_RECORD_XMAX       340
#define TOUCH_RECORD_YMIN       212
#define TOUCH_RECORD_YMAX       252

#define TOUCH_STOP_XMIN         205
#define TOUCH_STOP_XMAX         245
#define TOUCH_STOP_YMIN         212
#define TOUCH_STOP_YMAX         252

#define TOUCH_PAUSE_XMIN        125
#define TOUCH_PAUSE_XMAX        149
#define TOUCH_PAUSE_YMIN        212
#define TOUCH_PAUSE_YMAX        252

#define TOUCH_VOL_MINUS_XMIN    20
#define TOUCH_VOL_MINUS_XMAX    70
#define TOUCH_VOL_MINUS_YMIN    212
#define TOUCH_VOL_MINUS_YMAX    252

#define TOUCH_VOL_PLUS_XMIN     402
#define TOUCH_VOL_PLUS_XMAX     452
#define TOUCH_VOL_PLUS_YMIN     212
#define TOUCH_VOL_PLUS_YMAX     252

uint8_t pHeaderBuff[44];

#define SCRATCH_BUFF_SIZE  512

int32_t Scratch[SCRATCH_BUFF_SIZE];

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES (static AUDIO_IN_BufferTypeDef  BufferCtl);
ALIGN_32BYTES (static AUDIO_OUT_BufferTypeDef  outBufferCtl);

static __IO uint32_t uwVolume = 100;
static uint32_t  display_update = 1;

/* Private function prototypes -----------------------------------------------*/
static void AUDIO_REC_DisplayButtons(void);

/* Private functions ---------------------------------------------------------*/

/*  
  Four MEMS microphone MP34DT01 mounted on STM32F769I-DISCOVERY are connected
  to the DFSDM peripheral. In this example, only two MEMS microphonr are used. 
  The SAI is configured in master transmiter mode to play the recorded data. 
  In this mode, the SAI provides the clock to the WM8994.

  Data acquisition is performed in 16-bit PCM format and using SAI DMA mode.
  
  DMA is configured in circular mode

  In order to avoid data-loss, a 128 bytes buffer is used (BufferCtl.pdm_buff): 
   - When a DMA half transfer is detected using the call back BSP_AUDIO_IN_HalfTransfer_CallBack()
    PCM frame is saved in RecBuf.
  - The samples are then stored in USB buffer.
  - These two steps are repeated  when the DMA Transfer complete interrupt is detected
  - When half of internal USB buffer is reach, an evacuation though USB is done.
  
  To avoid data-loss:
  - IT ISR priority must be set at a higher priority than USB, this priority 
    order must be respected when managing other interrupts; 
*/

/**
  * @brief  Starts Audio streaming.    
  * @param  None
  * @retval Audio error
  */ 
AUDIO_ErrorTypeDef AUDIO_REC_Start(void)
{
  uint32_t byteswritten = 0;
  uwVolume = 100;

  AudioState = AUDIO_STATE_PRERECORD;
  AUDIO_REC_DisplayButtons();
  BSP_LCD_DisplayStringAt(247, LINE(6), (uint8_t *)"  [     ]", LEFT_MODE);

  BSP_AUDIO_IN_Init(BSP_AUDIO_FREQUENCY_16K, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
  BSP_AUDIO_IN_AllocScratch (Scratch, SCRATCH_BUFF_SIZE);
  BSP_AUDIO_IN_Record((uint16_t*)&BufferCtl.pcm_buff[0], AUDIO_IN_PCM_BUFFER_SIZE);
  BufferCtl.fptr = byteswritten;
  BufferCtl.pcm_ptr = 0;
  BufferCtl.offset = 0;
  BufferCtl.wr_state = BUFFER_EMPTY;
  BSP_LCD_DisplayStringAt(250, LINE(10), (uint8_t *)"  [PLAY ]", LEFT_MODE);
  BSP_AUDIO_OUT_Play((uint16_t*)&outBufferCtl.buff[0], AUDIO_OUT_BUFFER_SIZE);
  reverb_init(5801);
  return AUDIO_ERROR_NONE;
}


/*
 * Cop the contents of the Record buffer to the
 * Playback buffer
 *
 * If you wanted to hook into the signal and do some
 * signal processing, here is a place where you have
 * both buffers available
 *
 */
static void CopyBuffer(int16_t *pbuffer1, int16_t *pbuffer2, uint16_t BufferSize)
{
    uint32_t i = 0;
    for (i = 0; i < BufferSize; i++)
    {
    	pbuffer1[i] = reverb(pbuffer2[i], 0.697,
    					0, 5399,
						0, 4999,
						0, 4799,
						0, 1051,
						0, 337,
						0, 113);
//      	pbuffer1[i] = reverb(pbuffer2[i], 0.697,
//        					0.715, 5399,
//    						0.733, 4999,
//    						0.742, 4799,
//    						0.7, 1051,
//    						0.7, 337,
//    						0.7, 113);
    }
}

/**
  * @brief  Manages Audio process. 
  * @param  None
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_REC_Process(void)
{
  AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;
  static TS_StateTypeDef  TS_State={0};
  
  switch(AudioState)
  {
  case AUDIO_STATE_PRERECORD:
    if(TS_State.touchDetected == 1)   /* If previous touch has not been released, we don't proceed any touch command */
    {
      BSP_TS_GetState(&TS_State);
    }
    else
    {
      BSP_TS_GetState(&TS_State);
      if(TS_State.touchDetected == 1)
      {
        if ((TS_State.touchX[0] > TOUCH_STOP_XMIN) && (TS_State.touchX[0] < TOUCH_STOP_XMAX) &&
            (TS_State.touchY[0] > TOUCH_STOP_YMIN) && (TS_State.touchY[0] < TOUCH_STOP_YMAX))
        {
          AudioState = AUDIO_STATE_STOP;
        }
        else if ((TS_State.touchX[0] > TOUCH_RECORD_XMIN) && (TS_State.touchX[0] < TOUCH_RECORD_XMAX) &&
                 (TS_State.touchY[0] > TOUCH_RECORD_YMIN) && (TS_State.touchY[0] < TOUCH_RECORD_YMAX))
        {
          display_update = 1;
          AudioState = AUDIO_STATE_RECORD;
        }
      }
      else
      {
        AudioState = AUDIO_STATE_PRERECORD;
      }
    }


    break;
  case AUDIO_STATE_RECORD:
    if (display_update)
    {
      BSP_LCD_SetTextColor(LCD_COLOR_RED);    /* Display red record circle */
      BSP_LCD_FillCircle((TOUCH_RECORD_XMAX+TOUCH_RECORD_XMIN)/2,
                         (TOUCH_RECORD_YMAX+TOUCH_RECORD_YMIN)/2,
                         (TOUCH_RECORD_XMAX-TOUCH_RECORD_XMIN)/2);
      BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
      BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
      BSP_LCD_DisplayStringAt(247, LINE(6), (uint8_t *)"  [RECORD]", LEFT_MODE);
      display_update = 0;
    }
    
    if(TS_State.touchDetected == 1)   /* If previous touch has not been released, we don't proceed any touch command */
    {
      BSP_TS_GetState(&TS_State);
    }
    else
    {
      BSP_TS_GetState(&TS_State);
      if(TS_State.touchDetected == 1)
      {
        if ((TS_State.touchX[0] > TOUCH_STOP_XMIN) && (TS_State.touchX[0] < TOUCH_STOP_XMAX) &&
            (TS_State.touchY[0] > TOUCH_STOP_YMIN) && (TS_State.touchY[0] < TOUCH_STOP_YMAX))
        {
          AudioState = AUDIO_STATE_STOP;
        }
        else if ((TS_State.touchX[0] > TOUCH_PAUSE_XMIN) && (TS_State.touchX[0] < TOUCH_PAUSE_XMAX) &&
                 (TS_State.touchY[0] > TOUCH_PAUSE_YMIN) && (TS_State.touchY[0] < TOUCH_PAUSE_YMAX))
        {
          AudioState = AUDIO_STATE_PAUSE;
        }
      }
    }

    if(BufferCtl.wr_state == BUFFER_FULL)
    {
      BufferCtl.wr_state =  BUFFER_EMPTY;
    }
    break;
    
  case AUDIO_STATE_STOP:
  case AUDIO_STATE_PAUSE:

	  break;
    
  case AUDIO_STATE_RESUME:
    break;
    
  case AUDIO_STATE_NEXT:
  case AUDIO_STATE_PREVIOUS:   
    AudioState = AUDIO_STATE_RECORD;
    break;
    
  case AUDIO_STATE_WAIT:
    if(TS_State.touchDetected == 1)   /* If previous touch has not been released, we don't proceed any touch command */
    {
      BSP_TS_GetState(&TS_State);
    }
    else
    {
      BSP_TS_GetState(&TS_State);
      if(TS_State.touchDetected == 1)
      {
        if ((TS_State.touchX[0] > TOUCH_RECORD_XMIN) && (TS_State.touchX[0] < TOUCH_RECORD_XMAX) &&
            (TS_State.touchY[0] > TOUCH_RECORD_YMIN) && (TS_State.touchY[0] < TOUCH_RECORD_YMAX))
        {
          AudioState = AUDIO_STATE_RESUME;
        }
        else if ((TS_State.touchX[0] > TOUCH_PAUSE_XMIN) && (TS_State.touchX[0] < TOUCH_PAUSE_XMAX) &&
                 (TS_State.touchY[0] > TOUCH_PAUSE_YMIN) && (TS_State.touchY[0] < TOUCH_PAUSE_YMAX))
        {
          AudioState = AUDIO_STATE_RESUME;
        }
      }
    }
    break;

  case AUDIO_STATE_IDLE:
  case AUDIO_STATE_INIT: 
  default:
    /* Do Nothing */
    break;
  }
  return audio_error;
}

/**
  * @brief  Calculates the remaining file size and new position of the pointer.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  CopyBuffer((int16_t*)(outBufferCtl.buff+AUDIO_IN_PCM_BUFFER_SIZE/2),
		  (int16_t*)(BufferCtl.pcm_buff+AUDIO_IN_PCM_BUFFER_SIZE/2),
		  AUDIO_OUT_BUFFER_SIZE/2);
  memset((uint8_t*)(BufferCtl.pcm_buff),0, AUDIO_OUT_BUFFER_SIZE/2);
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{ 
  CopyBuffer((int16_t*)(outBufferCtl.buff),
		  	  (int16_t*)(BufferCtl.pcm_buff ),
			  AUDIO_IN_PCM_BUFFER_SIZE/2);
	  memset((uint8_t*)(BufferCtl.pcm_buff),0, AUDIO_IN_PCM_BUFFER_SIZE/2);
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/


/**
  * @brief  Display interface touch screen buttons
  * @param  None
  * @retval None
  */
static void AUDIO_REC_DisplayButtons(void)
{
  BSP_LCD_SetFont(&LCD_LOG_HEADER_FONT);
  BSP_LCD_ClearStringLine(13);            /* Clear dedicated zone */
  BSP_LCD_ClearStringLine(14);
  BSP_LCD_ClearStringLine(15);

  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
  BSP_LCD_FillCircle((TOUCH_RECORD_XMAX+TOUCH_RECORD_XMIN)/2, /* Record circle */
                     (TOUCH_RECORD_YMAX+TOUCH_RECORD_YMIN)/2,
                     (TOUCH_RECORD_XMAX-TOUCH_RECORD_XMIN)/2);
  BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);    /* Pause rectangles */
  BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + 20, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
  BSP_LCD_FillRect(TOUCH_STOP_XMIN, TOUCH_STOP_YMIN , /* Stop rectangle */
                   TOUCH_STOP_XMAX - TOUCH_STOP_XMIN,
                   TOUCH_STOP_YMAX - TOUCH_STOP_YMIN);
  BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
  BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
  BSP_LCD_DisplayStringAtLine(15, (uint8_t *)"Use record button to start record, stop to exit");
  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
}

