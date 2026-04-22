/*=========================================================
 * Implemented Elements list:
 * LCD: Adafruit #181, 16x2 character LCD, 4-bit mode
 * Button1
 * Button2
 * Force sensor
 *
 * Demo logic:
 * SELECTING:
 *   Button1 -> next recipe
 *   Button2 -> CONFIRMED
 *
 * CONFIRMED:
 *   wait a few seconds, then automatically go to DISPENSING
 *   Button1 -> no action
 *   Button2 -> cancel and return to SELECTING
 *
 * DISPENSING / MIXING / COMPLETE / ERROR:
 *   Button2 -> return to SELECTING
 */
 
#define F_CPU 1000000UL
#define FORCE_SENSOR_THRESHOLD 200
 
#define CONFIRMED_WAIT_MS    2000
#define DISPENSE_DISPLAY_MS  2000
#define MIXING_DISPLAY_MS    2000
 
#define MIXER_PORT PORTB
#define MIXER_DDR  DDRB
#define MIXER_BIT  PB0
 
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
 
#define SOLENOID_COFFEE PC3  // A
#define SOLENOID_MILK   PC4  // B
#define SOLENOID_WATER  PC5  // C
 
/*PIN MAPPING
   ---------------------------------------------------------
   4-bit mode:
   LCD RS -> PD2
   LCD E  -> PD3
   LCD D4 -> PD4
   LCD D5 -> PD5
   LCD D6 -> PD6
   LCD D7 -> PD7
 
   LCD RW -> GND
   LCD VSS -> GND
   LCD VDD -> +5V
   LCD VO  -> potentiometer wiper for contrast
   LCD A/K -> backlight power if used
 
   LED       -> PB1
   ERROR LED -> PB2
   -------------------------------------------------------
   Button1      -> PC1
   Button2      -> PC0
   Force Sensor -> PC2
   Mixing motor -> PB0
 
   Solenoid Coffee -> PC3  (active low)
   Solenoid Milk   -> PC4  (active low)
   Solenoid Water  -> PC5  (active low)
*/
 
#define LCD_RS_PORT PORTD
#define LCD_RS_DDR  DDRD
#define LCD_RS_PIN  PD2
 
#define LCD_E_PORT  PORTD
#define LCD_E_DDR   DDRD
#define LCD_E_PIN   PD3
 
#define LCD_D4_PORT PORTD
#define LCD_D4_DDR  DDRD
#define LCD_D4_PIN  PD4
 
#define LCD_D5_PORT PORTD
#define LCD_D5_DDR  DDRD
#define LCD_D5_PIN  PD5
 
#define LCD_D6_PORT PORTD
#define LCD_D6_DDR  DDRD
#define LCD_D6_PIN  PD6
 
#define LCD_D7_PORT PORTD
#define LCD_D7_DDR  DDRD
#define LCD_D7_PIN  PD7
 
/* Button
   ---------------------------------------------------------
   Hardware logic:
   pull-up to VCC, button press to GND
   released = HIGH
   pressed  = LOW
*/
#define BUTTON1_PORT PORTC
#define BUTTON1_PIN  PINC
#define BUTTON1_DDR  DDRC
#define BUTTON1_BIT  PC1
 
#define BUTTON2_PORT PORTC
#define BUTTON2_PIN  PINC
#define BUTTON2_DDR  DDRC
#define BUTTON2_BIT  PC0
 
#define LED_PORT PORTB
#define LED_DDR  DDRB
#define LED_BIT  PB1
 
#define ERROR_LED_PORT PORTB
#define ERROR_LED_DDR  DDRB
#define ERROR_LED_BIT  PB2
 
/* --------------------------------
   RECIPE / STATE DEFINITIONS
   --------------------------------*/
 
typedef enum
{
    STATE_SELECTING = 0,
    STATE_CONFIRMED,
    STATE_CANCELED,
    STATE_DISPENSING,
    STATE_MIXING,
    STATE_COMPLETE,
    STATE_ERROR
} SystemState;
 
typedef struct
{
    const char *name;   // LCD recipe name
    uint8_t amountA;    // coffee
    uint8_t amountB;    // milk
    uint8_t amountC;    // water
    bool mixing;
} Recipe;
 
/* Ingredient definition:
   A = coffee
   B = milk
   C = water
*/
static const Recipe recipes[] =
{
    { "1-Americano",   2, 0, 1, true  },
    { "2-Latte",       1, 1, 0, true  },
    { "3-LightCoffee", 1, 0, 2, true  },
    { "4-Coffee",      1, 0, 0, false },
    { "5-Milk",        0, 1, 0, false },
    { "6-Water",       0, 0, 1, false }
};
 
#define NUM_RECIPES (sizeof(recipes) / sizeof(recipes[0]))
 
/* -----------------------------------------------------
   GLOBAL VARIABLES
   ----------------------------------------------------- */
 
static volatile uint8_t current_recipe_index = 0;
static volatile SystemState current_state = STATE_SELECTING;
static volatile bool blink_enabled = true;
 
/* -----------------------------------------------------
   LCD LOW-LEVEL FUNCTIONS
   ----------------------------------------------------- */
 
static void lcd_pulse_enable(void)
{
    LCD_E_PORT |= (1 << LCD_E_PIN);
    _delay_us(1);
    LCD_E_PORT &= ~(1 << LCD_E_PIN);
    _delay_us(100);
}
 
static void lcd_write_4bits(uint8_t nibble)
{
    if (nibble & 0x01) LCD_D4_PORT |=  (1 << LCD_D4_PIN);
    else               LCD_D4_PORT &= ~(1 << LCD_D4_PIN);
 
    if (nibble & 0x02) LCD_D5_PORT |=  (1 << LCD_D5_PIN);
    else               LCD_D5_PORT &= ~(1 << LCD_D5_PIN);
 
    if (nibble & 0x04) LCD_D6_PORT |=  (1 << LCD_D6_PIN);
    else               LCD_D6_PORT &= ~(1 << LCD_D6_PIN);
 
    if (nibble & 0x08) LCD_D7_PORT |=  (1 << LCD_D7_PIN);
    else               LCD_D7_PORT &= ~(1 << LCD_D7_PIN);
 
    lcd_pulse_enable();
}
 
static void lcd_send(uint8_t value, bool is_data)
{
    if (is_data) LCD_RS_PORT |=  (1 << LCD_RS_PIN);
    else         LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
 
    lcd_write_4bits((value >> 4) & 0x0F);
    lcd_write_4bits(value & 0x0F);
}
 
static void lcd_command(uint8_t cmd)
{
    lcd_send(cmd, false);
    _delay_ms(2);
}
 
static void lcd_data(uint8_t data)
{
    lcd_send(data, true);
    _delay_us(100);
}
 
static void lcd_init(void)
{
    LCD_RS_DDR |= (1 << LCD_RS_PIN);
    LCD_E_DDR  |= (1 << LCD_E_PIN);
    LCD_D4_DDR |= (1 << LCD_D4_PIN);
    LCD_D5_DDR |= (1 << LCD_D5_PIN);
    LCD_D6_DDR |= (1 << LCD_D6_PIN);
    LCD_D7_DDR |= (1 << LCD_D7_PIN);
 
    _delay_ms(50);
 
    LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
    LCD_E_PORT  &= ~(1 << LCD_E_PIN);
 
    lcd_write_4bits(0x03);
    _delay_ms(5);
 
    lcd_write_4bits(0x03);
    _delay_us(150);
 
    lcd_write_4bits(0x03);
    _delay_us(150);
 
    lcd_write_4bits(0x02);  // 4-bit mode
    _delay_us(150);
 
    lcd_command(0x28);      // 4-bit, 2-line, 5x8 font
    lcd_command(0x0C);      // display on, cursor off, blink off
    lcd_command(0x06);      // entry mode: increment, no shift
    lcd_command(0x01);      // clear display
    _delay_ms(2);
}
 
static void lcd_clear(void)
{
    lcd_command(0x01);
    _delay_ms(2);
}
 
static void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t address = (row == 0) ? (0x00 + col) : (0x40 + col);
    lcd_command(0x80 | address);
}
 
static void lcd_print_padded_16(const char *str)
{
    uint8_t count = 0;
 
    while (*str && count < 16)
    {
        lcd_data((uint8_t)(*str));
        str++;
        count++;
    }
 
    while (count < 16)
    {
        lcd_data(' ');
        count++;
    }
}
 
static void lcd_show_two_lines(const char *line1, const char *line2)
{
    lcd_set_cursor(0, 0);
    lcd_print_padded_16(line1);
 
    lcd_set_cursor(1, 0);
    lcd_print_padded_16(line2);
}
 
/* -----------------------------------------------------
   BUTTON FUNCTIONS
   ----------------------------------------------------- */
 
static bool button1_pressed(void)
{
    static uint8_t last_state = 1;
 
    uint8_t current_state_pin = (BUTTON1_PIN & (1 << BUTTON1_BIT)) ? 1 : 0;
 
    if (last_state == 1 && current_state_pin == 0)
    {
        _delay_ms(20);
        current_state_pin = (BUTTON1_PIN & (1 << BUTTON1_BIT)) ? 1 : 0;
 
        if (current_state_pin == 0)
        {
            last_state = current_state_pin;
            return true;
        }
    }
 
    last_state = current_state_pin;
    return false;
}
 
static bool button2_pressed(void)
{
    static uint8_t last_state = 1;
 
    uint8_t current_state_pin = (BUTTON2_PIN & (1 << BUTTON2_BIT)) ? 1 : 0;
 
    if (last_state == 1 && current_state_pin == 0)
    {
        _delay_ms(20);
        current_state_pin = (BUTTON2_PIN & (1 << BUTTON2_BIT)) ? 1 : 0;
 
        if (current_state_pin == 0)
        {
            last_state = current_state_pin;
            return true;
        }
    }
 
    last_state = current_state_pin;
    return false;
}
 
/* -----------------------------------------------------
   FORCE SENSOR / ADC
   ----------------------------------------------------- */
 
static void adc_init(void)
{
    ADMUX  = 0b01000010;   // AVCC ref, ADC2 (PC2), right adjusted
    ADCSRA = 0b10000111;   // Enable ADC, prescaler = 128
}
 
static uint16_t read_force_sensor(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}
 
static bool cup_exists(void)
{
    return (read_force_sensor() > FORCE_SENSOR_THRESHOLD);
}
 
/* -----------------------------------------------------
   LED / MIXER HELPERS
   ----------------------------------------------------- */
 
static void led_on(void)
{
    LED_PORT |= (1 << LED_BIT);
}
 
static void led_off(void)
{
    LED_PORT &= ~(1 << LED_BIT);
}
 
static void error_led_on(void)
{
    ERROR_LED_PORT |= (1 << ERROR_LED_BIT);
}
 
static void error_led_off(void)
{
    ERROR_LED_PORT &= ~(1 << ERROR_LED_BIT);
}
 
static void mixer_on(void)
{
    MIXER_PORT |= (1 << MIXER_BIT);
}
 
static void mixer_off(void)
{
    MIXER_PORT &= ~(1 << MIXER_BIT);
}
 
/* -----------------------------------------------------
   SOLENOID HELPERS  (active low: LOW = ON, HIGH = OFF)
   ----------------------------------------------------- */
 
static void solenoids_all_off(void)
{
    PORTC |= (1 << SOLENOID_COFFEE) |
             (1 << SOLENOID_MILK)   |
             (1 << SOLENOID_WATER);
}
 
/* -----------------------------------------------------
   DISPENSER / MIXER CONTROL
   ----------------------------------------------------- */
 
static void read_recipe_and_dispense(const Recipe *recipe)
{
    // --- DISPENSE COFFEE (A) ---
    for (uint8_t i = 0; i < recipe->amountA; i++)
    {
        if (!cup_exists()) { current_state = STATE_ERROR; return; }
 
        PORTC &= ~(1 << SOLENOID_COFFEE);  // ON  (active low)
        _delay_ms(500);
        PORTC |=  (1 << SOLENOID_COFFEE);  // OFF (active low)
        _delay_ms(200);
    }
 
    // --- DISPENSE MILK (B) ---
    for (uint8_t i = 0; i < recipe->amountB; i++)
    {
        if (!cup_exists()) { current_state = STATE_ERROR; return; }
 
        PORTC &= ~(1 << SOLENOID_MILK);    // ON
        _delay_ms(500);
        PORTC |=  (1 << SOLENOID_MILK);    // OFF
        _delay_ms(200);
    }
 
    // --- DISPENSE WATER (C) ---
    for (uint8_t i = 0; i < recipe->amountC; i++)
    {
        if (!cup_exists()) { current_state = STATE_ERROR; return; }
 
        PORTC &= ~(1 << SOLENOID_WATER);   // ON
        _delay_ms(500);
        PORTC |=  (1 << SOLENOID_WATER);   // OFF
        _delay_ms(200);
    }
}
 
static void do_mixing(const Recipe *recipe)
{
    if (!recipe->mixing)
    {
        return;
    }
 
    if (!cup_exists())
    {
        mixer_off();
        current_state = STATE_ERROR;
        return;
    }
 
    mixer_on();
 
    // Mix for about 3 seconds, keep checking cup sensor
    for (uint8_t i = 0; i < 30; i++)
    {
        _delay_ms(100);
 
        if (!cup_exists())
        {
            mixer_off();
            current_state = STATE_ERROR;
            return;
        }
    }
 
    mixer_off();
    led_on();
}
 
static void turn_off_all_dispensors_and_mixer(void)
{
    mixer_off();
    led_off();
    solenoids_all_off();  // ensures all solenoids are OFF in error state
}
 
/* -----------------------------------------------------
   LCD STATE DISPLAY FUNCTIONS
   ----------------------------------------------------- */
 
static void lcd_show_selecting(uint8_t recipe_index, bool visible)
{
    if (visible)
    {
        lcd_show_two_lines("SELECTING:", recipes[recipe_index].name);
    }
    else
    {
        lcd_show_two_lines("SELECTING:", "");
    }
}
 
static void lcd_show_confirmed(void)
{
    lcd_show_two_lines("CONFIRMED", "Starting...");
}
 
static void lcd_show_canceled(void)
{
    lcd_show_two_lines("CANCELED", "");
}
 
static void lcd_show_dispensing(void)
{
    lcd_show_two_lines("DISPENSING...", "");
}
 
static void lcd_show_mixing(void)
{
    lcd_show_two_lines("MIXING...", "");
}
 
static void lcd_show_complete(void)
{
    lcd_show_two_lines("COMPLETE", "B2 to reset");
}
 
static void lcd_show_error(bool visible)
{
    if (visible)
    {
        lcd_show_two_lines("ERROR", "B2 to reset");
    }
    else
    {
        lcd_show_two_lines("", "");
    }
}
 
/* -----------------------------------------------------
   MAIN
   ----------------------------------------------------- */
 
int main(void)
{
    lcd_init();
    adc_init();
 
    // Mixer motor control pin
    MIXER_DDR  |= (1 << MIXER_BIT);
    MIXER_PORT &= ~(1 << MIXER_BIT);   // motor OFF
 
    // Button1 input + pull-up
    BUTTON1_DDR  &= ~(1 << BUTTON1_BIT);
    BUTTON1_PORT |=  (1 << BUTTON1_BIT);
 
    // Button2 input + pull-up
    BUTTON2_DDR  &= ~(1 << BUTTON2_BIT);
    BUTTON2_PORT |=  (1 << BUTTON2_BIT);
 
    // LED pins as output, both OFF
    LED_DDR       |= (1 << LED_BIT);
    LED_PORT      &= ~(1 << LED_BIT);
    ERROR_LED_DDR |= (1 << ERROR_LED_BIT);
    ERROR_LED_PORT &= ~(1 << ERROR_LED_BIT);
 
    // Solenoid pins as output, all OFF
    // Active low: HIGH = OFF, LOW = ON
    DDRC  |= (1 << SOLENOID_COFFEE) | (1 << SOLENOID_MILK) | (1 << SOLENOID_WATER);
    PORTC |= (1 << SOLENOID_COFFEE) | (1 << SOLENOID_MILK) | (1 << SOLENOID_WATER);
 
    // Initial state
    current_recipe_index = 0;
    current_state        = STATE_SELECTING;
    blink_enabled        = true;
 
    bool blink_visible = true;
 
    while (1)
    {
        /* Read buttons */
        bool b1 = button1_pressed();
        bool b2 = button2_pressed();
 
        /* Update LCD based on current state */
        switch (current_state)
        {
            case STATE_SELECTING:
                lcd_show_selecting(current_recipe_index, blink_visible);
                break;
 
            case STATE_CONFIRMED:
                lcd_show_confirmed();
                break;
 
            case STATE_CANCELED:
                lcd_show_canceled();
                break;
 
            case STATE_DISPENSING:
                lcd_show_dispensing();
                break;
 
            case STATE_MIXING:
                lcd_show_mixing();
                break;
 
            case STATE_COMPLETE:
                lcd_show_complete();
                break;
 
            case STATE_ERROR:
                lcd_show_error(blink_visible);
                break;
 
            default:
                lcd_clear();
                break;
        }
 
        /* State logic */
        switch (current_state)
        {
            case STATE_SELECTING:
                led_off();
                error_led_off();
 
                if (blink_enabled)
                {
                    blink_visible = !blink_visible;
                    _delay_ms(300);
                }
 
                if (b1)
                {
                    current_recipe_index++;
                    if (current_recipe_index >= NUM_RECIPES)
                    {
                        current_recipe_index = 0;
                    }
                }
 
                if (b2)
                {
                    current_state = STATE_CONFIRMED;
                }
                break;
 
            case STATE_CONFIRMED:
                if (b2)
                {
                    current_state = STATE_SELECTING;
                }
                else
                {
                    _delay_ms(CONFIRMED_WAIT_MS);
 
                    if (button2_pressed())
                    {
                        current_state = STATE_SELECTING;
                    }
                    else if (!cup_exists())
                    {
                        current_state = STATE_ERROR;
                    }
                    else
                    {
                        current_state = STATE_DISPENSING;
                    }
                }
                break;
 
            case STATE_CANCELED:
                current_state = STATE_SELECTING;
                break;
 
            case STATE_DISPENSING:
                if (b2)
                {
                    current_state = STATE_SELECTING;
                }
                else if (!cup_exists())
                {
                    current_state = STATE_ERROR;
                }
                else
                {
                    _delay_ms(DISPENSE_DISPLAY_MS);
 
                    if (button2_pressed())
                    {
                        current_state = STATE_SELECTING;
                    }
                    else
                    {
                        read_recipe_and_dispense(&recipes[current_recipe_index]);
 
                        if (current_state != STATE_ERROR)
                        {
                            if (recipes[current_recipe_index].mixing)
                            {
                                current_state = STATE_MIXING;
                            }
                            else
                            {
                                current_state = STATE_COMPLETE;
                            }
                        }
                    }
                }
                break;
 
            case STATE_MIXING:
                if (b2)
                {
                    current_state = STATE_SELECTING;
                }
                else if (!cup_exists())
                {
                    current_state = STATE_ERROR;
                }
                else
                {
                    _delay_ms(MIXING_DISPLAY_MS);
 
                    if (button2_pressed())
                    {
                        current_state = STATE_SELECTING;
                    }
                    else
                    {
                        do_mixing(&recipes[current_recipe_index]);
 
                        if (current_state != STATE_ERROR)
                        {
                            current_state = STATE_COMPLETE;
                        }
                    }
                }
                break;  // <-- FIX: break was previously inside the else block
 
            case STATE_COMPLETE:
                if (b2)
                {
                    current_state = STATE_SELECTING;
                }
                _delay_ms(100);
                break;
 
            case STATE_ERROR:
                turn_off_all_dispensors_and_mixer();
 
                if (blink_enabled)
                {
                    blink_visible = !blink_visible;
 
                    if (blink_visible)
                    {
                        error_led_on();
                    }
                    else
                    {
                        error_led_off();
                    }
 
                    _delay_ms(300);
                }
 
                if (b2)
                {
                    current_state = STATE_SELECTING;
                }
                break;
 
            default:
                current_state = STATE_SELECTING;
                break;
        }
    }
}