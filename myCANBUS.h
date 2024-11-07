//-------------------------------------------
// myCANBUS.h
//-------------------------------------------
// Defines, Variables, and methods common to CANBUS_Sensor.ino
// and CANBUS_Monitor.ino to allow single source driven by
// two different ino (preference.txt) files for building.
//
// Note that we use 250KBPS bus for compatability with NMEA2000.

#include <myDebug.h>
#include <SPI.h>

#ifndef USE_HSPI
	#define USE_HSPI		0
#endif
#ifndef HOW_CAN_BUS
	#define HOW_CAN_BUS		0
#endif


#define dbg_mon		0


#define HOW_BUS_MPC2515		0		// use github/autowp/arduino-mcp2515 library
#define HOW_BUS_CANBUS		1		// use github/_ttlappalainen/CAN_BUS_Shield mpc2515 library


#define CAN_CS_PIN		5
	// use ESP32 alternative HSPI for mcp2515 so that it
	// isn't mucked with by the st7789 display
	// only currently supported with HOW_BUS_NMEA2000



#if USE_HSPI
	SPIClass hspi(HSPI);
		// MOSI=13
		// MISO=12
		// SCLK=14
		// default CS = 15
#endif



#if HOW_CAN_BUS == HOW_BUS_MPC2515

	#include <mcp2515.h>

	#define WITH_ACK  					0
	#define dbg_ack						1		// only used if WITH_ACK
	#define CAN_ACK_ID 					0x037
	#define MY_TEMPERATURE_CANID		0x036

	// grumble SPIClass passed into in ctor which also calls SPI.begin() if it's null,
	// then also does a pinMode and digitalWrite() to _CS in static initialization,
	// and finally, takes requires a PRIVATE default constant in a PUBLIC api, ffs.

	#if USE_HSPI
		#define MCP2515_DEFAULT_SPI_CLOCK   10000000 // 10MHz
		MCP2515 mcp2515(CAN_CS_PIN,MCP2515_DEFAULT_SPI_CLOCK,&hspi);
	#else
		MCP2515 mcp2515(CAN_CS_PIN);
	#endif

#elif HOW_CAN_BUS == HOW_BUS_CANBUS

	#define SEND_NODE_ID	237
	#include <mcp_can.h>
	MCP_CAN	canbus(CAN_CS_PIN);

#endif



static void initCanBus()
{
	#if USE_HSPI
		hspi.begin();
	#elif 0
		SPI.begin();
	#endif

	#if HOW_CAN_BUS == HOW_BUS_MPC2515

		mcp2515.reset();
		mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);
		mcp2515.setNormalMode();

	#elif HOW_CAN_BUS == HOW_BUS_CANBUS

		#if USE_HSPI
			canbus.setSPI(&hspi);
		#endif

		byte err = canbus.begin(CAN_250KBPS, MCP_8MHz);
		if (err != CAN_OK)
			my_error("canbus.begin() err=%d",err);

	#endif

}

