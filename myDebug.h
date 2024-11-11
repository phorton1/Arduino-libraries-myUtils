#ifndef MYDEBUGH
#define MYDEBUGH

// Client initializes the serial port(s) themselves

#include "Arduino.h"

#ifndef WITH_INDENTS
	#define WITH_INDENTS        1
#endif
#ifndef WITH_INDENT_CHECKS
	#define WITH_INDENT_CHECKS  0
#endif
#ifndef WITH_DISPLAY
	#define WITH_DISPLAY        1
#endif
#ifndef WITH_WARNINGS
	#define WITH_WARNINGS       1
#endif
#ifndef WITH_ERRORS
	#define WITH_ERRORS         1
#endif
#ifndef WITH_DISPLAY_BYTES
    #define WITH_DISPLAY_BYTES       1
#endif
#ifndef WITH_DISPLAY_BYTES_LONG
    #define WITH_DISPLAY_BYTES_LONG  1
#endif


#define COLOR_CONST_DEFAULT  0
#define COLOR_CONST_WARNING  1
#define COLOR_CONST_ERROR    2


#if defined(CORE_TEENSY) || defined(ESP32)
    #define USE_PROGMEM         0
    #define USE_MEMORY_CHECK	0
	extern void setColorString(int what, const char *str);
#else
    #define USE_PROGMEM         1
    #define USE_MEMORY_CHECK	0
	#define setColorString(w,s)

	extern const char *floatToStr(float f);
		// print it out with 6 decimal places
		// uses a static buffer, so only one call per display!
#endif


extern int debug_level;
extern int warning_level;
extern Stream *dbgSerial;
extern Stream *extraSerial;


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
        #define display(l,f,...)        	display_fxn(0,l,PSTR(f),__VA_ARGS__)
		#define display_color(c,l,f,...)	display_fxn(P_STR(c),l,PSTR(f),__VA_ARGS__)
    #else
        #define display(l,f,...)        	display_fxn(0,l,f,__VA_ARGS__)
		#define display_color(c,l,f,...)	display_fxn(c,l,f,__VA_ARGS__)
    #endif

    extern void display_fxn(const char *alt_color, int level, const char *format, ...);
    extern void clearDisplay();

	#if defined(CORE_TEENSY) || defined(ESP32)
		extern void display_string(const char *alt_color, int level, const String &str);
			// Exists only on Teensy and ESP32 as I tire of this stuff
			// Allows for creating large Strings that can be explicitly and simply
			// set to dbgSerial and extraSerial
	#endif

	#if WITH_INDENTS
		#if USE_PROGMEM
			#define display_level(d,l,f,...)	{ int save = proc_level; proc_level=l; display_fxn(0,d,PSTR(f),__VA_ARGS__); proc_level=save; }
		#else
			#define display_level(d,l,f,...)	{ int save = proc_level; proc_level=l; display_fxn(0,d,f,__VA_ARGS__); proc_level=save; }
		#endif
	#else
		#if USE_PROGMEM
			#define display_level(d,l,f,...)        display_fxn(0,d,PSTR(f),__VA_ARGS__)
		#else
			#define display_level(d,l,f,...)        display_fxn(0,dl,f,__VA_ARGS__)
		#endif
	#endif

#else
	#define display_color(c,l,f,....)
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
#else
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