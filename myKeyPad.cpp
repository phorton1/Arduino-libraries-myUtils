#include "Arduino.h"
#include "myKeyPad.h"
#include "myDebug.h"

#define DBG_KPD  1

#define LONG_TIME  850
    // milliseconds for a long press

int ROW_PIN[KP_NUM_ROWS] = {3, 4, 5, 6};
int COL_PIN[KP_NUM_COLS] = {7, 8, 9, 10};

#define H_STATE_UP    0
#define H_STATE_DOWN  1
#define H_STATE_SHORT 2
#define H_STATE_LONG  3

        
void keyPadButton::check()
{
    digitalWrite(COL_PIN[col],1);
    uint8_t st = digitalRead(ROW_PIN[row]) ?
        KP_STATE_DOWN : KP_STATE_UP;
    digitalWrite(COL_PIN[col],0);
    if (!(state & st))
    {
        if (st == KP_STATE_DOWN)
        {
            display(DBG_KPD,"button[%d,%d] DOWN",row,col);
            event_time = millis();
            state = KP_STATE_DOWN;
            if (handler[H_STATE_DOWN])
                handler[H_STATE_DOWN](param[H_STATE_DOWN],row,col,state);
        }
        else
        {
            display(DBG_KPD,"button[%d,%d] UP",row,col);
            state |= KP_STATE_UP;
            state &= ~KP_STATE_DOWN;
            event_time = millis();
            
            if (handler[H_STATE_UP])
                handler[H_STATE_UP](param[H_STATE_UP],row,col,state);
            if (handler[H_STATE_SHORT] &&
                !(state & KP_STATE_LONG))
            {
                state |= KP_STATE_SHORT;
                handler[H_STATE_SHORT](param[H_STATE_SHORT],row,col,state);
            }
        }
    }
    else if (handler[H_STATE_LONG] &&
             st == KP_STATE_DOWN &&
             !(state & KP_STATE_LONG) &&
             millis() > event_time + LONG_TIME)
             
    {
        state |= KP_STATE_LONG;
        handler[H_STATE_LONG](param[H_STATE_LONG],row,col,state);
    }
}



void keyPadButton::registerHandler(void *user_param, uint8_t mask, buttonHandler client_handler)
{
    if (client_handler)
    {
        if (mask & KP_STATE_UP)
        {
            param[H_STATE_UP] = user_param;
            handler[H_STATE_UP] = client_handler;
        }
        if (mask & KP_STATE_DOWN)
        {
            param[H_STATE_DOWN] = user_param;
            handler[H_STATE_DOWN] = client_handler;
        }
        if (mask & KP_STATE_SHORT)
        {
            param[H_STATE_SHORT] = user_param;
            handler[H_STATE_SHORT] = client_handler;
        }
        if (mask & KP_STATE_LONG)
        {
            param[H_STATE_LONG] = user_param;
            handler[H_STATE_LONG] = client_handler;
        }
    }
    else
    {
        if (mask & KP_STATE_UP)
        {
            param[H_STATE_UP] = 0;
            handler[H_STATE_UP] = 0;
        }
        if (mask & KP_STATE_DOWN)
        {
            param[H_STATE_DOWN] = 0;
            handler[H_STATE_DOWN] = 0;
        }
        if (mask & KP_STATE_SHORT)
        {
            param[H_STATE_SHORT] = 0;
            handler[H_STATE_SHORT] = 0;
        }
        if (mask & KP_STATE_LONG)
        {
            param[H_STATE_LONG] = 0;
            handler[H_STATE_LONG] = 0;        
        }
    }
}


myKeyPad::myKeyPad()
{
    scan_pos = 0;

    for (int r=0; r<KP_NUM_ROWS; r++)
        for (int c=0; c<KP_NUM_COLS; c++)
            buttons[r * KP_NUM_COLS + c].assign(r,c);
            
    for (int r=0; r<KP_NUM_ROWS; r++)
        pinMode(ROW_PIN[r],INPUT_PULLDOWN);
    for (int c=0; c<KP_NUM_COLS; c++)
        pinMode(COL_PIN[c],OUTPUT);
}


void myKeyPad::task()
{
    buttons[scan_pos].check();
    scan_pos++;
    if (scan_pos >= KP_NUM_ROWS * KP_NUM_COLS)
        scan_pos = 0;
}

