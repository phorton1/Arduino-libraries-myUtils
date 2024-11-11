//-----------------------------------------------------------
// myESPTelnetStream
//-----------------------------------------------------------
// A derived version of ESPTelnetStream that is intended to
// be more or less non-blocking.
//
// (a) buffers telnet output to a 10K buffer
// (b) uses a task to write bytes to the telnet streem every 200ms
//
// This appears to result in better performance over TCP/IP than
// writing a full packet for every print() or println() call.

#pragma once

#ifdef ESP32

#include <ESPTelnetStream.h>

#define WITH_MY_ESP_TELNET_TASK	 1


class myESPTelnetStream : public ESPTelnetStream
{
public:

	void init();
		// because telnet.begin() is not virtual

private:
	
	size_t write(uint8_t) override;

	#if WITH_MY_ESP_TELNET_TASK
		static void telnetTask(void *param);
	#endif
  
	void flushOutput();
	
};

#endif

