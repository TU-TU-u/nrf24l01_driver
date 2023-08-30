#include "NRF24L01.h"

#define CE_LOW 		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET)
#define CE_HIGH 	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET)
#define CS_LOW		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET)
#define CS_HIGH		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET)
#define MAX_PAYLOAD_LENGTH 32
extern SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef* spi = &hspi1;	//менять под свой

void write_register(uint8_t reg, uint8_t val)
{
	uint8_t command = NRF24L01P_CMD_W_REGISTER | reg;
	uint8_t status;
	CS_LOW;
	HAL_SPI_TransmitReceive(spi,&command,&status,1,2000);
	HAL_SPI_Transmit(spi,&val,1,2000);
	CS_HIGH;
}

uint8_t read_register(uint8_t reg)
{
    uint8_t command = NRF24L01P_CMD_R_REGISTER | reg;
    uint8_t status;
    uint8_t read_val;

    CS_LOW;
    HAL_SPI_TransmitReceive(spi, &command, &status, 1, 2000);
    HAL_SPI_Receive(spi, &read_val, 1, 2000);
    CS_HIGH;

    return read_val;
}
/*Задать канал*/
void set_rf_chanel(uint16_t MHz)
{
	 write_register(NRF24L01P_REG_RF_CH, MHz - 2400);
}
/*Задать скорость приёма/передачи*/
void set_air_data_rate(void)	//
{
	uint8_t new_rf_setup = read_register(NRF24L01P_REG_RF_SETUP) & 0xD7;

	write_register(NRF24L01P_REG_RF_SETUP,new_rf_setup);	//1Mbps
}
/**/
void set_rf_tx_output_power(void)
{
	uint8_t new_rf_setup = read_register(NRF24L01P_REG_RF_SETUP) & 0xD7;
	new_rf_setup |= 1<<1;
	write_register(NRF24L01P_REG_RF_SETUP,new_rf_setup);			//-12dBm
}
/**/
void set_address_widths(uint8_t bytes)
{
	write_register(NRF24L01P_REG_SETUP_AW ,bytes<<0);
}
/**/
void rx_set_payload_widths(uint8_t bytes)
{
	write_register(NRF24L01P_REG_RX_PW_P0,bytes);
}

void set_crc_length(void)
{
	uint8_t new_conf = read_register(NRF24L01P_REG_CONFIG);
	new_conf &= ~(1<<3);
	write_register(NRF24L01P_REG_CONFIG,new_conf);
}
/*Режим приема*/
void prx_mode(void)
{
	uint8_t new_conf = read_register(NRF24L01P_REG_CONFIG);
	new_conf |= 1<<0;
	write_register(NRF24L01P_REG_CONFIG,new_conf);
}
/*Режим передачи*/
void ptx_mode(void)
{
	uint8_t new_conf = read_register(NRF24L01P_REG_CONFIG);
	new_conf &= ~(1<<0);
	write_register(NRF24L01P_REG_CONFIG,new_conf);
}
/*Включение питания*/
void power_up(void)
{
	uint8_t new_conf = read_register(NRF24L01P_REG_CONFIG);
	new_conf |= 1<<1;
	write_register(NRF24L01P_REG_CONFIG,new_conf);
}
/*Отключение питания*/
void power_down(void)
{
	uint8_t new_conf = read_register(NRF24L01P_REG_CONFIG);
	new_conf &=~(1<<1);
	write_register(NRF24L01P_REG_CONFIG,new_conf);
}
/*Сброс всего*/
void reset(void)
{
    // Reset pins
	CE_HIGH;
    CE_LOW;

    // Reset registers
    write_register(NRF24L01P_REG_CONFIG, 0x08);
    write_register(NRF24L01P_REG_EN_AA, 0x3F);
    write_register(NRF24L01P_REG_EN_RXADDR, 0x03);
    write_register(NRF24L01P_REG_SETUP_AW, 0x03);
    write_register(NRF24L01P_REG_SETUP_RETR, 0x03);
    write_register(NRF24L01P_REG_RF_CH, 0x02);
    write_register(NRF24L01P_REG_RF_SETUP, 0x07);
    write_register(NRF24L01P_REG_STATUS, 0x7E);
    write_register(NRF24L01P_REG_RX_PW_P0, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P0, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P1, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P2, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P3, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P4, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P5, 0x00);
    write_register(NRF24L01P_REG_FIFO_STATUS, 0x11);
    write_register(NRF24L01P_REG_DYNPD, 0x00);
    write_register(NRF24L01P_REG_FEATURE, 0x00);

    // Reset FIFO
    flush_rx_fifo();
    flush_tx_fifo();
}
/*Настройка автоматической повторной передачи*/
void auto_retrancmit(uint8_t count)
{
	uint8_t new_conf = 0;
	new_conf |= count<<0;	//Количество повтроных передачь = count
	new_conf &= ~(1<<0)		//Задержка повтроной передачи 250
	write_register(NRF24L01P_REG_RF_RETR, new_conf);
}
/**/
void flush_tx_fifo(void)
{
	CS_LOW;
	uint8_t status;
	HAL_SPI_TransmitReceive(spi,NRF24L01P_CMD_FLUSH_TX,&status,1,2000);
	CS_HIGH;
}
/**/
void flush_rx_fifo(void)
{
	CS_LOW;
	uint8_t status;
	HAL_SPI_TransmitReceive(spi,NRF24L01P_CMD_FLUSH_RX,&status,1,2000);
	CS_HIGH;
}
/**/
void read_rx_fifo(uint8_t* rx_payload)
{
	uint8_t status;
	uint8_t size = 8;

	CS_LOW;
	HAL_SPI_Transmi_Receive(spi,NRF24L01P_CMD_R_RX_PAYLOAD,&status,1,2000);
	HAL_SPI_Receive(spi,rx_payload,size,2000);
	CS_HIGH;

}
/**/
void write_tx_fifo(uint8_t* tx_payload)
{
	uint8_t status;
	uint8_t size = 8;

	CS_LOW;
	HAL_SPI_Transmi_Receive(spi,NRF24L01P_CMD_W_TX_PAYLOAD,&status,1,2000);
	HAL_SPI_Transmit(spi,tx_payload,size,2000);
	CS_HIGH;

}
/*Включает радиомодуль как приёмник*/
void prx_init(uint16_t MHz)
{
	reset();			//CE = 0

	prx_mode();			//PRIM_RX = 1
	power_up();			//PWR_UP = 1

	rx_set_payload_widths();

	set_rf_chanel(MHz);
	set_air_data_rate();
	set_rf_tx_output_power();

	set_address_widths(5);
	set_crc_length();

	auto_retrancmit(5);

	CE_HIGH;			//CE = 1
}
/*Включает радиомодуль как передатчик*/
void ptx_init(uint16_t MHz)
{
	reset();			//CE = 0

	ptx_mode();			//PRIM_RX = 0
	power_up();			//PWR_UP = 1

	set_rf_chanel(MHz);
	set_air_data_rate();
	set_rf_tx_output_power();

	set_address_widths(5);
	set_crc_length();

	auto_retrancmit(5);

	CE_HIGH;			//CE = 1
}


void rx_receive(uint8_t* rx_payload)
{
    read_rx_fifo(rx_payload);
    flush_rx_fifo();
    //clear_rx_dr();

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void tx_transmit(uint8_t* tx_payload)
{
    write_tx_fifo(tx_payload);
    flush_tx_fifo();
}


