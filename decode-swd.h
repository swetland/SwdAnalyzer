/* Copyright 2015 Brian Swetland <swetland@frotz.net>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DECODE_SWD_H_
#define _DECODE_SWD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	S_OFFLINE,
	S_RESET,
	S_IDLE,
	S_HEADER,
	S_HEADERT,
	S_REPLY,
	S_READ32,
	S_READP,
	S_READT,
	S_WRITET,
	S_WRITE32,
	S_WRITEP,
	S_IDLET,
} swd_state_t;

typedef struct {
	swd_state_t state;
	int resetcount;
	int lastindata;
	unsigned count;
	unsigned header;
	unsigned payload;
	unsigned ack;
	unsigned parity;
	uint64_t start;
	unsigned select;
} swd_t;

void swd_init(swd_t *s);


#define EMIT_NOTHING 0
#define EMIT_HDR 1
#define EMIT_TURN 2
#define EMIT_ACK 3
#define EMIT_DATA 4
#define EMIT_PARITY 5

int swd_process(swd_t *s, int edge, int data, uint64_t ts);

#ifdef __cplusplus
};
#endif

#endif
