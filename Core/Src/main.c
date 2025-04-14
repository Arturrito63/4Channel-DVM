/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "font_fixedsys_mono_16.h"
#include "verdana_11.h"
#include "verdana_14.h"
#include "verdana_24.h"
#include "ili9341_parallel.h"
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

enum item_status {normal, activo, edit};
enum step_escala {x1, x10, x100};
enum step_frecuencia {mps3, mps5, mps10, mps15, mps20};
enum step_lapso {M35, M70, M105, M140};
enum step_boolean {false, true};

typedef struct {
	uint16_t muestra;   // 16bit de ADC (o 16bits MSB)
	uint8_t  lsb_bs: 2; // par de bits LSB para 18bit de ADC
	uint8_t  factor: 2; // factor de ganancia PGA: 0 - 3 =>(1, 2, 4, 8)
	uint8_t  escala: 2; // escala conmutada:  0 - 3 =>(.1, 1, 10, 100)
	uint8_t  repite: 1; // lectura repetida
	uint8_t  activo: 1; // activo=1. inactivo=0
} D_ADC;

typedef struct {
	uint16_t muestra;    //  16 bits del ADC sin signo(0 a 65.535)
	uint8_t  escala:  2; // escala/factor:  0 - 3 =>(.1, 1, 10, 100)
	uint8_t  invert:  1; // polaridad: 0= +, 1= -
	uint8_t  rebose:  1; // desdbordamiento
	uint8_t  activo:  1;
	uint8_t  reserv:  3;
} D_DVM;

typedef struct { // Almacena valores positivos y negativos
  int16_t min_v;
  int16_t med_v;
  int16_t max_v;
} D_STA;

typedef struct {
	uint8_t canal1:  1;
	uint8_t canal2:  1;
	uint8_t canal3:  1;
	uint8_t canal4:  1;
	uint8_t escala:  2;
	uint8_t  rango:  2;
} D_SET;

/*
typedef struct {
	uint32_t canal_1: 8;
	uint32_t canal_2: 8;
	uint32_t canal_3: 8;
	uint32_t canal_4: 8;
} D_PIX;
*/

union D_CFG {
	D_SET   flag;
	uint8_t byte;
} dvm_set;

union D_WORD
{
	uint32_t dword[2];
	uint16_t sword[4];
}adc_buf;

D_DVM dvm[4][10]; // [canal],[registro dvm]
D_ADC adc[4];
D_STA sta[4];
//D_PIX pix[140];
uint8_t pix[160][4]; //[registro pix][canal]


char texto[16];
float fpv;
uint8_t box_sel, _box_sel, sbox_sel, _sbox_sel, item_sel, _item_sel, set_sel, _set_sel;
uint8_t r_temp, r_count, dvm_count, ch_set, v_pos, MPS, lapso, key_delay_count, key_eval_count, beep_count, media_set;
//uint8_t bit_sel[4] = {0x01, 0x02, 0x04, 0x08};
uint8_t total_conv[3] = {40, 80, 160};
uint8_t tope_item[5] = {2, 3, 2, 2, 1};
uint8_t salto_pix[3] = { 4, 2, 1};
uint8_t beep_corto = 20;
uint8_t beep_largo = 250;
uint8_t rango_h_set[3] = {15, 25, 30}; //equivale a 1.5, 2.5 y 3.0
uint8_t rango_h = 2; //rango_h => 0 = 1.5, 1 = 2.5, 2 = 3.0
uint8_t rango_l_set[3] = {0, 10, 15}; //equivale a 0.0, 1.0 y 1.5
uint8_t rango_l = 0; //rango_l => 0 = 0.0, 1 = 1.0, 2= 1.5
uint16_t ms_count_set[4] = {1000, 320, 200, 100};
uint16_t ch_color[4] = {ILI_CYAN, ILI_MAGENTA, ILI_GREEN, ILI_YELLOW};
uint16_t ch_colorbk[4] = {ILI_CYANBK, ILI_MAGENTABK, ILI_GREENBK, ILI_YELLOWBK};
uint16_t limite_l, limite_h, ms_count, msTick;
uint16_t media_canal[4]; //Registro para las muestras de los 4 canales para calcular la media
int16_t tim_count, _tim_count;



//_Bool EncA_ST, _EncA_ST, EncB_ST, _EncB_ST;
_Bool Enc_Inc, Enc_Dec;
_Bool ItemSelec;
_Bool Enc_Move;
_Bool Btn_Pres;
_Bool Btn_Pres_Ok;
_Bool Start_Pres;
_Bool Start_Pres_Ok;
_Bool Run;
_Bool Beep_On;
_Bool rastro;
_Bool submenu;
_Bool item_set;
_Bool ms_count_ok;
_Bool key_delay_ok;
_Bool key_eval_ok;
_Bool redraw;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void DrawBoxes(void);
void SwapBox(uint8_t box, _Bool set);
void DrawGrilla(void);
void DrawRangoGrilla(void);
void DrawSelSet(uint8_t item, uint8_t set);
void UpdateSet(uint8_t item, uint8_t set);
void DrawConfig(void);
void SetLimites(void);
void DrawEscalaSet(uint8_t escala);
void DrawMPS_Set(uint8_t mps);
void DrawLapsoSet(uint8_t registros);
void DrawMediaSet(uint8_t mediaset);
void DrawRastroSet(uint8_t fantasma);
void SwapItem(uint8_t item, uint8_t set);
void DrawPix(uint8_t registro);
void DrawPixColor(uint8_t registro, _Bool dark);
void DrawPixCanal(uint8_t registro, uint8_t canal, uint16_t color);
void DrawBGCanal(uint8_t canal);
void DrawSTAStatus(uint8_t canal, _Bool activo);
void DrawTrazosBack(void);
void ReDrawTrazos(void);
void RedrawPrevios(void);
void SetSTA(void);
void DrawSTA(void);
void EncoderMenu(void);
void EncoderSubMenu(void);
void EncoderItemSet(void);
void MuestraMedia(void);
void Conversiones(void);

uint16_t SetMinima(uint8_t canal);
uint16_t SetMaxima(uint8_t canal);
uint16_t SetPromedio(uint8_t canal);


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void EncoderEvent(void);

void SysTick_Counter(void);

/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	EncoderEvent();
}
*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */

  ili_init();
  ili_rotate_display(1);
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);

  dvm_set.byte = 0b01001111;
  r_count = 0;
  dvm_count = 0;
  limite_l = 0;
  limite_h = 0;
  redraw = false;
  box_sel = 0;
  _box_sel = 0;
  sbox_sel = 0;
  _sbox_sel = 0;
  ItemSelec = false;
  set_sel = 0;
  _set_sel = 0;
  rastro = 1;
  MPS = 2; // Modo cantidad de Muestras por segundo: 0= 1, 1 = 3, 2 = 5, 3 = 10
  ms_count = ms_count_set[MPS];
  msTick = 0;
  ms_count_ok = false;
  lapso= 0; //Longitud de registro: 0 = 40, 1 = 80, 2 = 160
  media_set = 0; //Muestra media: 0 = 3, 1 = 4, 2 = 5
  submenu = false;
  item_set = false;
  key_delay_count = 0;
  key_delay_ok = true;
  Run = false;
  Beep_On = false;
  tim_count = 0;
  _tim_count = 0;

  DrawBoxes();
  DrawGrilla();
  DrawRangoGrilla();
  DrawConfig();
  SwapBox(box_sel, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

	if(key_eval_ok)
	{
		if(Start_Pres)
		{
			Start_Pres_Ok = ((GPIOA->IDR & START_Pin) >> 8) == 0;
		}

		if(Btn_Pres)
		{
			Btn_Pres_Ok = ((GPIOA->IDR & BOTON_Pin) >> 5) == 0;
		}
		key_eval_ok = false;
	}

	if (Start_Pres_Ok)
	{
		Run = !Run;
		Beep_On = true;
		if (Run)
		{
			GPIOC->BRR = LED_Pin;
			//GPIOA->BRR = LED_START_Pin;
		}
		else
		{
			GPIOC->BSRR = LED_Pin;
			//GPIOA->BSRR = LED_START_Pin;
		}
		Start_Pres = false;
		Start_Pres_Ok = false;
	}

	if (Btn_Pres_Ok || Enc_Inc || Enc_Dec)
	{
		Beep_On = true;
		if (!submenu && !item_set) EncoderMenu();
	 	if (submenu) EncoderSubMenu();
	 	if (item_set) EncoderItemSet();
	}

	if (Run)
	{
		if ((dvm_set.byte & 0x0F) && (ms_count_ok))
	 	{
			Conversiones();
	 	}
	 	if (r_count > total_conv[lapso])
	 	{
	 		r_count = 0;
	 		DrawGrilla();
	 		DrawTrazosBack();
	 	}


	}

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == BOTON_Pin) //Botón pulsado
	{
		if(key_delay_ok)
		{
			key_delay_count = 0;
			key_eval_count = 0;
			key_delay_ok = false;;
			key_eval_ok = false;
			Btn_Pres = true;
		}
	}
	else if (GPIO_Pin == START_Pin) //Botón pulsado
	{
		if(key_delay_ok)
		{
			key_delay_count = 0;
			key_eval_count = 0;
			key_delay_ok = false;;
			key_eval_ok = false;
			Start_Pres = true;
		}
	}
}

void EncoderEvent(void)
{
	int32_t tmp;
	tmp = __HAL_TIM_GET_COUNTER(&htim3);
	tim_count = (int16_t)tmp/4;
	if (tim_count < _tim_count) Enc_Dec = true;
	if (tim_count > _tim_count) Enc_Inc = true;
	_tim_count = tim_count;
}


void SysTick_Counter(void)
{
	msTick++;
	key_delay_count++;
	key_eval_count++;
	if (msTick >= ms_count)
	{
		ms_count_ok = true;
		msTick = 0;
	}
	if (key_delay_count >= 100) //Período en mSeg. mínimo entre pulsaciones
	{
		key_delay_ok = true;
		key_delay_count = 0;
	}
	if(key_eval_count >= 60) //Período en mSeg. hasta validar pulsación
	{
		key_eval_ok = true;
		key_eval_count = 0;
	}
	if(Beep_On)
	{
		beep_count++;
		GPIOA->ODR^=BEEP_Pin;
		if(beep_count > beep_corto)
		{
			Beep_On = false;
			beep_count = 0;
		}
	}
	else
	{
		GPIOA->ODR &= ~BEEP_Pin;
	}
}

// Dibuja las cajas (boxes) y muestra las etiquetas
void DrawBoxes(void)
{
	uint16_t x_pos = 0, y_pos = 0;
	ili_fill_screen(ILI_BLACK);
	ili_draw_rectangle(0, 0, 196, 150, ILI_ORANGE);

	for (uint8_t canal = 0; canal < 4; canal++)
	{
		y_pos = 38 * canal;
		ili_draw_rectangle(198, y_pos, 121, 36, ILI_ORANGE);
		y_pos +=  3;
		ili_fill_rect_fast(201, y_pos, 4, 30, ch_color[canal]); //marca color de canal
		y_pos +=  3;
		ili_draw_string_withbg(230, y_pos, " ACTIVO", ch_color[canal], ILI_BLACK, &font_fixedsys_mono_16);
	}

	ili_draw_rectangle(198, 152, 121, 88, ILI_ORANGE);
	for(uint8_t item = 0; item < 5; item++)
	{
		SwapItem(item, normal);
	}

	do
	{
		ili_fill_rect(x_pos, 152, 65, 18, ILI_ORANGE);
		x_pos += 66;
	}
	while(x_pos < 133);

	_ili_draw_fast_v_line(65, 170, 239, 1, ILI_ORANGE);
	_ili_draw_fast_v_line(131, 170, 239, 1, ILI_ORANGE);

	ili_draw_string(15, 151, "MIN V", ILI_BLACK, &font_fixedsys_mono_16);
	ili_draw_string(82, 151, "RMS V", ILI_BLACK, &font_fixedsys_mono_16);
	ili_draw_string(147,151, "MAX V", ILI_BLACK, &font_fixedsys_mono_16);
	//_ili_draw_fast_h_line(0, 172, 196, 1, ILI_ORANGE);
}


void SwapBox(uint8_t box, _Bool set)
{
	uint16_t color, y_pos;
	if (set)
	{
		color = ILI_BLUE;
	}
	else
	{
		color = ILI_ORANGE;
	}
	if (box < 4)
	{
		y_pos = 38 * box;
		ili_draw_rectangle(198, y_pos, 121, 36, color);
	}
	else
	{
		ili_draw_rectangle(198, 152, 121, 88, color);
	}

}


// Dibuja la grilla de 7x7
void DrawGrilla(void)
{
	uint16_t x_pos, y_pos;

	ili_fill_rect_fast(30, 2, 165, 148, ILI_BLACK);
	for (uint16_t linea=0; linea < 9; linea++)
	{
		y_pos = 19 + (linea * 20);
		x_pos = 32 + (linea * 20);
		if (linea < 7 ) _ili_draw_fast_h_line(30, y_pos, 195, 1, ILI_OLIVE);
		//_ili_draw_fast_h_line(37, y_pos, 193, 1, ILI_OLIVE);
		_ili_draw_fast_v_line(x_pos, 3, 148, 1, ILI_OLIVE);
	}
}


void DrawSelSet(uint8_t item, uint8_t set)
{
	switch(item)
	{
		case 0: DrawEscalaSet(set); break;
		case 1: DrawMPS_Set(set); break;
		case 2: DrawLapsoSet(set); break;
		case 3: DrawMediaSet(set); break;
		case 4: DrawRastroSet(set); break;
	}
}


void UpdateSet(uint8_t item, uint8_t set)
{
	r_count = 0;
	DrawGrilla();
	DrawSelSet(item, set);
	switch(item)
	{
		case 0: dvm_set.flag.escala = set; r_count = 0; DrawRangoGrilla(); break;
		case 1: MPS = set; ms_count = ms_count_set[set]; msTick = 0; break;
		case 2: lapso = set; DrawGrilla(); r_count = 0; break;
		case 3: media_set = set; break;
		case 4: rastro = (set != 0); DrawTrazosBack(); break;
	}
}


// Muestra la configuración (Escala, Rango)
void DrawConfig(void)
{
	//DrawEscalaSet(dvm_set.flag.escala);
	DrawMPS_Set(MPS);
	DrawLapsoSet(lapso);
	DrawMediaSet(media_set);
	DrawRastroSet(rastro); //Huella, Fantasma
}


void SetLimites(void)
{
	uint8_t _rango_l, _rango_h;

	_rango_l = rango_l;
	_rango_h = rango_h;

	if(limite_h < 1925) rango_h = 0; //si es menor que 1.6V
	if(limite_h > 1924) rango_h = 1; //si es mayor que 1.6V
	if(limite_h > 3103) rango_h = 2; //si es mayor que 2.5V

	if(limite_l > 1861) rango_l = 2; //si es mayor que 1.5
	if(limite_l < 1862) rango_l = 1; //si es menor que 1.5V
	if(limite_l < 1241) rango_l = 0; //si es menor que 1.0V

	if ((rango_l == 0) && (rango_h == 1)) rango_h = 2; //0-2.5 (no!) => 0-3.0
	if ((rango_l == 1) && (rango_h == 0)) rango_h = 1; //1-1.5 (no!) => 1-2.5
	if ((rango_l == 1) && (rango_h == 2)) rango_l = 0; //1-3.0 (no!) => 0-3.0
	if ((rango_l == 2) && (rango_h < 2))  rango_l = 1; //1.5-1.5(no!), 1.5-2.5 (no!) => 1.0-2.5

	if ((rango_l < _rango_l) || (rango_h > _rango_h)) redraw = true;
}


// Muestra la escala seleccionada según rango a la izq de la Grilla
void DrawRangoGrilla(void) // escala-> 0= x1, 1 = x10, 2 = x100
{                     // rengo-> 0=1.5v, 1=3v, 2=4.5v, 3=6v
	//float step;
	char filtro[10];
	uint8_t xfactor, y_pos, valor_v, y_min, y_dif;

	y_min = rango_l_set[rango_l];
	y_dif = rango_h_set[rango_h] - y_min;
	ili_fill_rect_fast(3, 3, 26, 147, ILI_BLACK);
	strcpy(filtro, "%");
	switch(dvm_set.flag.escala)
	{
		case 0: strcat(filtro, ".2"); xfactor = 1; break;
		case 1: strcat(filtro, "4.1"); xfactor = 10; break;
		case 2: strcat(filtro, "3.0"); xfactor = 100; break;
	}
	strcat(filtro, "f");
	for (uint16_t cont =0; cont < 7; cont++)
	{
		y_pos = 10 + (cont * 20);
		valor_v = 6 - cont;
		fpv = (float) (xfactor * y_min) /10;
		fpv += (float)(xfactor * valor_v * y_dif ) / 60;
		sprintf(texto, filtro, fpv);
		ili_draw_string(3, y_pos, texto, ILI_ORANGE, &verdana_11);
	}
}


void DrawEscalaSet(uint8_t escala)
{
	char txt[6];
	switch(escala)
	{
		case 0: strcpy(txt, " X1 "); break;
		case 1: strcpy(txt, " X10"); break;
		case 2: strcpy(txt, "X100"); break;
	}
	ili_draw_string_withbg(275, 153, txt, ILI_WHITE  , ILI_BLACK, &font_fixedsys_mono_16);
}


void DrawMPS_Set(uint8_t mps)
{
	char txt[8];
	switch(mps)
	{
		case 0: strcpy(txt, " 1P/s"); break;
		case 1: strcpy(txt, " 3P/s"); break;
		case 2: strcpy(txt, " 5P/s"); break;
		case 3: strcpy(txt, "10P/s"); break;
		//case 4: strcpy(txt, "20P/s"); break;
	}
	//ms_count = ms_count_set[mps];    // ################# ¡REVISAR!.. #################
	//msTick = 0;
	ili_draw_string_withbg(275, 170, txt, ILI_WHITE  , ILI_BLACK, &font_fixedsys_mono_16);
}

void DrawLapsoSet(uint8_t registros)
{
	char txt[8];
	switch(registros)
	{
		case 0: strcpy(txt, " 40 R") ; break;
		case 1: strcpy(txt, " 80 R") ; break;
		case 2: strcpy(txt, "160 R") ; break;
	}
	ili_draw_string_withbg(275, 186, txt, ILI_WHITE  , ILI_BLACK, &font_fixedsys_mono_16);
}

void DrawMediaSet(uint8_t mediaset)
{
	char txt[8];
	switch(mediaset)
	{
		case 0: strcpy(txt, "3Lec.") ; break;
		case 1: strcpy(txt, "4Lec.") ; break;
		case 2: strcpy(txt, "5Lec.") ; break;
	}
	ili_draw_string_withbg(275, 202, txt, ILI_WHITE  , ILI_BLACK, &font_fixedsys_mono_16);
}


void DrawRastroSet(uint8_t fantasma)
{
	char txt[5];
	switch(fantasma)
	{
		case false: strcpy(txt, " NO") ; break;
		case  true: strcpy(txt, " SI") ; break;
	}
	ili_draw_string_withbg(275, 218, txt, ILI_WHITE  , ILI_BLACK, &font_fixedsys_mono_16);
}


void SwapItem(uint8_t item, uint8_t status)
{
	uint8_t y_pos;
	uint16_t color;
	char txt[10];
	switch(status)
	{
	   case normal: color = ILI_ORANGE; break;
	   case activo: color = ILI_BLUE; break;
	   case   edit: color = ILI_CYAN; break;
	}
	switch(item)
	{
		case 0: strcpy(txt, "  ESCALA"); break;
		case 1: strcpy(txt, "   RITMO"); break;
		case 2: strcpy(txt, "LONGITUD"); break;
		case 3: strcpy(txt, "PROMEDIO"); break;
		case 4: strcpy(txt, "  RASTRO"); break;
	}
	y_pos = 153 + (item * 16);
	ili_draw_string_withbg(205, y_pos, txt, color , ILI_BLACK, &font_fixedsys_mono_16);
}



//Dibuja los pixeles correspondientes a los canales activos para el registro actual
void DrawPix(uint8_t registro)
{
	uint8_t tope = total_conv[lapso];
	if (registro < tope)
	{
		DrawPixColor(registro, 0);
	}

}


// Se le indica el registro, grupo de colo (dark si/no) e imprime los pixeles de canales
void DrawPixColor(uint8_t registro, _Bool dark)
{
	uint8_t activo;

	for (uint8_t canal = 0; canal < 4; canal++)
	{
		activo = dvm_set.byte & 0x0F;
		activo &= 0x01 << canal;
		if (activo)
		{
			if (dark)
			{
				DrawPixCanal(registro, canal, ch_colorbk[canal]);
			}
			else
			{
				DrawPixCanal(registro, canal, ch_color[canal]);
			}
		}
	}
}


// Dibuja un pixel o linea que une el valor anterior y el actual
void DrawPixCanal(uint8_t registro, uint8_t canal, uint16_t color)
{
	uint8_t registro_, y_pixtmp, _y_pixtmp;
	uint8_t x_pix, y_pix, _y_pix, alto;
	uint16_t resta = 139;

	SetLimites();
	if (redraw)
	{
		redraw = false;
		DrawGrilla();
		DrawRangoGrilla();
		RedrawPrevios();
		//ReDrawTrazos();

	}
	else
	{
		x_pix = 32 + (registro * salto_pix[lapso]);
		if(registro > 0)
		{
			registro_ = registro - 1;
		}
		else registro_ = total_conv[lapso] - 1;

		y_pixtmp = pix[registro][canal];
		_y_pixtmp = pix[registro_][canal];

		if((rango_h < 2) || (rango_l > 0))
		{
			y_pixtmp = y_pixtmp << 1;
			_y_pixtmp = _y_pixtmp << 1;
		}
		if(rango_l == 1) resta = 219;
		if(rango_l == 2) resta = 259;

		y_pix = (uint8_t)(resta - y_pixtmp);
		_y_pix = (uint8_t)(resta - _y_pixtmp);

		if (y_pix > _y_pix)
		{
			alto= _y_pix - y_pix;
		}
		else
		{
			alto= y_pix - _y_pix;
		}

		alto= _y_pix - y_pix;
		if ((y_pix < 150) && (_y_pix < 150))
		{
			if (alto > 1)
			{
				_ili_draw_fast_v_line(x_pix, y_pix, _y_pix, 1, color);
			}
			else
			{
				ili_draw_pixel(x_pix, y_pix, color);
			}

			if (lapso < 2) ili_draw_pixel(x_pix + 1, y_pix, color); 	//lapso = 1
			if (lapso < 1)												//lapso = 0
			{
				ili_draw_pixel(x_pix + 2, y_pix, color);
				ili_draw_pixel(x_pix + 3, y_pix, color);
			}
		}
	}
}



//Dibuja un rectangulo negro para borrar un canal
void DrawBGCanal(uint8_t canal)
{
	uint16_t y_pos = 3 + (38 * canal);
	ili_fill_rect_fast(206, y_pos, 110, 30, ILI_BLACK);
}

//Dibuja las curvas correspondientes a los canales activos en modo oscuro (back)
void DrawTrazosBack(void)
{
	SetLimites();
	ReDrawTrazos();
}


void ReDrawTrazos(void)
{
	uint8_t tope = total_conv[lapso];
	DrawRangoGrilla();
	if (rastro)
	{
		for (uint8_t registro = 0; registro < tope; registro++)
		{
			DrawPixColor(registro, 1);
		}
	}
	r_count = 0;
}


void RedrawPrevios(void)
{
	for (uint8_t registro = 0; registro < (r_count + 1); registro++)
	{
		DrawPixColor(registro, 0);
	}
}

void DrawSTA(void)
{
	uint8_t  y_pos, activo;
	//for (uint8_t canal = 0; canal < 4; canal++)
	for (uint8_t canal = 0; canal < 4; canal++)
	{
		activo = dvm_set.byte & 0x0F;
		activo &= 0x01 << canal;
		if (activo)
		{
			y_pos = 170 + (canal * 16);
			fpv = sta[canal].min_v * 3.3 /4095;
			sprintf(texto, "%.3f", fpv);
			ili_draw_string_withbg(13, y_pos, texto, ch_color[canal], ILI_BLACK, &verdana_14);

			fpv = sta[canal].med_v * 3.3 /4095;
			sprintf(texto, "%.3f", fpv);
			ili_draw_string_withbg(79, y_pos, texto, ch_color[canal], ILI_BLACK, &verdana_14);

			fpv = sta[canal].max_v * 3.3 /4095;
			sprintf(texto, "%.3f", fpv);
			ili_draw_string_withbg(147, y_pos, texto, ch_color[canal], ILI_BLACK, &verdana_14);
		}

	}
}


void SetSTA(void)
{
	uint16_t min_tmp, med_tmp, max_tmp, old_tmp;

	for (uint8_t canal = 0; canal < 4; canal++)
	{
		min_tmp = SetMinima(canal);
		old_tmp = sta[canal].min_v;
		if (old_tmp > 0)
		{
			sta[canal].min_v = (min_tmp + old_tmp) / 2;
		}
		else sta[canal].min_v = min_tmp;

		med_tmp = SetPromedio(canal);
		old_tmp = sta[canal].med_v;
		if (old_tmp > 0)
		{
			sta[canal].med_v = (med_tmp + old_tmp) / 2;
		}
		else sta[canal].med_v = med_tmp;

		max_tmp = SetMaxima(canal);
		old_tmp = sta[canal].max_v;
		if (old_tmp > 0)
		{
			sta[canal].max_v =  (max_tmp + old_tmp) / 2;
		}
		else sta[canal].max_v = max_tmp;
	}
}

void DrawSTAStatus(uint8_t canal, _Bool activo)
{
	char txt[10];
	if (activo)
	{
		strcpy(txt, "      ");
	}
	else
	{
		strcpy(txt, "-----");
	}
	v_pos = 170 + (16 * box_sel);
	ili_draw_string_withbg(13, v_pos, txt, ch_color[canal], ILI_BLACK, &font_fixedsys_mono_16);
	ili_draw_string_withbg(79, v_pos, txt, ch_color[canal], ILI_BLACK, &font_fixedsys_mono_16);
	ili_draw_string_withbg(147, v_pos, txt, ch_color[canal], ILI_BLACK, &font_fixedsys_mono_16);
}

uint16_t SetMinima(uint8_t canal)
{
	uint16_t dvm_temp = dvm[canal][0].muestra;

	for (uint8_t r_dvm = 1; r_dvm < 9; r_dvm++)
	{
		if (dvm[canal][r_dvm].muestra < dvm_temp) dvm_temp = dvm[canal][r_dvm].muestra;
	}
	return dvm_temp;
}


uint16_t SetMaxima(uint8_t canal)
{
	uint16_t dvm_temp = 0;

		for (uint8_t r_dvm = 0; r_dvm < 9; r_dvm++)
		{
			if (dvm[canal][r_dvm].muestra > dvm_temp) dvm_temp = dvm[canal][r_dvm].muestra;
		}
		return dvm_temp;
}


uint16_t SetPromedio(uint8_t canal)
{
	uint16_t media;
	uint32_t suma = 0;
	for (uint8_t r_dvm = 0; r_dvm < 9; r_dvm++)
	{
		suma += dvm[canal][r_dvm].muestra;
	}
	media = (uint16_t) (suma /9);
	return media;
}


void EncoderMenu(void) // Selecciona los marcos (box)
{
	_Bool box_activo;
	if (Btn_Pres_Ok)
	{
		Btn_Pres = false;
		if (box_sel < 4)
		{
			dvm_set.byte ^= 1 << box_sel; // Activa/Desactiva canal
			box_activo = dvm_set.byte & (1 << box_sel);
			if (!box_activo)
			{
				DrawBGCanal(box_sel);
				DrawSTAStatus( box_sel, false);
			}
			else
			{
				if (!Run)
				{
					v_pos = 6 + (38 * box_sel);
					ili_draw_string_withbg(230, v_pos, " ACTIVO", ch_color[box_sel], ILI_BLACK, &font_fixedsys_mono_16);
					DrawSTAStatus( box_sel, true);
				}
			}


			if ((dvm_set.byte & 0x0F) == 0) //Si todos los canales estan inactivos
			{
				r_count = 0;
				sprintf(texto, "%3u", r_count);
				ili_draw_string_withbg(266, 154, texto, ILI_WHITE  , ILI_BLACK, &font_fixedsys_mono_16);
				DrawGrilla();
			}

		}
		else
		{
			submenu = true;
			item_sel = 0;
			SwapItem(item_sel, activo);
			//SwapBox(box_sel, false);
		}
	}

	if (!Btn_Pres_Ok)
	{
		if (Enc_Inc)
		{
			if (box_sel < 4) //Total 5 opciones (0 - 4)
			{
				_box_sel = box_sel;
				box_sel++;
			}
			else
			{
				_box_sel = box_sel;
				box_sel = 0;
			}
			Enc_Inc = false;
			Enc_Move = true;
		}

		if (Enc_Dec)
		{
			if (box_sel > 0)
			{
				_box_sel = box_sel;
				box_sel--;
			}
			else
			{
				_box_sel = box_sel;
				box_sel = 4;
			}
			Enc_Dec = false;
			Enc_Move = true;
		}
	}
	Btn_Pres_Ok = false;

	if (Enc_Move)
	{
		SwapBox(box_sel, true);
		SwapBox(_box_sel, false);
		Enc_Move = false;
	}
}

void EncoderSubMenu(void) //Selecciona los items del menu Setting
{
	if (Btn_Pres_Ok)
	{
		Btn_Pres = false;
		SwapItem(item_sel, edit);
		ItemSelec = false;
		item_set = true;
		submenu = false;
	}
	if (!Btn_Pres_Ok)
	{
		if (Enc_Inc)
		{
			if (item_sel < 4) //Total 5 opciones (0 - 4)
			{
				_item_sel = item_sel;
				item_sel++;
			}
			else
			{
				_item_sel = item_sel;
				item_sel = 0;
			}
			Enc_Inc = false;
			Enc_Move = true;
		}

		if (Enc_Dec)
		{
			if (item_sel > 0)
			{
				_item_sel = item_sel;
				item_sel--;
			}
			else
			{
				_item_sel = item_sel;
				item_sel = 4;
			}
			Enc_Dec = false;
			Enc_Move = true;
		}
	}
	Btn_Pres_Ok = false;

	if (Enc_Move)
	{
		SwapItem(item_sel, activo);
		SwapItem(_item_sel, normal);
		Enc_Move = false;
	}
}


void EncoderItemSet(void) // Selecciona los parámetros del item seleccionado
{
	uint8_t tope_set = tope_item[item_sel];
	if (!ItemSelec)
	{
		switch(item_sel)
		{
			case 0: set_sel = dvm_set.flag.escala; break;
			case 1: set_sel = MPS; break;
			case 2: set_sel = lapso; break;
			case 3: set_sel = media_set; break;
			case 4: set_sel = rastro; break;
		}
		ItemSelec = true;
	}

	if (Btn_Pres_Ok)
	{
		Btn_Pres = false;
		UpdateSet(item_sel, set_sel);
		SwapItem(item_sel, normal);
		submenu = false;
		item_set = false;
		ItemSelec = false;
	}

	if (!Btn_Pres_Ok)
	{
		if (Enc_Inc)
		{
			if (set_sel < tope_set)
			{
				_set_sel = set_sel;
				set_sel++;
			}
			else
			{
				_set_sel = set_sel;
				set_sel = 0;
			}
			Enc_Inc = false;
			Enc_Move = true;
		}

		if (Enc_Dec)
		{
			if (set_sel > 0)
			{
				_set_sel = set_sel;
				set_sel--;
			}
			else
			{
				_set_sel = set_sel;
				set_sel = tope_set;
			}
			Enc_Dec = false;
			Enc_Move = true;
		}
	}
	Btn_Pres_Ok = false;

	if (Enc_Move)
	{
		DrawSelSet(item_sel, set_sel);
		Enc_Move = false;
	}
	//set_sel = 4;
	_set_sel = set_sel;
}


void MuestraMedia(void)
{
	uint8_t ciclo, ciclo_top;
	ciclo = 0;
	ciclo_top = media_set + 2;
	if (r_count == 0)
	{
		//En todos los canales la media = 0
		for (uint8_t canal = 0; canal < 4; canal++) media_canal[canal] = 0;
	}

	while(ciclo < (ciclo_top))
	{
		if (HAL_ADC_Start_DMA(&hadc1, adc_buf.dword, 4) == HAL_OK)
		{
			HAL_Delay(5);
			for(uint8_t canal = 0; canal < 4; canal++)
			{
				ch_set = dvm_set.byte & 0x0F;
				ch_set &= 0x01 << canal;
				if (ch_set)
				{
					media_canal[canal] += adc_buf.sword[canal];
				}
			}
			ciclo++;
		}
	}
	for(uint8_t canal = 0; canal < 4; canal++)
	{
		if(media_canal[canal] > 0)
		{
			if (r_count == 0)
			{
				media_canal[canal] = media_canal[canal] / ciclo_top;
			}
			else
			{
				media_canal[canal] = media_canal[canal] / (ciclo_top + 1);
			}
		}
	}
}


void Conversiones(void)
{

	ms_count_ok = false; //Comienza nueva cuenta de tiempo
	MuestraMedia();
	if (r_count == 0)
	{
		limite_l = media_canal[0];
		limite_h = 0;
	}

	for(uint8_t canal = 0; canal < 4; canal++)
	{
		ch_set = dvm_set.byte & 0x0F;
		ch_set &= 0x01 << canal;
		if (ch_set)
		{
			dvm[canal][dvm_count].muestra = media_canal[canal];
			if (media_canal[canal] < limite_l) limite_l =  media_canal[canal];
			if( media_canal[canal] > limite_h) limite_h =  media_canal[canal];

			pix[r_count][canal] = (uint8_t)( media_canal[canal] / 31);
			fpv =  media_canal[canal] * 3.3 /4095; /* uint16_t a float en el rango de 0 a 3.3Volts */
			v_pos = 4 + (38 * canal);
			sprintf(texto, "%.3fV", fpv);
			ili_draw_string_withbg(230, v_pos, texto, ch_color[canal], ILI_BLACK, &verdana_24);
		}
	}

	dvm_count++;
	if (dvm_count > 8) //Cada 10 conversiones se calcula la mínima, media y máxima
	{
		SetSTA();
		DrawSTA();
		dvm_count = 0;
	}
	GPIOA->BRR = LED_START_Pin;
	//GPIOC->BRR = LED_Pin;  //Led ON
	sprintf(texto, "(%3u)", r_count + 1);
	ili_draw_string_withbg(160, 1, texto, ILI_WHITE, ILI_BLACK, &verdana_11);
/*
	sprintf(texto, "(%3u)", set_sel);
	ili_draw_string_withbg(155, 140, texto, ILI_WHITE, ILI_BLACK, &verdana_11);
*/
	DrawPix(r_count);
	GPIOA->BSRR = LED_START_Pin;
	//GPIOC->BSRR = LED_Pin; // Led OFF
	r_count++;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
