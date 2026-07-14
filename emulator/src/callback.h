/* -------------------------------------------------------------------------- */
/* - MMU Callbacks for ÖverDOS Emulator                                     - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#ifndef CALLBACK_H
#define CALLBACK_H

void cb_uart0 (void *data, int size, int offset, int action);
void cb_interrupt_config (void *data, int size, int offset, int action);
void cb_led (void *data, int size, int offset, int action);
void cb_rtc (void *data, int size, int offset, int action);

#endif
