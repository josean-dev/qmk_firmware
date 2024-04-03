/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include "print.h"
#include "oled/oled_global_variables.h"
#include "oled/render_oleds.c"

enum {
    _BASE,
    _FIRST,
    _SECOND
};

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) { return OLED_ROTATION_270; }

bool oled_task_user(void) {
  current_wpm = get_current_wpm();
  led_usb_state = host_keyboard_led_state();


  if (is_keyboard_master()) {
    render_main_oled();
  } else {
    render_peripheral_oled();
  }

  return false;
}

#endif

enum keycodes {
    HYPR_SPC = MT(MOD_HYPR, KC_SPC),
    MEH_SPC = MT(MOD_LCTL | MOD_LALT | MOD_LSFT, KC_SPC),
    LALT_ENT = MT(MOD_LALT, KC_ENT),
    RSE_LEADER = LT(_SECOND, QK_LEAD)
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_LCTL:
        case KC_RCTL:
            if (record->event.pressed) {
                isSneaking = true;
            } else {
                isSneaking = false;
            }
            break;
        case HYPR_SPC:
            if (record->event.pressed) {
                isJumping  = true;
                showedJump = false;
            } else {
                isJumping = false;
            }
            break;
        case RSE_LEADER:
            if (record->tap.count && record->event.pressed) {
                leader_start();
                return false;        // Return false to ignore further processing of key
            }
            break;
    }

    return true;
}

// Light LEDs 6 to 9 and 12 to 15 red when caps lock is active. Hard to ignore!
const rgblight_segment_t PROGMEM my_capslock_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {6, 1, HSV_RED},       // Light 1 LED, starting with LED 6
    {13, 2, HSV_RED},       // Light 2 LEDs, starting with LED 13
    {33, 1, HSV_RED},       // Light 1 LED, starting with LED 33
    {40, 2, HSV_RED}       // Light 2 LEDs, starting with LED 40
);

// // Light LED 13 in purple when keyboard layer 2 is active
// const rgblight_segment_t PROGMEM my_layer1_layer[] = RGBLIGHT_LAYER_SEGMENTS(
//     {6, 1, HSV_TEAL},       // Light 1 LED, starting with LED 6
//     {13, 2, HSV_TEAL}       // Light 2 LEDs, starting with LED 13
// );
//
// // Light LED 40 in purple when keyboard layer 2 is active
// const rgblight_segment_t PROGMEM my_layer2_layer[] = RGBLIGHT_LAYER_SEGMENTS(
//     {33, 1, HSV_PURPLE},       // Light 1 LED, starting with LED 33
//     {40, 2, HSV_PURPLE}       // Light 2 LEDs, starting with LED 40
// );

// Now define the array of layers. Later layers take precedence
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
    // my_layer1_layer,
    // my_layer2_layer,
    my_capslock_layer
);

void keyboard_post_init_user(void) {
    // Enable the LED layers
    rgblight_layers = my_rgb_layers;
}

bool led_update_user(led_t led_state) {
    rgblight_set_layer_state(0, led_state.caps_lock);
    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    rgblight_set_layer_state(0, layer_state_cmp(state, _FIRST));
    rgblight_set_layer_state(1, layer_state_cmp(state, _SECOND));
    return state;
}

// Tap Dance declarations
enum {
    TD_ESC_CAPS,
    TD_TAB_CUSTOM_FN
};

void custom_fn(tap_dance_state_t *state, void *user_data) {
    switch (state->count) {
        case 1:
            SEND_STRING(SS_TAP(X_TAB));
            break;
        case 2:
            SEND_STRING("Tapped 2 times!");
            break;
        case 3:
            SEND_STRING("Tapped 3 times!");
            break;
        case 4:
            SEND_STRING("Tapped 4 times!");
            break;
    }
    reset_tap_dance(state);
}
// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Escape, twice for Caps Lock
    [TD_ESC_CAPS] = ACTION_TAP_DANCE_DOUBLE(KC_ESC, KC_CAPS),
    [TD_TAB_CUSTOM_FN] = ACTION_TAP_DANCE_FN(custom_fn)
};

void leader_start_user(void) {
    // Do something when the leader key is pressed
}

void leader_end_user(void) {
    if (leader_sequence_two_keys(KC_O, KC_T)) {
        // Leader, o, t => open new chrome tab
        SEND_STRING(SS_LGUI("t"));
    } else if (leader_sequence_two_keys(KC_C, KC_T)) {
        // Leader, c, t => close chrome tab
        SEND_STRING(SS_LGUI("w"));
    } else if (leader_sequence_two_keys(KC_R, KC_T)) {
        // Leader, r, t => restore recently closed chrome tab
        SEND_STRING(SS_LGUI(SS_LSFT("t")));
    } else if (leader_sequence_two_keys(KC_N, KC_T)) {
        // Leader, n, t  => next chrome tab
        SEND_STRING(SS_LGUI(SS_LALT(SS_TAP(X_RIGHT))));
    } else if (leader_sequence_two_keys(KC_P, KC_T)) {
        // Leader, p, t => previous chrome tab
        SEND_STRING(SS_LGUI(SS_LALT(SS_TAP(X_LEFT))));
    } else if (leader_sequence_two_keys(KC_S, KC_S)) {
        // Leader, s, s => screenshot
        SEND_STRING(SS_LGUI(SS_LSFT(SS_TAP(X_4))));
    }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
       TD(TD_TAB_CUSTOM_FN),    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                         KC_Y,    KC_U,    KC_I,    KC_O,   KC_P,  TD(TD_ESC_CAPS),
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_LCTL,    KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                         KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN, LT(_FIRST, KC_QUOT),
 //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_LSFT,    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                         KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                            KC_LGUI,   MO(_FIRST),  LALT_ENT,                   HYPR_SPC,   KC_BSPC, RSE_LEADER
                                      //`--------------------------'  `--------------------------'

  ),

    [_FIRST] = LAYOUT_split_3x6_3(
            //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      _______, LSFT(KC_1), LSFT(KC_2), LSFT(KC_3), LSFT(KC_4), LSFT(KC_5),       LSFT(KC_6), LSFT(KC_7), LSFT(KC_8), LSFT(KC_9), LSFT(KC_0), KC_BSLS,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      _______, KC_1, KC_2, KC_3, KC_4, KC_5,                                     KC_MINS, KC_EQL, KC_GRV, KC_LBRC, KC_RBRC, LSFT(KC_BSLS),
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      _______, KC_6, KC_7, KC_8, KC_9, KC_0,                                     LSFT(KC_MINS), LSFT(KC_EQL), LSFT(KC_GRV), LSFT(KC_LBRC), LSFT(KC_RBRC), _______,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          _______, _______,  _______,     _______,   _______, _______
                                      //`--------------------------'  `--------------------------'
  ),

    [_SECOND] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
       QK_BOOT, KC_F1,   KC_F2, KC_F3,  KC_F4, KC_F5,                            KC_F6, KC_F7, KC_F8, KC_F9, KC_0, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      _______, KC_MPRV, KC_MNXT, KC_VOLD, KC_VOLU, KC_MPLY,                      KC_LEFT,  KC_DOWN, KC_UP, KC_RIGHT, XXXXXXX,  XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      RGB_TOG, RGB_MOD, RGB_HUI, RGB_SAI, RGB_VAI, RGB_SPI,                      KC_WH_L, KC_WH_U, 	KC_WH_D, KC_WH_R, XXXXXXX, _______,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          _______,   _______,  _______,     _______, _______, _______
                                      //`--------------------------'  `--------------------------'
  )
};

