

//--------------------------------------------------------
// myOledMonitor.cpp
//--------------------------------------------------------
// Normalizes rotations to a horizontal orientation

#include "myOledMonitor.h"
// #include <myDebug.h>
//	   client must include myDebug.h because display() define
//	   conflicts with SSD1306 library, so we call display_fxn directly herein
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_SSD1306.h>


#define DEBUG_SCREEN  0

#define SSD1306_I2C_ADDR    0x3C    // works on all displays that I own; sample prog has "0x3D for 128x64"

#define GFX_CHAR_WIDTH     	6       // at size 1, 5x8 + 1 space = 21.3 characters
#define GFX_CHAR_HEIGHT    	8

#define COLOR_BLACK			0
#define ST7789_WHITE		0xffff
#define ST1306_WHITE		1

//---------------------------------
// Pins Used
//---------------------------------
// Comments are reminders of hardwired pins on Ideaspark modules
//
// #define ESP32_SDL		22
// #define ESP32_SDA		22
//
// #define ESP32_MOSI 		23
// #define ESP32_SCLK 		18
// #define ST7789_BLK   	32		// hardwired on ideaspark module, not used in API

#define ESP32_CS   		15		// hardwired to use default ESP32 CS
#define ST7789_DC    	2		// hardwired on ideaspark module
#define ST7789_RST   	4		// hardwired on ideaspark module

Adafruit_ST7789 *st7789;
Adafruit_SSD1306 *ssd1306;
Adafruit_GFX *oled;

static uint16_t COLOR_WHITE;

static uint16_t g_driver;
static int g_font_size;
static int g_with_task;
static int g_with_display;

static int screen_width;
static int screen_height;
static int char_width;
static int char_height;
static int oled_cols;
static int oled_rows;
static char *screen_buf;

static volatile int head;
static volatile int tail;
static volatile int print_counter;
static volatile bool in_print;


// forward, extern, and convenience declarations

static void update();
static void updateTask(void *param);
extern void display_fxn(const char *alt_color, int level, const char *format, ...);

#define buf_row(r)		(&screen_buf[(r) * (oled_cols+1)])


//--------------------------------------
// implementation
//--------------------------------------

myOledMonitor::myOledMonitor(uint16_t driver, int font_size/*=2*/, bool with_task/*=false*/)
{
	g_driver = driver;
	g_font_size = font_size;
	g_with_task = with_task;
	char_width = font_size * GFX_CHAR_WIDTH;
	char_height = font_size * GFX_CHAR_HEIGHT;

	// native screen sizes in rotation(0)
	
	if (g_driver == DRIVER_ST7789_320x170)
	{
		screen_width = 170;
		screen_height = 320;
		COLOR_WHITE = ST7789_WHITE;
	}
	else
	{
		screen_width = 128;
		COLOR_WHITE = ST1306_WHITE;
		if (g_driver == DRIVER_SSD1306_128x64)
			screen_height = 64;
		else
			screen_height = 32;
	}
}


// static
void myOledMonitor::init(int rotation/*=0*/, bool with_display/*=false*/)
{
	#if DEBUG_SCREEN
		display_fxn(0,0,"myOledMonitor::init(%d,%d) driver(0x%04x) font(%d) task(%d) screen(%d,%d) char(%d,%d)",
			rotation,with_display,
			g_driver,
			g_font_size,
			g_with_task,
			screen_width,screen_height,
			char_width,char_height);
	#endif

	g_with_display = with_display;

	// initialize with native constants
	// then use logical constants

	if (g_driver == DRIVER_ST7789_320x170)
	{
		display_fxn(0,0,"initing 7789",0);
		st7789 = new Adafruit_ST7789(ESP32_CS, ST7789_DC, ST7789_RST);
		st7789->init(screen_width,screen_height);
		oled = st7789;
	}
	else
	{
		ssd1306 = new Adafruit_SSD1306(screen_width, screen_height);
		ssd1306->begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDR);
		// ssd1306->display(); 	// show built in Adafruit splash screen
		// delay(1000);
		ssd1306->clearDisplay();
		oled = ssd1306;
	}

	// switch logical height and width by rotation
	// and set logical number of rows and columns

	if (rotation & 1)
	{
		int temp = screen_width;
		screen_width = screen_height;
		screen_height = temp;
	}

	oled_cols = screen_width/char_width;
	oled_rows = screen_height/char_height;

	#if DEBUG_SCREEN
		display_fxn(0,0,"oled_rows(%d) oled_cols(%d)",oled_rows,oled_cols);
	#endif
	// initialize the screen buffer

	screen_buf = new char[(oled_rows+1) * (oled_cols+1)];
	memset(screen_buf,0,(oled_rows+1) * (oled_cols+1));

	// set the rotation

	oled->setRotation(rotation);
		// ST7789:
		// 		0 = top = usb connector
		// 		1 = usb connector to left
		// 		2 = bottom = usb connector
		// 		3 = usb connector to right
		// SSD1306
		//		0 = usb right, yellow at top
		//      1 = usb top, yellow at right
		//		2 = usb left, yellow at bottom
		//		3 = usb bottom, yellow at left

	// clear the screen and go ...

	oled->fillScreen(COLOR_BLACK);
	oled->setTextSize(g_font_size);
	oled->setTextColor(COLOR_WHITE,COLOR_BLACK);
	oled->setTextWrap(false);
	
	if (g_driver & DRIVER_MASK_SSD1306)
		ssd1306->display();

	if (g_with_task)
	{
        #define ESP32_CORE_ARDUINO 	1
        #define ESP32_CORE_OTHER  	0

        // must run on ESP32_CORE_ARDUINO

        #if DEBUG_SCREEN
			display_fxn(0,0,"creating myOledMonitor::monUpdateTask pinned to core %d",ESP32_CORE_ARDUINO);
	    #endif

		xTaskCreatePinnedToCore(
			updateTask,
			"monUpdateTask",
			4096,	// stack
			NULL,	// param
			5,  	// priority
			NULL,   // handle
			ESP32_CORE_ARDUINO);
	}
}



void updateTask(void *param)
{
    delay(1000);
	#if DEBUG_SCREEN
		display_fxn(0,0,"starting myOledMonitor::monUpdateTask() on core(%d)",xPortGetCoreID());
	#endif
	
    while (1)
    {
        vTaskDelay(1);
        update();
    }
}


//------------------------------------------------------
// update and println
//------------------------------------------------------

static void update()
{
	static int last_counter;
	if (last_counter == print_counter)
		return;
	last_counter = print_counter;

	while (in_print) { delay(1); }

	int use_tail = tail;
	int use_head = head;
	int out_row = 0;

	#if DEBUG_SCREEN
		display_fxn(0,0,"update(%d) use_tail(%d) use_head(%d)",last_counter,use_tail,use_head);
	#endif

	if (use_tail > use_head)
	{
		while (use_tail < oled_rows+1)
		{
			#if DEBUG_SCREEN
				display_fxn(0,0,"tail_row(%d) use_tail(%d) s=%s",out_row,use_tail,buf_row(use_tail));
			#endif

			oled->setCursor(0,out_row * char_height);
			oled->print(buf_row(use_tail));
			use_tail++;
			out_row++;
		}
		use_tail = 0;
	}

	while (use_tail < use_head)
	{
		#if DEBUG_SCREEN
			display_fxn(0,0,"head_row(%d) use_tail(%d) s=%s",out_row,use_tail,buf_row(use_tail));
		#endif

		oled->setCursor(0,out_row * char_height);
		oled->print(buf_row(use_tail));
		use_tail++;
		out_row++;
	}

	if (g_driver & DRIVER_MASK_SSD1306)
		ssd1306->display();
}




// static
void myOledMonitor::println(const char *format, ...)
{
	while (in_print) { delay(1); }
	in_print = 1;

	va_list var;
	va_start(var, format);
	char buffer[oled_cols + 1];

	vsnprintf(buffer,oled_cols,format,var);
	int len = strlen(buffer);

	if (g_with_display)
		display_fxn(0,0,"mon(%d): %s",print_counter,buffer);
	print_counter++;

	if (len < oled_cols)
	{
		memset(&buffer[len],' ',oled_cols-len);
		buffer[oled_cols] = 0;
	}

	// put into circular buffer

	strcpy(buf_row(head),buffer);
	head++;
	if (head >= oled_rows+1)
		head = 0;
	if (tail == head)
	{
		tail++;
		if (tail >= oled_rows+1)
			tail = 0;
	}

	in_print = 0;
	print_counter++;

	// call update if no task

	if (!g_with_task)
		update();
}
