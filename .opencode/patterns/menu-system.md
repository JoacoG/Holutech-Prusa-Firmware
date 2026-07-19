# Menu System Patterns

Reference: `Firmware/menu.h`, `Firmware/menu.cpp`, `Firmware/ultralcd.cpp`.

## Menu Function Signature
Every menu is a `void` function with no parameters, matching `menu_func_t`:
```cpp
static void lcd_menu_my_feature(void)
{
    MENU_BEGIN();
    MENU_ITEM_BACK_P(_T(MSG_BACK));
    // ... items ...
    MENU_END();
}
```

## Menu Item Macros
All items use `_P` suffix variants for PROGMEM strings. Wrap labels with `_T()` for i18n:
```cpp
MENU_ITEM_BACK_P(_T(MSG_BACK));                        // back/return item
MENU_ITEM_SUBMENU_P(_T(MSG_LABEL), lcd_submenu_func);  // navigate to submenu
MENU_ITEM_FUNCTION_P(_T(MSG_LABEL), callback);         // call function on click
MENU_ITEM_TOGGLE_P(_T(MSG_LABEL), _T(MSG_ON), toggle_func);  // toggle with current state shown
MENU_ITEM_GCODE_P(_T(MSG_LABEL), PSTR("G28"));         // send G-code on click
MENU_ITEM_EDIT_int3_P(_T(MSG_LABEL), &variable, min, max);    // edit int8/int16 value
```

## Static Menu (no state)
Simple list of submenus or actions:
```cpp
static void lcd_menu_settings(void)
{
    MENU_BEGIN();
    MENU_ITEM_BACK_P(_T(MSG_MAIN));
    MENU_ITEM_SUBMENU_P(_T(MSG_TEMPERATURE), lcd_menu_temperature);
    MENU_ITEM_SUBMENU_P(_T(MSG_MOTION), lcd_menu_motion);
    MENU_ITEM_TOGGLE_P(_T(MSG_FAN), fan_on ? _T(MSG_ON) : _T(MSG_OFF), lcd_toggle_fan);
    MENU_END();
}
```

## Stateful Menu (with persistent data)
For menus that need to remember state between calls, use `menu_data` union. Define a struct that fits in `MENU_DATA_SIZE` (32 bytes):
```cpp
typedef struct {
    int16_t edit_value;
    uint8_t cursor_pos;
} menu_my_feature_data_t;

static void lcd_menu_my_feature(void)
{
    menu_my_feature_data_t* data = (menu_my_feature_data_t*)menu_data;
    if (!data->initialized) {
        data->edit_value = eeprom_read_word((uint16_t*)EEPROM_MY_VALUE);
        data->initialized = 1;
    }
    MENU_BEGIN();
    MENU_ITEM_BACK_P(_T(MSG_BACK));
    MENU_ITEM_EDIT_int3_P(_T(MSG_VALUE), &data->edit_value, 0, 100);
    ON_MENU_LEAVE(
        eeprom_update_word((uint16_t*)EEPROM_MY_VALUE, data->edit_value);
    );
    MENU_END();
}
```

## Saving on Exit
Use `ON_MENU_LEAVE()` to persist changes when the user exits the menu:
```cpp
ON_MENU_LEAVE(
    my_save_function();
);
```
Place it immediately after `MENU_BEGIN()`.

## LCD Output for Non-Menu Screens
For info screens (no interactive items), use `lcd_printf_P` directly:
```cpp
static void lcd_menu_status(void)
{
    lcd_timeoutToStatus.stop(); // infinite timeout
    lcd_home();
    lcd_printf_P(PSTR("%S\n"
                      " %-16.16S%-3d\n"),
                 _T(MSG_TITLE),
                 _T(MSG_LABEL), value);
    menu_back_if_clicked();
}
```

## Navigation
- `menu_submenu(func)` — push current menu onto stack and enter submenu.
- `menu_back()` — return to parent menu.
- `menu_back(n)` — return n levels up.
- `menu_goto(func, encoder, reset, feedback)` — switch to arbitrary menu.

## Key Rules
- Always use `_T()` macro for user-visible strings (enables i18n).
- Always use `_P` variants of menu item macros (PROGMEM strings).
- Menu functions should be `static` unless exported in `ultralcd.h`.
- `lcd_encoder` tracks knob position; `lcd_clicked()` detects selection.
- `menu_item_ret()` consumes the click event and prevents further item processing in the current cycle.
