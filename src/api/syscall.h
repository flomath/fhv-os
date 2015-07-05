/*
 * syscall.h
 *
 * Created on: 02.07.15
 *     Author: Nino Schoch
 */

#ifndef SRC_API_SYSCALL_H
#define SRC_API_SYSCALL_H

#include <inttypes.h>

#define SYS_DEBUG		10
#define SYS_EXIT	    20
#define SYS_PRINT       30
#define SYS_READ        31

#define SYS_LOAD_PROC   40
#define SYS_START_PROC  41

#define SYS_AUDIO_PLAYL  50
#define SYS_AUDIO_PLAYR  51

#pragma SWI_ALIAS(syscall, 0)
void syscall(uint32_t swiID, uint32_t params[], uint32_t paramLength);

#endif //SRC_API_SYSCALL_H
