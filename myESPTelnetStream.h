//-----------------------------------------------------------
// myESPTelnetStream
//-----------------------------------------------------------
// A derived version of ESPTelnetStream that is intended to
// be more or less non-blocking.
//
// (a) buffers telnet output to a 10K buffer
// (b) requies client to call loop() and flushOutput()
// (c_ best if loop() and flushOutput() are called from a task
//
// This appears to result in better performance over TCP/IP than
// writing a full packet for every print() or println() call.

#pragma once

#ifdef ESP32

#include <ESPTelnetStream.h>


class myESPTelnetStream : public ESPTelnetStream
{
public:

	void flushOutput();

	static uint32_t m_num_missed;		// writes while disconnected
	static uint32_t m_num_error;		// flush errors
	static uint32_t m_num_warning;		// flush warnings
	static uint32_t m_num_overflow;		// overflows in write
	static uint32_t m_num_wait;			// wait loops for flushOutput from write

private:
	
	size_t write(uint8_t) override;
	
};


#endif

