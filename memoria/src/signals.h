#ifndef SIGNALS_H_
#define SIGNALS_H_

#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <time.h>
#include <signal.h>
#include "error.h"
#include "memory.h"

#include <stdio.h>
#include <string.h>
#include <commons/collections/list.h>
#include "lib.h"

void sig_handler_sigint(int);
void sig_handler_sig1(int);
void sig_handler_sig2(int);
void register_signals();

void increment_hit(t_carpincho*);
void increment_miss(t_carpincho*);


char* retornarFechaHora ();
void dump();

#endif /* SIGNALS_H_ */
