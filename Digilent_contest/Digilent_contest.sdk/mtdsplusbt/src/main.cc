

#include "xil_cache.h"
#include "xparameters.h"
#include "PmodBT2.h"

//required definitions for sending & receiving data over the host board's UART port
#ifdef __MICROBLAZE__
#include "xuartlite.h"
typedef XUartLite SysUart;
#define SysUart_Send            XUartLite_Send
#define SysUart_Recv            XUartLite_Recv
#define SYS_UART_DEVICE_ID      XPAR_AXI_UARTLITE_0_DEVICE_ID
#define BT2_UART_AXI_CLOCK_FREQ XPAR_CPU_M_AXI_DP_FREQ_HZ
#else
#include "xuartps.h"
typedef XUartPs SysUart;
#define SysUart_Send            XUartPs_Send
#define SysUart_Recv            XUartPs_Recv
#define SYS_UART_DEVICE_ID      XPAR_PS7_UART_1_DEVICE_ID
#define BT2_UART_AXI_CLOCK_FREQ 100000000
#endif

PmodBT2 myDevice;
SysUart myUart;

extern void setup();
extern void loop();


void DemoInitialize();
void DemoRun();
void SysUartInit();
void EnableCaches();
void DisableCaches();

int main()
{
	EnableCaches();
	setup();
    DemoInitialize();
    DemoRun();
    DisableCaches();
    return XST_SUCCESS;
}

void DemoInitialize()
{
    //EnableCaches();
    SysUartInit();
    BT2_Begin (
        &myDevice,
        XPAR_PMODBT2_0_AXI_LITE_GPIO_BASEADDR,
        XPAR_PMODBT2_0_AXI_LITE_UART_BASEADDR,
        BT2_UART_AXI_CLOCK_FREQ,
        115200
    );
}

void DemoRun()
{
    u8 buf[1];
    int n;

    xil_printf("Initialized PmodBT2, received data will be echoed here, type to send data\r\n");

    while(1) {
    	loop();
        //echo all characters received from both BT2 and terminal to terminal
        //forward all characters received from terminal to BT2
        n = SysUart_Recv(&myUart, buf, 1);
        if (n != 0) {
            SysUart_Send(&myUart, buf, 1);
            BT2_SendData(&myDevice, buf, 1);
        }

        n = BT2_RecvData(&myDevice, buf, 1);
        if (n != 0) {
            SysUart_Send(&myUart, buf, 1);
        }
    }
}

//initialize the system uart device, AXI uartlite for microblaze, uartps for Zynq
void SysUartInit()
{
#ifdef __MICROBLAZE__
    XUartLite_Initialize(&myUart, SYS_UART_DEVICE_ID);
#else
    XUartPs_Config *myUartCfgPtr;
    myUartCfgPtr = XUartPs_LookupConfig(SYS_UART_DEVICE_ID);
    XUartPs_CfgInitialize(&myUart, myUartCfgPtr, myUartCfgPtr->BaseAddress);
#endif
}

void EnableCaches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
#endif
#endif
}

void DisableCaches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheDisable();
#endif
#endif
}
