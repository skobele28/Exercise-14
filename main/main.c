#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#define LOOP_DELAY_MS           10      // Loop sampling time (ms)
#define DEBOUNCE_TIME           40      // Debounce time (ms)
#define NROWS                   4       // Number of keypad rows
#define NCOLS                   4       // Number of keypad columns

#define ACTIVE                  0       // Keypad active state (0 = low, 1 = high)

#define NOPRESS                 '\0'    // NOPRESS character

int row_pins[] = {GPIO_NUM_9, GPIO_NUM_8, GPIO_NUM_18, GPIO_NUM_17};     // Pin numbers for rows
int col_pins[] = {GPIO_NUM_16, GPIO_NUM_15, GPIO_NUM_7, GPIO_NUM_6};   // Pin numbers for columns

char keypad_array[NROWS][NCOLS] = {   // Keypad layout
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// declare function for initializing keypad inputs and outputs
static void init_keypad(void);

// declare function for scanning keypad
static char scan_keypad(void);

void app_main(void) {
    
    init_keypad();
    
    typedef enum {
        WAIT_FOR_PRESS,
        DEBOUNCE,
        WAIT_FOR_RELEASE
    } State_t;
    State_t state;

    while(true){
        char new_key = scan_keypad();
        int time;
        char last_key;
        state = WAIT_FOR_PRESS;  // set initial state
        switch(state){
            case WAIT_FOR_PRESS:
                if(new_key != NOPRESS){
                    time = 0;
                    last_key = new_key;
                    state = DEBOUNCE;
                } else {
                    state = WAIT_FOR_PRESS;
                }
                break;
            case DEBOUNCE:
                bool timed_out = (time == DEBOUNCE_TIME);
                if(!timed_out){
                    vTaskDelay(LOOP_DELAY_MS/portTICK_PERIOD_MS);
                    time += LOOP_DELAY_MS;
                    state = DEBOUNCE;
                }
                else if(timed_out && new_key != last_key){
                    state = WAIT_FOR_PRESS;
                }
                else if(timed_out && new_key == last_key){
                    state = WAIT_FOR_RELEASE;
                }
                break;
            case WAIT_FOR_RELEASE:
                if(new_key != NOPRESS){
                    state = WAIT_FOR_RELEASE;
                }
                else if(new_key == NOPRESS){
                    printf("%c\n", last_key);
                    state = WAIT_FOR_PRESS;
                }
                break;
        }
    }
}

// function to initialize keypad inputs and outputs
void init_keypad(void)
{
    int i;
    for(i = 0; i < NROWS; i++){
        gpio_reset_pin(row_pins[i]);
        gpio_set_direction(row_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(row_pins[i], !ACTIVE);
    }
    
    for(i = 0; i < NCOLS; i++){
    gpio_reset_pin(col_pins[i]);
    gpio_set_direction(col_pins[i], GPIO_MODE_INPUT);
    
        if (ACTIVE == 0){
            gpio_pullup_en(col_pins[i]);
        }
        else{
            gpio_pulldown_en(col_pins[i]);
        }
    }
}

char scan_keypad(void){
    char key_char = NOPRESS;
    for(int row = 0; row < NROWS; row++){
        if (key_char != NOPRESS){break;}
        gpio_set_level(row_pins[row], ACTIVE);
        for(int col = 0; col < NCOLS; col++){
            int level = gpio_get_level(col_pins[col]);
            if(level == ACTIVE){
                gpio_set_level(row_pins[row], !ACTIVE);
                return key_char = keypad_array[row][col];
            }
        }
        gpio_set_level(row_pins[row], !ACTIVE);
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    return key_char;
}
