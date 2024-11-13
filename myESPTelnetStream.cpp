//-----------------------------------------------------------
// myESPTelnetStream
//-----------------------------------------------------------
// To do this correctly, client (nmTelnet) we should check for
// STA_DISCONNECTED and use a reconnect timer, and restart the
// Telnet when it reconnects.

// However the immediate errors I was seeing were not because
// a lost STATION connection, but rather a failure in the console
// to write to the socket.
//
// Before implementing the task, I was getting a lot of failures
// with current console timing of about 10 seconds to send 0's for
// heartbeat check. But I noticed that if I called client.stop()
// it would "recovers" (reconnect).
//
// Without this task approach, calling telnet.loop() from the Arduino loop()
// essentially blocks the NMEA processing, but once I implemented the task
// the overall behavior of telnet versus time critial loop() functions
// seems much better.
//
// I still get occasional long delays in the socket client, but nonetheless,
// it appear to work much better than without the task.

#ifdef ESP32

#include "myESPTelnetStream.h"
#include <myDebug.h>

#define TELNET_MS		 300
#define MAX_TELNET_BYTES 10000

static volatile bool connected;
static volatile bool in_flush;
static volatile size_t telnet_bytes;
static uint8_t telnet_buffer[MAX_TELNET_BYTES];
static uint32_t last_telnet_time;

uint32_t myESPTelnetStream::m_num_missed;		// writes while disconnected
uint32_t myESPTelnetStream::m_num_error;		// flush errors
uint32_t myESPTelnetStream::m_num_warning;		// flush warnings
uint32_t myESPTelnetStream::m_num_overflow;		// overflows in write
uint32_t myESPTelnetStream::m_num_wait;			// wait loops for flushOutput from write



size_t myESPTelnetStream::write(uint8_t byte)	// override;
{
	if (!client || !isConnected())
	{
		m_num_missed++;		// writes while disconnected
		telnet_bytes = 0;
		return 0;
	}
	
	if (telnet_bytes >= MAX_TELNET_BYTES)
	{
		m_num_overflow++;		// overflows in write
		Serial.println("myESPTelnetStream flushing buffer");
		flushOutput();

		int counter = 0;
		uint32_t start = millis();
		while (in_flush)
		{
			uint32_t now = millis();
			if (now - start > 1000)
			{
				m_num_wait++;			// wait loops for flushOutput from write
				Serial.print("waiting for flush to complete ");
				Serial.println(counter++);
				start = now;
			}
		}
	}

	telnet_buffer[telnet_bytes++] = byte;
	return 1;
}



void myESPTelnetStream::flushOutput()
{
	if (in_flush)
		return;
	in_flush = 1;

	// Serial.println("flushOutput");
		
	if (!client || !isConnected())
	{
		telnet_bytes = 0;
		in_flush = 0;
		return;
	}

	uint32_t now = millis();
	if (now - last_telnet_time >= TELNET_MS)
	{
		last_telnet_time = now;
		
		// Serial.println("flushOutput");

		if (telnet_bytes)
		{
			// Serial.print("flushOutput bytes=");
			// Serial.println(telnet_bytes);
			
			size_t rslt = client.write(telnet_buffer, telnet_bytes);

			if (rslt == 0 || rslt > telnet_bytes)
			{
				// Bright Red 	    91 	101
				Serial.print("\033[91m");
				Serial.print("myESPTelnetStream error wrote ");
				Serial.print(rslt);
				Serial.print("/");
				Serial.println(telnet_bytes);
				m_num_error++;		// flush errors

				extraSerial = 0;
					// layer violation

				client.stop();
				telnet_bytes = 0;
			}
			else if (rslt != telnet_bytes)
			{
				// Bright Yellow 	93 	103
				Serial.print("\033[93m");
				Serial.print("myESPTelnetStream warning writing ");
				Serial.print(rslt);
				Serial.print("/");
				Serial.println(telnet_bytes);
				m_num_warning++;		// flush warnings				

				#if 1
					size_t move_len = telnet_bytes-rslt;
					memcpy(telnet_buffer,&telnet_buffer[rslt],move_len);
					telnet_bytes = move_len;
				#else
					// It was hanging occastionally ... and this call to client.stop()
					// seemed to fix it, but does not seep needed with the task approach.
					client.stop();
					telnet_bytes = 0;
				#endif
			}
			else
			{
				telnet_bytes = 0;
			}
		}
	}
	in_flush = 0;
}



#endif	// ESP32
