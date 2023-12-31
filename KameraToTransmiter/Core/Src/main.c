/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SPIRIT_Config.h"
#include <string.h> /* memset */

#include "string.h"
#include "ArduCAM.h"
#include <stdbool.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define XTAL_FREQUENCY              50000000

/*  Radio configuration parameters  */
#define XTAL_OFFSET_PPM             0
//#define INFINITE_TIMEOUT            0.0
#define BASE_FREQUENCY              433.0e6
#define CHANNEL_SPACE               100e3
#define CHANNEL_NUMBER              0
#define MODULATION_SELECT           FSK
#define DATARATE                    38400
#define FREQ_DEVIATION              20e3
#define BANDWIDTH                   100E3

#define POWER_INDEX                 7
#define POWER_DBM                   11.6

//#define RECEIVE_TIMEOUT             2000.0 // change the value for required timeout period
#define RSSI_THRESHOLD              -120  // Default RSSI at reception, more than noise floor
//#define CSMA_RSSI_THRESHOLD         -90   // Higher RSSI to Transmit. If it's lower, the Channel will be seen as busy.

///*  Packet configuration parameters  */
#define PREAMBLE_LENGTH             PKT_PREAMBLE_LENGTH_04BYTES
#define SYNC_LENGTH                 PKT_SYNC_LENGTH_4BYTES
#define SYNC_WORD                   0x88888888
#define LENGTH_TYPE                 PKT_LENGTH_VAR
#define LENGTH_WIDTH                7
#define CRC_MODE                    PKT_CRC_MODE_8BITS
#define CONTROL_LENGTH              PKT_CONTROL_LENGTH_0BYTES
#define EN_ADDRESS                  S_ENABLE
#define EN_FEC                      S_DISABLE
#define EN_WHITENING                S_ENABLE

#define EN_FILT_MY_ADDRESS          S_ENABLE
#define EN_FILT_MULTICAST_ADDRESS   S_ENABLE
#define EN_FILT_BROADCAST_ADDRESS   S_ENABLE
#define MY_ADDRESS                  0x44
#define MULTICAST_ADDRESS           0xEE
#define BROADCAST_ADDRESS           0xFF

#define MAX_BUFFER_LEN              96
#define MAX_PAYLOAD_LEN             126 // (2^7 - 1) - 1 - 0 = 126 (LENGTH_WID=7, 1 address byte, & 0 control bytes)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void SPSGRF_Init(void);
void SPSGRF_StartTx(uint8_t *txBuff, uint8_t txLen);
void SPSGRF_StartRx(void);
uint8_t SPSGRF_GetRxData(uint8_t *rxBuff);


int wrSensorRegs16_8(const struct sensor_reg reglist[]);
void initCam();
uint8_t wrSensorReg16_8(int regID, int regDat);
uint8_t rdSensorReg16_8(uint16_t regID, uint8_t* regDat);
void write_reg(int address,int value);
uint8_t get_bit(uint8_t addr, uint8_t bit);
volatile bool checkForLastBit(uint8_t temp, uint8_t temp_last);
volatile bool checkForFirstBit(uint8_t temp, uint8_t temp_last);



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile SpiritFlagStatus xTxDoneFlag;
volatile SpiritFlagStatus xRxDoneFlag;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t payload[40];



	uint8_t temp = 0;
	uint8_t count = 0;
	uint8_t spi_recv_buf = 0;
	uint8_t spi_buf;
	uint8_t tempData, tempData_last;

	uint8_t vid, pid;
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

	//camera init
	HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	//Check if SPI  communication with camera module is working
	while (spi_recv_buf != 0x55) {
		spi_buf = 0x00 | 0x80;
		HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_RESET);
		temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);

		spi_buf = 0x55;
		temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);
		HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_SET);

		spi_buf = 0x00;
		HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_RESET);
		temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);

		spi_buf = 0x55;
		temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);
		HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_SET);

		if (spi_recv_buf != 0x55) {
			HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin); //Toogle diode to show if  Camera hasnt responded, or we dont recive correct data
			HAL_Delay(1000);
		} else {
			HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin); //Tooge diode when Camera  responds correctly
			HAL_Delay(1000);

		}
	}
	//Check if the camera module type is OV5642
	wrSensorReg16_8(0xff, 0x01);
	rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
	rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
	//Check if camera module responds
	if ((vid != 0x56) || (pid != 0x42)) {
		//Serial.println("Can't find OV5642 module!");
		while (1);
	}

	// init cam
	initCam();

	// Write ARDUCHIP_TIM, VSYNC_LEVEL_MASK to spi
	write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);

	//Change picture size
	wrSensorRegs16_8(ov5642_2592x1944);

	// Close auto exposure mode
	//uint8_t _x3503;
	//wrSensorReg16_8(0x5001,_x3503|0x01);
	//Manually set the exposure value
	wrSensorReg16_8(0x3500, 0x00);
	wrSensorReg16_8(0x3501, 0x79);
	wrSensorReg16_8(0x3502, 0xe0);

	 /*
	 *
	 *
	 *
	 * Spirit 1 init
	 *
	 *
	 *
	 */

	SpiritEnterShutdown();
	SpiritExitShutdown();
	SpiritManagementWaExtraCurrent();

	do {
		for (volatile uint8_t i = 0; i != 0xFF; i++)
			; // delay for state transition
		SpiritRefreshStatus(); // reads the MC_STATUS register
	} while (g_xStatus.MC_STATE != MC_STATE_READY);

	SRadioInit xRadioInit;

	// Initialize radio RF parameters
	xRadioInit.nXtalOffsetPpm = XTAL_OFFSET_PPM;
	xRadioInit.lFrequencyBase = BASE_FREQUENCY;
	xRadioInit.nChannelSpace = CHANNEL_SPACE;
	xRadioInit.cChannelNumber = CHANNEL_NUMBER;
	xRadioInit.xModulationSelect = MODULATION_SELECT;
	xRadioInit.lDatarate = DATARATE;
	xRadioInit.lFreqDev = FREQ_DEVIATION;
	xRadioInit.lBandwidth = BANDWIDTH;
	SpiritRadioSetXtalFrequency(XTAL_FREQUENCY); // Must be called before SpiritRadioInit()
	SpiritRadioInit(&xRadioInit);

	// Set the transmitter power level
	SpiritRadioSetPALeveldBm(POWER_INDEX, POWER_DBM);
	SpiritRadioSetPALevelMaxIndex(POWER_INDEX);

	PktBasicInit xBasicInit;
	PktBasicAddressesInit xBasicAddress;

	// Configure packet handler to use the Basic packet format
	xBasicInit.xPreambleLength = PREAMBLE_LENGTH;
	xBasicInit.xSyncLength = SYNC_LENGTH;
	xBasicInit.lSyncWords = SYNC_WORD;
	xBasicInit.xFixVarLength = LENGTH_TYPE;
	xBasicInit.cPktLengthWidth = LENGTH_WIDTH;
	xBasicInit.xCrcMode = CRC_MODE;
	xBasicInit.xControlLength = CONTROL_LENGTH;
	xBasicInit.xAddressField = EN_ADDRESS;
	xBasicInit.xFec = EN_FEC;
	xBasicInit.xDataWhitening = EN_WHITENING;
	SpiritPktBasicInit(&xBasicInit);

	// Configure destination address criteria for automatic packet filtering
	xBasicAddress.xFilterOnMyAddress = EN_FILT_MY_ADDRESS;
	xBasicAddress.cMyAddress = MY_ADDRESS;
	xBasicAddress.xFilterOnMulticastAddress = EN_FILT_MULTICAST_ADDRESS;
	xBasicAddress.cMulticastAddress = MULTICAST_ADDRESS;
	xBasicAddress.xFilterOnBroadcastAddress = EN_FILT_BROADCAST_ADDRESS;
	xBasicAddress.cBroadcastAddress = BROADCAST_ADDRESS;
	SpiritPktBasicAddressesInit(&xBasicAddress);

	SGpioInit xGpioInit;

	// Configure GPIO3 as interrupt request pin (active low)
	xGpioInit.xSpiritGpioPin = SPIRIT_GPIO_3;
	xGpioInit.xSpiritGpioMode = SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP;
	xGpioInit.xSpiritGpioIO = SPIRIT_GPIO_DIG_OUT_IRQ;
	SpiritGpioInit(&xGpioInit);

	// Generate an interrupt request for the following IRQs
	SpiritIrqDeInit(NULL);
	SpiritIrq(TX_DATA_SENT, S_ENABLE);
	SpiritIrq(RX_DATA_READY, S_ENABLE);
	SpiritIrq(RX_DATA_DISC, S_ENABLE);
	SpiritIrq(RX_TIMEOUT, S_ENABLE);
	SpiritIrqClearStatus();

	// Enable the synchronization quality indicator check (perfect match required)
	// NOTE: 9.10.4: "It is recommended to always enable the SQI check."
	SpiritQiSetSqiThreshold(SQI_TH_0);
	SpiritQiSqiCheck(S_ENABLE);

	// Set the RSSI Threshold for Carrier Sense (9.10.2)
	// NOTE: CS_MODE = 0 at reset
	SpiritQiSetRssiThresholddBm(RSSI_THRESHOLD);

	// Configure the RX timeout
#ifdef RECEIVE_TIMEOUT
	  SpiritTimerSetRxTimeoutMs(2000.0);
	  #else
	SET_INFINITE_RX_TIMEOUT();
#endif /* RECIEVE_TIMEOUT */
	SpiritTimerSetRxTimeoutStopCondition(SQI_ABOVE_THRESHOLD);

	SpiritPktBasicSetDestinationAddress(0x44);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		// camera code
		//Clear fifo flag
		write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);

		// Start capture
		write_reg(ARDUCHIP_FIFO, FIFO_RDPTR_RST_MASK);

		///Flush FIFO buffer
		write_reg(ARDUCHIP_FIFO, FIFO_WRPTR_RST_MASK);

		//Clear fifo flag
		write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);

		// Start capture
		write_reg(ARDUCHIP_FIFO, FIFO_START_MASK);

		//Wait for capture to be done
		while (get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) == 0);
		//HAL_Delay(2000);

		//Start to read data from FIFO
		//Set CS to low
		HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_RESET);
		//Send start command

		//spi_buf = BURST_FIFO_READ;
		spi_buf = SINGLE_FIFO_READ;
		temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);

		//Read FIFO data
		spi_buf = 0x00;
		do {

			tempData_last = tempData;

			temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1,
					100);
			payload[count] = spi_recv_buf;
			if (tempData == 0x00) {
				HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin); //Reset LCD screen
			}
			count = count + 1;
			if (count == 40) {
				xTxDoneFlag = S_RESET;
				SPSGRF_StartTx(&payload, strlen(payload));
				while (!xTxDoneFlag);
				count = 0;
				memset(payload, '\0', strlen(payload));
			}

			tempData = spi_recv_buf;
			if (!checkForFirstBit(tempData, tempData_last)) {
				HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin); //Reset LCD screen
			}

		} while (checkForLastBit(tempData, tempData_last));

		HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_SET);

		HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);

		HAL_Delay(1000); // Block for 2000 ms

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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */



void SPSGRF_StartTx(uint8_t *txBuff, uint8_t txLen) {
	// flush the TX FIFO
	SpiritCmdStrobeFlushTxFifo();

	// Avoid TX FIFO overflow
	//txLen = (txLen > MAX_BUFFER_LEN ? txLen : MAX_BUFFER_LEN);
	txLen = (txLen > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : txLen);

	// start TX operation
	SpiritSpiWriteLinearFifo(txLen, txBuff);
	SpiritPktBasicSetPayloadLength(txLen);
	SpiritCmdStrobeTx();
}

void SPSGRF_StartRx(void) {
	SpiritCmdStrobeRx();
}

uint8_t SPSGRF_GetRxData(uint8_t *rxBuff) {
	uint8_t len;

	len = SpiritLinearFifoReadNumElementsRxFifo();
	SpiritSpiReadLinearFifo(len, rxBuff);

	return len;
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
	SpiritIrqs xIrqStatus;

	if (GPIO_Pin != SPIRIT1_GPIO3_Pin) {
		return;
	}

	SpiritIrqGetStatus(&xIrqStatus);
	if (xIrqStatus.IRQ_TX_DATA_SENT) {
		xTxDoneFlag = S_SET;
	}
	if (xIrqStatus.IRQ_RX_DATA_READY) {
		xRxDoneFlag = S_SET;
	}
	if (xIrqStatus.IRQ_RX_DATA_DISC || xIrqStatus.IRQ_RX_TIMEOUT) {
		SpiritCmdStrobeRx();
	}
}

//camera functions

int wrSensorRegs16_8(const struct sensor_reg reglist[]) {

	unsigned int reg_addr;
	unsigned char reg_val;
	const struct sensor_reg *next = reglist;

	while ((reg_addr != 0xffff) | (reg_val != 0xff)) {

		reg_addr = pgm_read_word(&next->reg);
		reg_val = pgm_read_word(&next->val);

		wrSensorReg16_8(reg_addr, reg_val);

		next++;

	}
	return 1;
}

uint8_t rdSensorReg16_8(uint16_t regID, uint8_t *regDat) {

	uint8_t I2C_buf_register[2];

	I2C_buf_register[0] = regID >> 8;
	I2C_buf_register[1] = regID & 0x00FF;
	HAL_I2C_Master_Transmit(&hi2c1, 0x78, I2C_buf_register, 2, HAL_MAX_DELAY);

	HAL_I2C_Master_Receive(&hi2c1, 0x79, regDat, 1, HAL_MAX_DELAY);

	HAL_Delay(1);

	return 1;
}

uint8_t wrSensorReg16_8(int regID, int regDat) {
	uint8_t ret = 0;
	uint8_t I2C_buf_register[3];

	I2C_buf_register[0] = regID >> 8;
	I2C_buf_register[1] = regID & 0x00FF;
	I2C_buf_register[2] = regDat & 0x00FF;
	ret = HAL_I2C_Master_Transmit(&hi2c1, 0x78, I2C_buf_register, 3,
			HAL_MAX_DELAY);

	return 1;
}

void write_reg(int address, int value) {
	uint8_t temp = 0;
	uint8_t spi_recv_buf = 0;
	uint8_t spi_buf;

	spi_buf = address | 0x80;
	HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_RESET);
	temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);

	spi_buf = value;
	temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);
	HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_SET);

}

uint8_t read_reg(int address) {
	uint8_t temp = 0;
	uint8_t spi_recv_buf = 0;
	uint8_t spi_buf;

	spi_buf = address;
	HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_RESET);
	temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);

	spi_buf = 0x00;
	temp = HAL_SPI_TransmitReceive(&hspi2, &spi_buf, &spi_recv_buf, 1, 100);
	HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_SET);
	return spi_recv_buf;

}

uint8_t get_bit(uint8_t addr, uint8_t bit) {
	uint8_t temp;
	temp = read_reg(addr & 0x7F);
	temp = temp & bit;
	return temp;
}

bool checkForLastBit(uint8_t temp, uint8_t temp_last) {
	if (temp != 0xD9) {
		return true;
	}
	if (temp_last != 0xFF) {
		return true;
	}
	return false;
}

bool checkForFirstBit(uint8_t temp, uint8_t temp_last) {
	if (temp != 0xD8) {
		return true;
	}
	if (temp_last != 0xFF) {
		return true;
	}
	return false;
}

void initCam() {

	wrSensorReg16_8(0x3008, 0x80);
	wrSensorRegs16_8(OV5642_QVGA_Preview);

	HAL_Delay(200);

	wrSensorRegs16_8(OV5642_JPEG_Capture_QSXGA);
	wrSensorRegs16_8(ov5642_320x240);
	HAL_Delay(100);

	wrSensorReg16_8(0x3818, 0xa8);
	wrSensorReg16_8(0x3621, 0x10);
	wrSensorReg16_8(0x3801, 0xb0);
	wrSensorReg16_8(0x4407, 0x08);
	wrSensorReg16_8(0x5888, 0x00);
	wrSensorReg16_8(0x5000, 0xFF);
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
