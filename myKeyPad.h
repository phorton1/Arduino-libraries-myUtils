#ifndef _MYKEYPADH__
#define _MYKEYPADH__

#define KP_NUM_ROWS   4
#define KP_NUM_COLS   4

#define KP_STATE_UP     0x01
#define KP_STATE_DOWN   0x02
#define KP_STATE_SHORT  0x04
#define KP_STATE_LONG   0x08
#define KP_MASK_ALL     0x0f


class myKeyPad;
typedef void (*buttonHandler)(void *param, int row, int col, uint8_t state);


class keyPadButton
{
public:

    void registerHandler(void *param, uint8_t mask, buttonHandler client_handler);

    uint8_t getRow()  { return row; }
    uint8_t getCol()  { return col; }
    uint8_t getState()  { return state; }
    uint8_t getEventTime()  { return event_time; }
    uint8_t getElapsedTime()  { return millis() - event_time; }


private:
friend class myKeyPad;    

    void assign(int r, int c)
    {
        row = r;
        col = c;
    }
    
    keyPadButton()
    {
        row = 0;
        col = 0;
        state = KP_STATE_UP;        
        event_time = 0;
        for (int i=0; i<4; i++)
        {
            param[i] = 0;
            handler[i] = 0;
        }
    }
    
    void check();
    
    uint8_t   row = 0;
    uint8_t   col = 0;
    uint8_t   state = 0;
    uint32_t  event_time = 0;
    
    buttonHandler handler[4];
    void *param[4];
    
};



class myKeyPad
{
public:

    myKeyPad();
    
    void task();
    
    keyPadButton *getButton(int button_num)
        { return &buttons[button_num]; }
    keyPadButton *getButton(int row, int col)
        { return &buttons[row * KP_NUM_COLS + col]; }
    
private:

    uint8_t scan_pos;
    keyPadButton buttons[KP_NUM_ROWS * KP_NUM_COLS]; 
};




#endif // !_MYKEYPADH_