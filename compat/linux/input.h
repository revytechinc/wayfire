/* SPDX-License-Identifier: MIT */
/*
 * FreeBSD compatibility shim for linux/input.h
 */
#ifndef _LINUX_INPUT_H
#define _LINUX_INPUT_H
#include <stdint.h>
#define EV_SYN 0x00
#define EV_KEY 0x01
#define EV_REL 0x02
#define EV_ABS 0x03
#define EV_MSC 0x04
#define EV_LED 0x11
#define EV_SND 0x12
#define EV_REP 0x14
#define EV_FF 0x15
#define EV_PWR 0x16
#define EV_FF_STATUS 0x17
#define KEY_F1 0x3b
#define KEY_F2 0x3c
#define KEY_F3 0x3d
#define KEY_F4 0x3e
#define KEY_F5 0x3f
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_F11 0x45
#define KEY_F12 0x46
#define KEY_F13 0x47
#define KEY_F14 0x48
#define KEY_F15 0x49
#define KEY_F16 0x4a
#define KEY_F17 0x4b
#define KEY_F18 0x4c
#define KEY_F19 0x4d
#define KEY_F20 0x4e
#define KEY_F21 0x4f
#define KEY_F22 0x50
#define KEY_F23 0x51
#define KEY_F24 0x52
#define BTN_LEFT 0x110
#define BTN_RIGHT 0x111
#define BTN_MIDDLE 0x112
#define BTN_SIDE 0x113
#define BTN_EXTRA 0x114
#define BTN_FORWARD 0x115
#define BTN_BACK 0x116
#define BTN_TASK 0x117
#define BTN_TOOL_PEN 0x140
#define BTN_TOOL_RUBBER 0x141
#define BTN_TOOL_BRUSH 0x142
#define BTN_TOOL_PENCIL 0x143
#define BTN_TOOL_AIRBRUSH 0x144
#define BTN_TOOL_FINGER 0x145
#define BTN_TOOL_MOUSE 0x146
#define BTN_TOOL_LENS 0x147
#define BTN_TOOL_TOUCH 0x14a
#define BTN_TOUCH 0x14a
#define BTN_STYLUS 0x14b
#define BTN_STYLUS2 0x14c
#define BTN_STYLUS3 0x14d
#define ABS_X 0x00
#define ABS_Y 0x01
#define ABS_Z 0x02
#define ABS_RX 0x03
#define ABS_RY 0x04
#define ABS_RZ 0x05
#define ABS_THROTTLE 0x06
#define ABS_RUDDER 0x07
#define ABS_WHEEL 0x08
#define ABS_GAS 0x09
#define ABS_BRAKE 0x0a
#define ABS_PRESSURE 0x18
#define ABS_DISTANCE 0x19
#define ABS_TILT_X 0x1a
#define ABS_TILT_Y 0x1b
#define ABS_MISC 0x28
#define BUS_USB 0x03
#define BUS_HIL 0x00
#define BUS_PS2 0x01
#define BUS_VIRTUAL 0x06
#define BUS_AXIS 0x06
#define BUS_XEN 0x0C
#endif /* _LINUX_INPUT_H */
