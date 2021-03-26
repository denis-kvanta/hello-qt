#include <fcntl.h>                      //Needed for SPI port
#include <sys/ioctl.h>                  //Needed for SPI port
#include <linux/spi/spidev.h>           //Needed for SPI port
// #include <unistd.h>                     //Needed for SPI port
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


// #include "socket.h"	// Just include one header for WIZCHIP


#define ZERRO 0
#define SPI_ERROR_1 "Error - Could not open SPI device"
#define SPI_ERROR_2 "Could not set SPIMode (WR)...ioctl fail"
#define SPI_ERROR_3 "Could not set SPIMode (RD)...ioctl fail"
#define SPI_ERROR_4 "Could not set SPI bitsPerWord (WR)...ioctl fail"
#define SPI_ERROR_5 "Could not set SPI bitsPerWord(RD)...ioctl fail"
#define SPI_ERROR_6 "Could not set SPI speed (WR)...ioctl fail"
#define SPI_ERROR_7 "Could not set SPI speed (RD)...ioctl fail"
#define SPI_ERROR_8 "Error - Problem transmitting spi data..ioctl"
#define SPI_ERROR_9 "Error - Could not close SPI device"

#define SPI_FILE_DESCRIPTOR "/dev/spidev0.0"
#define SIZE_OF_FILE_DESCRIPTOR 50
#define SPI_MODE SPI_MODE_0
#define SPI_CS_FOLDER 0
#define SPI_BITS_PER_WORD 8
#define SPI_SPEED_HZ 35*1000*1000
#define SPI_DELAY_USECS 0
#define SPI_RX_BUFFER 0
#define SPI_TX_BUFFER 0
#define SPI_RETURN_VALUE_RW 0
#define SPI_RETURN_STATUS_VALUE 0

#define SIZE_OF_PORT_NUMBER 4//2
#define SIZE_OF_PIN_DIRECTION 3
#define SIZE_OF_STATMENT 1

typedef struct {
const char* error_1;
const char* error_2;
const char* error_3;
const char* error_4;
const char* error_5;
const char* error_6;
const char* error_7;
const char* error_8;
const char* error_9;
}SPI_Errors;

SPI_Errors SPIErrors = {
SPI_ERROR_1,
SPI_ERROR_2,
SPI_ERROR_3,
SPI_ERROR_4,
SPI_ERROR_5,
SPI_ERROR_6,
SPI_ERROR_7,
SPI_ERROR_8,
SPI_ERROR_9
};

SPI_Errors *SPIErrorsSructPointer=&SPIErrors; //Pointer

typedef struct {
    unsigned char tx_buffer[2048];
    unsigned char rx_buffer[2048];
    uint32_t delay_usecs;
    uint32_t speed_hz;
    uint32_t bits_per_word;
    unsigned char mode;
    uint32_t return_value_rw;
    uint32_t status_value;
    int32_t cs_folder;
    const char file_descriptor[SIZE_OF_FILE_DESCRIPTOR];
}SPI_Initialization;

SPI_Initialization SPIInit = {
    {ZERRO},
    {ZERRO},
    SPI_DELAY_USECS,
    SPI_SPEED_HZ,
    SPI_BITS_PER_WORD,
    SPI_MODE,
    SPI_RETURN_VALUE_RW,
    SPI_RETURN_STATUS_VALUE,
    SPI_CS_FOLDER,
    SPI_FILE_DESCRIPTOR
};

SPI_Initialization *SPIInitStrPtr=&SPIInit; //Pointer

struct spi_ioc_transfer ioctl_struct[2] = {0};

void  my_ioctl(unsigned long cmd, unsigned char mode, const char *err){
   SPIInitStrPtr->status_value = ioctl(SPIInitStrPtr->cs_folder, cmd, &mode);
   if(SPIInitStrPtr->status_value < ZERRO)
    {
     perror(err);
     exit(1);
    }
}


int SpiOpenPort (){
    SPIInitStrPtr->status_value = -1;
    SPIInitStrPtr->cs_folder = open(SPIInitStrPtr->file_descriptor, O_RDWR);
    if (SPIInitStrPtr->cs_folder < ZERRO){
        perror(SPIErrorsSructPointer->error_1);
        exit(1);
    }

  my_ioctl(SPI_IOC_WR_MODE, SPIInitStrPtr->mode, SPIErrorsSructPointer->error_2);
  //printf("PizdUK_1: %d\n",SPIInitStrPtr->status_value);
  my_ioctl(SPI_IOC_RD_MODE, SPIInitStrPtr->mode, SPIErrorsSructPointer->error_3);
  my_ioctl(SPI_IOC_WR_BITS_PER_WORD, SPIInitStrPtr->bits_per_word, SPIErrorsSructPointer->error_4);
  my_ioctl(SPI_IOC_RD_BITS_PER_WORD, SPIInitStrPtr->bits_per_word, SPIErrorsSructPointer->error_5);
  my_ioctl(SPI_IOC_WR_MAX_SPEED_HZ, SPIInitStrPtr->speed_hz, SPIErrorsSructPointer->error_6);
  my_ioctl(SPI_IOC_RD_MAX_SPEED_HZ, SPIInitStrPtr->speed_hz, SPIErrorsSructPointer->error_7);

    return(SPIInitStrPtr->status_value);
}


int SpiClosePort (){

    SPIInitStrPtr->status_value = -1;
    SPIInitStrPtr->status_value = close(SPIInitStrPtr->cs_folder);
    if(SPIInitStrPtr->status_value < ZERRO)
    {
        perror(SPIErrorsSructPointer->error_9);
        exit(1);
    }
    return(SPIInitStrPtr->status_value);
}



//********** SPI WRITE & READ DATA **********
int _SpiRead(const unsigned char *AddrBuf, unsigned char *RxData, int Length) {

    SPIInitStrPtr->return_value_rw = -1;

    SPIInitStrPtr->tx_buffer[0] = AddrBuf[0];
    SPIInitStrPtr->tx_buffer[1] = AddrBuf[1];
    SPIInitStrPtr->tx_buffer[2] = AddrBuf[2];
    // printf("Addr: 0x%02x 0x%02x 0x%02x len=%d; ", SPIInitStrPtr->tx_buffer[0], SPIInitStrPtr->tx_buffer[1], SPIInitStrPtr->tx_buffer[2], Length);

    ioctl_struct[0].tx_buf = (unsigned long)SPIInitStrPtr->tx_buffer;
    ioctl_struct[0].rx_buf = (unsigned long)SPIInitStrPtr->rx_buffer;
    ioctl_struct[0].len = Length + 3;
    ioctl_struct[0].delay_usecs = SPIInitStrPtr->delay_usecs;
    ioctl_struct[0].speed_hz = SPIInitStrPtr->speed_hz;
    ioctl_struct[0].bits_per_word = SPIInitStrPtr->bits_per_word;
    ioctl_struct[0].cs_change = 0;

    SPIInitStrPtr->return_value_rw =
        ioctl(SPIInitStrPtr->cs_folder, SPI_IOC_MESSAGE(1), ioctl_struct);
    if (SPIInitStrPtr->return_value_rw < ZERRO) {
        perror(SPIErrorsSructPointer->error_8);
        exit(1);
    }
    memcpy(RxData, SPIInitStrPtr->rx_buffer + 3, Length);
    // printf("_SpiRead: ");
    // for (int i=0; i<Length; i++)
    //     printf("%02X ", SPIInitStrPtr->rx_buffer[i+3]);
    // printf("\n");
    return SPIInitStrPtr->return_value_rw;
}

//********** SPI WRITE & READ DATA **********
int _SpiWrite(const unsigned char *AddrBuf, const unsigned char *TxData, int Length) {

    SPIInitStrPtr->return_value_rw = -1;

    SPIInitStrPtr->tx_buffer[0] = AddrBuf[0];
    SPIInitStrPtr->tx_buffer[1] = AddrBuf[1];
    SPIInitStrPtr->tx_buffer[2] = AddrBuf[2];

    memcpy(SPIInitStrPtr->tx_buffer + 3, TxData, Length);
    // printf("Addr: 0x%02x 0x%02x 0x%02x len=%d; ", SPIInitStrPtr->tx_buffer[0], SPIInitStrPtr->tx_buffer[1], SPIInitStrPtr->tx_buffer[2], Length);
    // printf("_SpiWrite: ");
    // for (int i=0; i<Length + 3; i++)
    //     printf("%02X ", SPIInitStrPtr->tx_buffer[i]);
    // printf("\n");
    ioctl_struct[0].tx_buf = (unsigned long)SPIInitStrPtr->tx_buffer;
    ioctl_struct[0].rx_buf = 0;
    ioctl_struct[0].len = Length + 3;
    ioctl_struct[0].delay_usecs = SPIInitStrPtr->delay_usecs;
    ioctl_struct[0].speed_hz = SPIInitStrPtr->speed_hz;
    ioctl_struct[0].bits_per_word = SPIInitStrPtr->bits_per_word;
    ioctl_struct[0].cs_change = 0;

    SPIInitStrPtr->return_value_rw =
        ioctl(SPIInitStrPtr->cs_folder, SPI_IOC_MESSAGE(1), ioctl_struct);
    if (SPIInitStrPtr->return_value_rw < ZERRO) {
        perror(SPIErrorsSructPointer->error_8);
        exit(1);
    }
    return SPIInitStrPtr->return_value_rw;
}


int SpiRead (const unsigned char *AddrBuf, unsigned char *RxData, int Length){
   SpiOpenPort ();
   int ret = _SpiRead(AddrBuf, RxData, Length);
   SpiClosePort ();
   return ret;
}

//********** SPI WRITE & READ DATA **********
int SpiWrite (const unsigned char *AddrBuf, const unsigned char *TxData, int Length){
   SpiOpenPort ();
   int ret = _SpiWrite(AddrBuf, TxData, Length);
   SpiClosePort ();
   return ret;
}

