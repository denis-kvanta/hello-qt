// #include <fcntl.h>                      //Needed for SPI port
// #include <sys/ioctl.h>                  //Needed for SPI port
// #include <linux/spi/spidev.h>           //Needed for SPI port
#include "file-op.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> //Needed for SPI port

#include "socket.h" // Just include one header for WIZCHIP

/////////////////////////////////////////
// SOCKET NUMBER DEFINION for Examples //
/////////////////////////////////////////
#define SOCK_TCPS 0
#define SOCK_UDPS 1

////////////////////////////////////////////////
// Shared Buffer Definition for LOOPBACK TEST //
////////////////////////////////////////////////
#define DATA_BUF_SIZE 1024
uint8_t gDATABUF[DATA_BUF_SIZE];

///////////////////////////////////
// Default Network Configuration //
///////////////////////////////////
wiz_NetInfo gWIZNETINFO = {.mac = {0x00, 0x08, 0xdc, 0x00, 0xab, 0xcd},
                           .ip = {10, 10, 10, 199},
                           .sn = {255, 255, 255, 0},
                           .gw = {10, 10, 10, 1},
                           .dns = {0, 0, 0, 0},
                           .dhcp = NETINFO_STATIC};

// initialize the dependent host peripheral

//////////
// TODO //
//////////////////////////////////////////////////////////////////////////////////////////////
// Call back function for W5500 SPI - Theses used as parameter of
// reg_wizchip_xxx_cbfunc()  // Should be implemented by WIZCHIP users because
// host is dependent                         //
//////////////////////////////////////////////////////////////////////////////////////////////
void wizchip_select(void);
void wizchip_deselect(void);
void wizchip_write(uint8_t wb);
uint8_t wizchip_read();
void wizchip_readburst(uint8_t *pBuf, uint16_t len);
void wizchip_writeburst(uint8_t *pBuf, uint16_t len);
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////
// For example of ioLibrary_BSD //
//////////////////////////////////
void network_init(void); // Initialize Network information and display it
int32_t loopback_tcps(uint8_t, uint8_t *, uint16_t); // Loopback TCP server
int32_t loopback_udps(uint8_t, uint8_t *, uint16_t); // Loopback UDP server
//////////////////////////////////

/////////////////////////////////////////////////////////////
// Intialize the network information to be used in WIZCHIP //
/////////////////////////////////////////////////////////////
void network_init(void) {
    uint8_t tmpstr[6];
    ctlnetwork(CN_SET_NETINFO, (void *)&gWIZNETINFO);
    ctlnetwork(CN_GET_NETINFO, (void *)&gWIZNETINFO);

    // Display Network Information
    ctlwizchip(CW_GET_ID, (void *)tmpstr);
    printf("\r\n=== %s NET CONF ===\r\n", (char *)tmpstr);
    printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0],
           gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3],
           gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
    printf("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1],
           gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
    printf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1],
           gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
    printf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1],
           gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
    printf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1],
           gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
    printf("======================\r\n");
}
/////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// Loopback Test Example Code using ioLibrary_BSD      //
///////////////////////////////////////////////////////////////
int32_t loopback_tcps(uint8_t sn, uint8_t *buf, uint16_t port) {
    int32_t ret;
    uint16_t size = 0, sentsize = 0;
    switch (getSn_SR(sn)) {
    case SOCK_ESTABLISHED:
        if (getSn_IR(sn) & Sn_IR_CON) {
            printf("%d:Connected\r\n", sn);
            setSn_IR(sn, Sn_IR_CON);
        }
        if ((size = getSn_RX_RSR(sn)) > 0) {
            if (size > DATA_BUF_SIZE)
                size = DATA_BUF_SIZE;
            ret = recv(sn, buf, size);
            if (ret <= 0)
                return ret;
            sentsize = 0;
            while (size != sentsize) {
                ret = send(sn, buf + sentsize, size - sentsize);
                if (ret < 0) {
                    close_soc(sn);
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
        }
        break;
    case SOCK_CLOSE_WAIT:
        printf("%d:CloseWait\r\n", sn);
        if ((ret = disconnect(sn)) != SOCK_OK)
            return ret;
        printf("%d:Closed\r\n", sn);
        break;
    case SOCK_INIT:
        printf("%d:Listen, port [%d]\r\n", sn, port);
        if ((ret = listen(sn)) != SOCK_OK)
            return ret;
        break;
    case SOCK_CLOSED:
        printf("%d:LBTStart\r\n", sn);
        if ((ret = socket(sn, Sn_MR_TCP, port, 0x00)) != sn)
            return ret;
        printf("%d:Opened\r\n", sn);
        break;
    default:
        break;
    }
    return 1;
}

int32_t loopback_udps(uint8_t sn, uint8_t *buf, uint16_t port);
//////////////////////////////////////////////////////////////////////////////

void dumpControlRegisters() {
    return;
    printf("Control registers content\n");
    unsigned char readed[256] = {0};
    int control_reg_len = 0x40;
    WIZCHIP_READ_BUF(0x0000 << 8, readed, control_reg_len);
    for (int i = 0; i < 4; i++) {
        printf("%02X: ", i * 0x10);
        for (int j = 0; j < 0x10; j++) {
            printf("%02X ", readed[i * 0x10 + j]);
            // printf("%02x:%02X ", i*0x10 + j, readed[i*0x10 + j]);
        }
        printf("\n");
    }
    printf("----------------------\n");
}

void testRawSpiRW(uint16_t address_logic, const uint8_t *input,
                  const uint8_t *err_msg) {
    uint8_t addr[3];
    addr[0] = 0;
    addr[1] = 0xFF & address_logic;
    addr[2] = 0b100; // var len write
    printf("\nTesting testRawSpiRW\n");
    printf("Writing to 0x%02X '%s'\n", address_logic, input);
    unsigned char readed[256] = {0};
    int len = strlen(input);
    SpiWrite(addr, input, len);
    addr[2] = 0b000; // var len read
    SpiRead(addr, readed, len);
    if (strcmp(input, readed) != 0) {
        printf("Test fail. %s\n", err_msg);
        printf("len = %d\n", len);
        printf("Read %s\n", readed);
        dumpControlRegisters();
        // exit(EXIT_FAILURE);
        return;
    }
    printf("OK ----------------------\n");
}

void testSpiBUF(uint32_t address_logic, const uint8_t *input,
                const uint8_t *err_msg) {
    printf("\nTesting WIZCHIP READ WRITE BUF\n");
    printf("Writing to 0x%02X '%s'\n", address_logic, input);
    unsigned char readed[256] = {0};
    int len = strlen(input);
    WIZCHIP_WRITE_BUF(address_logic << 8, input, len);
    WIZCHIP_READ_BUF(address_logic << 8, readed, len);
    if (strcmp(input, readed) != 0) {
        printf("Test WIZCHIP READ WRITE BUF fail. %s\n", err_msg);
        dumpControlRegisters();
        // exit(EXIT_FAILURE);
        return;
    }
    printf("OK ----------------------\n");
}

void testSpiBYTE(uint32_t address_logic, uint8_t byte, const uint8_t *err_msg) {
    printf("\nTesting READ WRITE SINGLE BYTE\n");
    printf("Writing to 0x%02X '%s'\n", address_logic, &byte);
    unsigned char readed = 0;
    WIZCHIP_WRITE(address_logic << 8, byte);
    readed = WIZCHIP_READ(address_logic << 8);
    if (readed != byte) {
        printf("Test WIZCHIP READ WRITE SINGLE BYTE fail. %s\n", err_msg);
        dumpControlRegisters();
        // exit(EXIT_FAILURE);
        return;
    }
    printf("OK ----------------------\n");
}

void testW5500Version() {
    printf("Reading vertion from 0x39. It should be always 0x04\n");
    int val = WIZCHIP_READ(0x39);
    if (val == 0x04)
        printf("Test ok. Version=%d\n", val);
    else {
        printf("Test version fail. Ver = 0x%02x\n", val);
        // exit(EXIT_FAILURE);
        return;
    }
    printf("OK ----------------------\n");
}

void testW5500VersionWrite() {
    printf("Set value to addr 0x39. 0x65\n");
    WIZCHIP_WRITE(0x39, 0x65);
}

void readFirstByte() {
    int val = WIZCHIP_READ(0x00);
    int addr = 0;
    printf("From 0x%02x read value 0x%02x\n", addr, val);
}

void testSpi() {
    wizchip_sw_reset();
    usleep(1000*1000);
    dumpControlRegisters();
    testRawSpiRW(0xF, "addr", "Updating 'addr' in RAW mode");
    dumpControlRegisters();
    testRawSpiRW(0xF, "ADDR", "Updating 'ADDR' in RAW mode");
    dumpControlRegisters();
    testSpiBUF(0xF, "bcde", "Updating 'bcde' in BUF mode");
    dumpControlRegisters();
    testSpiBUF(0xF, "BCDe", "Updating 'BCDe' in BUF mode");
    dumpControlRegisters();
    testSpiBYTE(0x9, 's', "Updating 's' in BYTE mode");
    testSpiBYTE(0x9, 'S', "Updating 'S' in BYTE mode");
    dumpControlRegisters();
    testW5500Version();

    // testW5500VersionWrite();
    // dumpControlRegisters();
    // testW5500Version();
    // readFirstByte();
    // dumpControlRegisters();

    // exit(EXIT_FAILURE);
}

//////////////////////////////////////////////////////////////////////////

int main(void) {
    uint8_t tmp;
    int32_t ret = 0;
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2},
                             {2, 2, 2, 2, 2, 2, 2, 2}};

    ///////////////////////////////////////////
    // Host dependent peripheral initialized //
    ///////////////////////////////////////////

    //////////
    // TODO //
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // First of all, Should register SPI callback functions implemented by user
    // for accessing WIZCHIP //
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    reg_wizchip_cs_cbfunc(0, 0);
    reg_wizchip_spi_cbfunc(0, 0);
    reg_wizchip_spiburst_cbfunc(0, 0);
    reg_wizchip_spiburst2_cbfunc(SpiRead, SpiWrite);

    ////////////////////////////////////////////////////////////////////////

    // testSpi();
    testW5500Version();

    printf("WIZCHIP SOCKET Buffer initialize\n");
    /* WIZCHIP SOCKET Buffer initialize */
    if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1) {
        printf("WIZCHIP Initialized fail.\r\n");
        while (1)
            ;
    }

    printf("PHY link status check\n");
    /* PHY link status check */
    int cnt = 0;
    do {
        //  int x = ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);
        //  if(x == -1)
        if (ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1)
            printf("Unknown PHY Link stauts.\r\n");
    } while (tmp == PHY_LINK_OFF);

    printf("CW_INIT_WIZCHIP_3\n");

    /* Network initialization */
    network_init();

    /*******************************/
    /* WIZnet W5500 Code Examples  */
    /* TCPS/UDPS Loopback test     */
    /*******************************/
    /* Main loop */
    while (1) {
        /* Loopback Test */
        // TCP server loopback test
        // if( (ret = loopback_tcps(SOCK_TCPS, gDATABUF, 5000)) < 0) {
        //  printf("SOCKET ERROR : %ld\r\n", ret);
        // }

        // UDP server loopback test
        if ((ret = loopback_udps(SOCK_UDPS, gDATABUF, 3000)) < 0) {
            printf("SOCKET ERROR : %ld\r\n", ret);
        }
        // usleep(1000);
    } // end of Main loop

} // end of main()

int32_t loopback_udps(uint8_t sn, uint8_t *buf, uint16_t port) {
    int32_t ret;
    uint16_t size, sentsize;
    uint8_t destip[4];
    uint16_t destport;
    // uint8_t  packinfo = 0;
    int stat = getSn_SR(sn);
    // printf("%d:Sock loopback Status=0x%x\r\n",sn, stat);
    // printf("Compare 0x22 vs 0x%x: %d\r\n", stat, stat == SOCK_UDP);
    // switch(getSn_SR(sn))
    switch (stat) {
    case SOCK_UDP:
        if ((size = getSn_RX_RSR(sn)) > 0) {
            if (size > DATA_BUF_SIZE)
                size = DATA_BUF_SIZE;
            ret = recvfrom(sn, buf, size, destip, (uint16_t *)&destport);
            if (ret <= 0) {
                printf("%d: recvfrom error. %ld\r\n", sn, ret);
                return ret;
            }else{
                printf("%d: recv %d bytes.\r\n", sn, ret);
                for (int i=0; i<ret; i++){
                    if (i % 50 == 0) printf("\n");
                    if (buf[i] < ' ') printf("\\%02X", buf[i]);
                    else printf("%c", buf[i]);
                }
                printf("\n");
            }
            size = (uint16_t)ret;
            sentsize = 0;
            while (sentsize != size) {
                destport = 5005;
                ret = sendto(sn, buf + sentsize, size - sentsize, destip,
                             destport);
                if (ret < 0) {
                    printf("%d: sendto error. %ld\r\n", sn, ret);
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
        }
        break;
    case SOCK_CLOSED:
        printf("%d:LBUStart\r\n", sn);
        ret = socket(sn, Sn_MR_UDP, port, 0x00);
        if (ret != sn)
            printf("%d:Error opening port. ret=%d\r\n", sn, ret);
        return ret;
        printf("%d:Opened, port [%d]\r\n", sn, port);
        break;
    default:
        break;
    }
    return 1;
}
