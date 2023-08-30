#ifndef MYDEBUGH
#define MYDEBUGH

// Client initializes the serial port themselves

#include "Arduino.h"
// #include <stdint.h>


#define WITH_INDENTS        1
#define WITH_INDENT_CHECKS  0
#define WITH_DISPLAY        1
#define WITH_WARNINGS       1
#define WITH_ERRORS         1
#define USE_MEMORY_CHECK    0

#if defined(CORE_TEENSY) || defined(ESP32)
    #define USE_PROGMEM         0
#else
    #define USE_PROGMEM         1
#endif

// #include <SoftwareSerial.h> and change below if you want
// Change below on Teensy to Serial1 if you want

#if defined(CORE_TEENSY) || defined(ESP32)
    #undef USE_MEMORY_CHECK
    extern Stream *dbgSerial;
#else
    #define dbgSerial     (&Serial)
#endif

extern Stream *extraSerial;


#if WITH_DISPLAY
    // you can turn these on or off individually
    // but they default to following WITH_DISPLAY
    #define WITH_DISPLAY_BYTES       1
    #define WITH_DISPLAY_BYTES_EP    1
    #define WITH_DISPLAY_BYTES_LONG  1
#else
    #define WITH_DISPLAY_BYTES       0
    #define WITH_DISPLAY_BYTES_EP    0
    #define WITH_DISPLAY_BYTES_LONG  0
#endif





//---------------------------------------------------
// turn major chunks of code of or on
//---------------------------------------------------

#if WITH_INDENTS
	extern int proc_level;
	#if WITH_INDENT_CHECKS
		#define proc_entry()	proc_level++;  int local_pl=proc_level
		#define proc_leave()	if (local_pl != proc_level) dbgSerial->print("<<<<<<<<"); proc_level--
	#else
		#define proc_entry()    proc_level++
		#define proc_leave()    proc_level--
    #endif
#else
    #define proc_entry()
    #define proc_leave()
#endif

#if WITH_DISPLAY
    #if USE_PROGMEM
        #define display(l,f,...)        display_fxn(l,PSTR(f),__VA_ARGS__)
    #else
        #define display(l,f,...)        display_fxn(l,f,__VA_ARGS__)
    #endif
    extern void display_fxn(int level, const char *format, ...);
    extern void clearDisplay();
    extern void inhibitCr();

	#if WITH_INDENTS
		#if USE_PROGMEM
			#define display_level(d,l,f,...)	{ int save = proc_level; proc_level=l; display_fxn(d,PSTR(f),__VA_ARGS__); proc_level=save; }
		#else
			#define display_level(d,l,f,...)	{ int save = proc_level; proc_level=l; display_fxn(d,f,__VA_ARGS__); proc_level=save; }
		#endif
	#else
		#if USE_PROGMEM
			#define display_level(d,l,f,...)        display_fxn(d,PSTR(f),__VA_ARGS__)
		#else
			#define display_level(d,l,f,...)        display_fxn(dl,f,__VA_ARGS__)
		#endif
	#endif

#else
	#define display_level(d,l,f,...)
    #define display(l,f,...)
    #define clearDisplay()
#endif


#if WITH_WARNINGS
    #if USE_PROGMEM
        #define warning(l,f,...)        warning_fxn(l,PSTR(f),__VA_ARGS__)
    #else
        #define warning(l,f,...)        warning_fxn(l,f,__VA_ARGS__)
    #endif
    extern void warning_fxn(int level, const char *format, ...);
#else
    #define warning(l,f,...)
#endif


#if WITH_ERRORS
    #if USE_PROGMEM
        #define my_error(f,...)        error_fxn(PSTR(f),__VA_ARGS__)
    #else
        #define my_error(f,...)        error_fxn(f,__VA_ARGS__)
    #endif
    extern void error_fxn(const char *format, ...);
#else
    #define my_error(f,...)
#endif



#if WITH_DISPLAY_BYTES
    extern void display_bytes(int level, const char *label, const uint8_t *buf, int len);
    #if defined(CORE_TEENSY) || defined(ESP32)
        // display_bytes_ep() only available on teensy
        extern void display_bytes_ep(int level, const uint8_t ep, const char *label, const uint8_t *buf, int len);
    #endif
#else
    #if defined(CORE_TEENSY) || defined(ESP32)
        // and we enforce it by not defining a null method otherwise
        #define display_bytes_ep(l,a,b,x,z)
    #endif
    #define display_bytes(l,a,b,z)
#endif


#if WITH_DISPLAY_BYTES_LONG
    extern void display_bytes_long(int level, uint16_t addr, const uint8_t *buf, int len, Stream *use_stream=0);
#else
    #define display_bytes_long(l,a,b,z)
    #define display_bytes_long(l,a,b,z,s)
#endif


// Extern buffers for public use

#if defined(CORE_TEENSY) || defined(ESP32)
    #define DISPLAY_BUFFER_SIZE     255
#else
    #define DISPLAY_BUFFER_SIZE     80
#endif


#if WITH_DISPLAY || WITH_WARNINGS || WITH_ERRORS
    extern char display_buffer1[DISPLAY_BUFFER_SIZE];
    #if USE_PROGMEM
        extern char display_buffer2[DISPLAY_BUFFER_SIZE];
    #endif
#endif


//---------------------------------------------------
// these are always defined
//---------------------------------------------------

extern int debug_level;
extern int warning_level;

#ifndef ESP32
    // defined on Arduino/Teensy even if MY_DEBUG is turned off
    extern uint8_t myButtonPressed(uint8_t pin, uint8_t *state);
#endif


//----------------------------------------
// memory checking
//----------------------------------------

#if USE_MEMORY_CHECK
    extern "C" {

        #define MEMORY_LIMIT_WARNING 256

        void checkMem();
            // call this at the top of a routine and if there is less than
            // MEMORY_LIMIT_WARNING of bytes of free memory, it will print
            // a warning to the dbgSerial port.  Note that this is called
            // in all debug routines, so merely calling display() in your
            // routine *should* roughly check the memory situation

        void dbgMem();
            // call this and it will print out the amount of free memory
            // to the dbgSerial port
    }
#else
    #define checkMem()
    #define dbgMem()
#endif


#endif  // !MYDEBUGH