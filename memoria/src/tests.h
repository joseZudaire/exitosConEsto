#ifndef TEST_H_
#define TEST_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <pthread.h>

void test_tlb();
void test_GLOBAL_CACHE_LRU_TLB_FIFO();
void test_GLOBAL_CACHE_LRU_TLB_LRU();
void test_GLOBAL_CACHE_MCLOCK_TLB_FIFO();
void test_GLOBAL_CACHE_MCLOCK_TLB_LRU();
void test_FIJA_CACHE_LRU_TLB_FIFO();
void test_FIJA_CACHE_LRU_TLB_LRU();
void test_mini_FIJA_CACHE_MCLOCK_TLB_FIFO();
void test_FIJA_CACHE_MCLOCK_TLB_FIFO();
void test_FIJA_CACHE_MCLOCK_TLB_LRU();
void test_obtener_todas_las_paginas_con_presencia_en_cache();
void test_paginacion();

#endif /* TEST_H_ */
