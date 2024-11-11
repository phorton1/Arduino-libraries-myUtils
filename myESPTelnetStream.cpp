//-----------------------------------------------------------
// myESPTelnetStream
//-----------------------------------------------------------
// To do this correctly, client (nmTelnet) we should check for
// STA_DISCONNECTED and use a reconnect timer, and restart the
// Telnet when it reconnects.

// However the immediate errors I am seeing are not because
// a lost STATION connection, but rather a failure in the console
// to write to the socket.  I'm not quite sure what to do about that.
//
// Before implementing the task, I was getting a lot of failures
// with current console timing of about 10 seconds to send 0's for
// heartbeat check. But I noticed that if I called client.stop()
// it would "recovers" (reconnects).
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

volatile bool in_flush;
volatile size_t telnet_bytes;
static uint8_t telnet_buffer[MAX_TELNET_BYTES];
static uint32_t last_telnet_time;


void myESPTelnetStream::init()
{
	begin();	// call base class's non-virtual begin method

    #if WITH_MY_ESP_TELNET_TASK
        // Must run on ESP32_CORE_ARDUINO==1
        // Cannot run on ESP32_CORE_OTHER==0
        // see notes in bilgeAlarm.cpp lcdPrint()
		#define USE_CORE 	1
        display(0,"starting telnetTask pinned to core %d",USE_CORE);
        xTaskCreatePinnedToCore(
            telnetTask,
            "telnetTask",
            4096,	// stack
            this,
            1,  	// priority
            NULL,   // handle
            USE_CORE);		// core
    #endif
}


#if WITH_MY_ESP_TELNET_TASK
	void myESPTelnetStream::telnetTask(void *this_ptr)
	{
		delay(1000);
		display(0,"starting telnetTask loop on core(%d)",xPortGetCoreID());
		myESPTelnetStream *self = (myESPTelnetStream *) this_ptr;
		while (1)
		{
			vTaskDelay(1);

			self->loop();
			#if WITH_MY_ESP_TELNET_TASK
				self->flushOutput();
			#endif
		}
	}
#endif




size_t myESPTelnetStream::write(uint8_t byte)	// override;
{
	if (!client || !isConnected())
	{
		telnet_bytes = 0;
		return 0;
	}
	
	if (telnet_bytes >= MAX_TELNET_BYTES)
	{
		Serial.println("myESPTelnetStream flushing buffer");
		flushOutput();

		int counter = 0;
		uint32_t start = millis();
		while (in_flush)
		{
			uint32_t now = millis();
			if (now - start > 1000)
			{
				Serial.print("waiting for flush to complete ");
				Serial.println(counter++);
				start = now;
			}
		}
	}

	telnet_buffer[telnet_bytes++] = byte;

	#if !WITH_MY_ESP_TELNET_TASK
		flushOutput();
	#endif

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

				// It was hanging occastionally ... and this call to client.stop()
				// seemed to fix it, but does not seep needed with the task approach.

				#if WITH_MY_ESP_TELNET_TASK
					size_t move_len = telnet_bytes-rslt;
					memcpy(telnet_buffer,&telnet_buffer[rslt],move_len);
					telnet_bytes = move_len;
				#else
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
