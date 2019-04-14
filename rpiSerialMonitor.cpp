#include <Arduino.h>
#include "myDebug.h"
#include "rpiSerialMonitor.h"


#define PIN_PI_REBOOT     23      // from teensy to transistor base to reboot rpi
#define PIN_PI_ACTIVE     22      // a led showing inverse of RPIM_REBOOT

#define IDLE_MS_FOR_KEYPRESS  200

rpiSerialMonitor::rpiSerialMonitor(uint8_t start_command, uint8_t num_commands)
{
    m_start_command = start_command;
    m_num_commands = num_commands;
    m_key_pressed = 0;
    m_key_timer = 0;

    pinMode(PIN_PI_REBOOT,OUTPUT);
    pinMode(PIN_PI_ACTIVE,OUTPUT);
    digitalWrite(PIN_PI_REBOOT,0);
    digitalWrite(PIN_PI_ACTIVE,1);
}


rpiSerialMonitor::~rpiSerialMonitor() {}


void rpiSerialMonitor::rebootPi()
{
    display(0,"rpiSerialMonitor::rebootPi() called",0);
    digitalWrite(PIN_PI_ACTIVE,0);
    digitalWrite(PIN_PI_REBOOT,1);
    delay(100);
    digitalWrite(PIN_PI_REBOOT,0);
    digitalWrite(PIN_PI_ACTIVE,1);
}

    
uint8_t rpiSerialMonitor::task()
{
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








