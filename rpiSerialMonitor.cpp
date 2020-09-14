#include <Arduino.h>
#include "myDebug.h"
#include "rpiSerialMonitor.h"

#ifdef CORE_TEENSY


// inputs

#define SENSE_RPI_RUN      11      // A9 sense rpi RUN (REBBOOT) pin, HIGH == rpi has voltage
#define SENSE_RPI_READY    4       // sense rpi GPIO25, HIGH == my program has initialized

// outputs

#define LED_RPI_RUN        15      // show state of RPI_RUN sense
#define LED_RPI_READY      14      // show state of RPI_READY sense
#define PIN_PI_REBOOT      12      // (2 for rpiZero) HIGH==REBOOT
    // PIN_PI_REBOOT brings the rpi RUN line to ground via
    // the base of a transistor, causing the rPi to reboot.

#define IDLE_MS_FOR_KEYPRESS  200




rpiSerialMonitor::rpiSerialMonitor(uint8_t start_command, uint8_t num_commands)
{
    rpi_running = 0;
    rpi_ready = 0;

    m_start_command = start_command;
    m_num_commands = num_commands;
    m_key_pressed = 0;
    m_key_timer = 0;
}


void rpiSerialMonitor::init()
{
    pinMode(PIN_PI_REBOOT,OUTPUT);
    digitalWrite(PIN_PI_REBOOT,0);

    pinMode(SENSE_RPI_RUN,INPUT_PULLDOWN);
    pinMode(SENSE_RPI_READY,INPUT_PULLDOWN);

    pinMode(LED_RPI_RUN,OUTPUT);
    pinMode(LED_RPI_READY,OUTPUT);

    #if 0
        for (int i=0; i<31; i++)
        {
            display(0,"flashing leds %d",i);
            digitalWrite(LED_RPI_RUN,i % 2);
            digitalWrite(LED_RPI_READY,i % 2);
            delay(500);
        }
    #endif

    digitalWrite(LED_RPI_RUN,0);
    digitalWrite(LED_RPI_READY,1);
}


rpiSerialMonitor::~rpiSerialMonitor() {}


void rpiSerialMonitor::rebootPi()
{
    display(0,"rpiSerialMonitor::rebootPi() called",0);
    digitalWrite(LED_RPI_RUN,0);
    digitalWrite(LED_RPI_READY,0);
    digitalWrite(PIN_PI_REBOOT,1);
    // rpi_running = 0;
    // rpi_ready = 0;
    delay(900);
    digitalWrite(PIN_PI_REBOOT,0);
}


uint8_t rpiSerialMonitor::task()
{
    if (digitalRead(SENSE_RPI_RUN) != rpi_running)
    {
        rpi_running = !rpi_running;
        digitalWrite(LED_RPI_RUN,rpi_running);
        display(0,"rpi %s",(rpi_running ? "RUNNING" : "NOT RUNNING"));
    }
    if (digitalRead(SENSE_RPI_READY) != rpi_ready)
    {
        rpi_ready = !rpi_ready;
        digitalWrite(LED_RPI_READY,rpi_ready);
        display(0,"rpi %s",(rpi_ready ? "READY" : "NOT READY"));
    }


    if (Serial.available())
    {
        int c = Serial.read();
        #if 0
            Serial1.write(c);
        #else
            if (m_key_pressed)
            {
                Serial.write(m_key_pressed);
                Serial1.write(c);
                m_key_pressed = c;
                m_key_timer = 0;
            }
            else if (c == 2 &&
                m_key_timer > IDLE_MS_FOR_KEYPRESS)
            {
                m_key_pressed = c;
                m_key_timer = 0;
            }
            else if (m_num_commands &&
                c >= m_start_command &&
                c <= m_start_command + m_num_commands - 1 &&
                m_key_timer > IDLE_MS_FOR_KEYPRESS)
            {
                m_key_pressed = c;
                m_key_timer = 0;
            }
            else
            {
                Serial1.write(c);
                m_key_timer = 0;
            }
        #endif
    }
    #if 1
        else if (m_key_pressed &&
            m_key_timer > IDLE_MS_FOR_KEYPRESS)
        {
            uint32_t retval = m_key_pressed;
            m_key_timer = 0;
            m_key_pressed = 0;
            if (retval == 2)
            {
                rebootPi();
            }
            else
            {
                // retval &= 0xff;
                display(0,"rpiSerialMonitor::task() returning char(%s)",(const char *)&retval);
                return retval;
            }
        }
    #endif


    if (Serial1.available())
    {
        Serial.write(Serial1.read());
    }

    return 0;
}

#endif // CORE_TEENSY
