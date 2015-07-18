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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "decode-swd.h"

const char *ackstr[8] = {
	"??? 0",
	"OK",
	"WAIT",
	"??? 3",
	"FAULT",
	"??? 5",
	"??? 6",
	"??? 7",
};

// START=1 RDnWR DPnAP A2 A3 PARITY STOP=0 PARK=1

#define A_OK		1
#define A_WAIT		2
#define A_FAULT		4

#define H_AP		2
#define H_RD		4	
#define H_ADDR(n)	(((n) >> 3) & 3)

const char *dprdstr[4] = {
	"IDCODE  ",
	"CTRLSTAT",
	"RESEND  ",
	"RDBUFF  ",
};

const char *dpwrstr[4] = {
	"ABORT   ",
	"CTRLSTAT",
	"SELECT  ",
	"RDBUFF  ",
};

void swd_init(swd_t *s) {
	s->state = S_OFFLINE;
	//s->state = S_IDLE; 
	s->resetcount = 0;
	s->lastindata = 0;
	s->count = 0;
	s->header = 0;
	s->payload = 0;
	s->ack = 0;
	s->parity = 0;
	s->select = 0;
}

const char *addrstr(swd_t *s) {
	static char buf[32];
	if (s->header & H_AP) {
		unsigned r = (H_ADDR(s->header) << 2) | (s->select & 0xF0);

		switch (r) {
		case 0x00: return "CSW     ";
		case 0x04: return "TAR     ";
		case 0x0C: return "DRW     ";
		case 0x10: return "BD0     ";
		case 0x14: return "BD1     ";
		case 0x18: return "BD2     ";
		case 0x1C: return "BD3     ";
		case 0xF4: return "CFG     ";
		case 0xF8: return "BASE    ";
		case 0xFC: return "IDR     ";
		default:
			sprintf(buf, "%02x      ", r);
			return buf;
		}
	} else {
		if (s->header & H_RD) {
			return dprdstr[H_ADDR(s->header)];
		} else {
			return dpwrstr[H_ADDR(s->header)];
		}
	}
}

void swd_header(unsigned hdr, char *out) {
	if (hdr & H_AP) {
		sprintf(out, "%s AP.0x?%x",
			(hdr & H_RD) ? "RD" : "WR",
			H_ADDR(hdr) << 2);
	} else {
		sprintf(out, "%s DP.%s",
			(hdr & H_RD) ? "RD" : "WR",
			(hdr & H_RD) ? dprdstr[H_ADDR(hdr)] : dpwrstr[H_ADDR(hdr)]);
	}
}

void printtxn(swd_t *s) {
#ifndef PLUGIN
	const char *x;
	switch(s->ack) {
	case 1: x = "OKAY "; break;
	case 2: x = "WAIT "; break;
	case 4: x = "FAULT"; break;
	default: x = "INVAL"; break;
	}

	if (s->ack == A_OK) {
		printf("TXN %s %s.%s %s %08x\n",
			(s->header & H_RD) ? "RD" : "WR",
			(s->header & H_AP) ? "AP" : "DP",
			addrstr(s), x, s->payload);

		if (s->header & H_AP) return;
		if (s->header & H_RD) return;
		if (H_ADDR(s->header) != 2) return;
		s->select = s->payload;
	} else {
		printf("TXN %s %s.%s %s\n",
			(s->header & H_RD) ? "RD" : "WR",
			(s->header & H_AP) ? "AP" : "DP",
			addrstr(s), x);
	}
#endif
}

#ifdef PLUGIN
#define printf(x...) do {} while (0)
#endif

int swd_process(swd_t *s, int edge, int data, uint64_t ts) {
	if (edge == 0) {
		// sample input on falling edges
		s->lastindata = data;
		return 0;
	}
	// always watch for bus reset sequence
	if (data) {
		s->resetcount++;
		if (s->resetcount >= 50) {
			s->state = S_RESET;
		}
	} else {
		s->resetcount = 0;
	}

	switch (s->state) {
	case S_OFFLINE:
		break;
	case S_RESET:
		if (data == 0) {
			printf("RESET\n");
			s->state = S_IDLE;
			s->count = 1;
		}
		break;
	case S_IDLE:
		if (data == 1) {
			if (s->count) printf("IDLE %d\n", s->count);
			s->start = ts;
			s->state = S_HEADER;
			s->count = 1;
			s->header = 1;
			s->parity = 1;
		} else {
			s->count++;
		}
		break;
	case S_HEADER:
		s->parity ^= data;
		s->header |= (data << s->count);
		s->count++;
		if (s->count == 8) {
			if (s->parity) {
				printf("HDR %02x PARITY ERROR\n", s->header);
				s->state = S_OFFLINE;
			} else if (s->header & 0x40) {
				printf("HDR %02x BAD PARK BIT\n", s->header);
				s->state = S_OFFLINE;
			} else {
				printf("HDR %02x %s %s %d\n", s->header,
					(s->header & H_RD) ? "RD" : "WR",
					(s->header & H_AP) ? "AP" : "DP",
					H_ADDR(s->header));
				s->state = S_HEADERT;
				s->count = 0;
				s->ack = 0;
			}
			return EMIT_HDR;
		}
		break;	
	case S_HEADERT:
		s->state = S_REPLY;
		s->start = ts;
		return EMIT_TURN;
	case S_REPLY:
		if (s->count == 0) {
			s->start = ts;
		}
		data = s->lastindata;
		s->ack |= (data << s->count);
		s->count++;
		if (s->count == 3) {
			printf("ACK %s\n", ackstr[s->ack]);
			s->payload = 0;
			s->count = 0;
			s->parity = 0;
			if (s->header & H_RD) {
				if (s->ack == A_OK) {
					s->state = S_READ32;
				} else {
					s->state = S_IDLET;
					printtxn(s);
				}
			} else {
				if (s->ack == A_OK) {
					s->state = S_WRITET;
				} else {
					s->state = S_IDLET;
					printtxn(s);
				}
			}
			return EMIT_ACK;
		}
		break;
	case S_IDLET:
		s->state = S_IDLE;
		s->count = 0;
		s->start = ts;
		return EMIT_TURN;
	case S_READ32:
		if (s->count == 0) {
			s->start = ts;
		}
		data = s->lastindata;
		s->parity ^= data;
		s->payload |= (data << s->count);
		s->count++;
		if (s->count == 32) {
			s->state = S_READP;
			printtxn(s);
			return EMIT_DATA;
		}
		break;
	case S_READP:
		data = s->lastindata;
		if (s->parity != data) {
			printf(" RD %08x PARITY ERROR\n", s->payload);
		} else {
			printf(" RD %08x\n", s->payload);
		}
		s->state = S_READT;
		s->start = ts;
		return EMIT_PARITY;
	case S_READT:
		s->state = S_IDLE;
		s->count = 0;
		s->start = ts;
		return EMIT_TURN;
	case S_WRITET:
		s->state = S_WRITE32;
		s->start = ts;
		return EMIT_TURN;
	case S_WRITE32:
		if (s->count == 0) {
			s->start = ts;
		}
		s->parity ^= data;
		s->payload |= (data << s->count);
		s->count++;
		if (s->count == 32) {
			s->state = S_WRITEP;
			printtxn(s);
			return EMIT_DATA;
		}
		break;
	case S_WRITEP:
		if (s->parity != data) {
			printf(" WR %08x PARITY ERROR\n", s->payload);
		} else {
			printf(" WR %08x\n", s->payload);
		}
		s->state = S_IDLE;
		s->count = 0;
		s->start = ts;
		return EMIT_PARITY;
	}
	return 0;
}

#ifdef PLUGIN
#undef printf
#endif

#ifndef PLUGIN

#define CLK 1
#define DAT 2

// logic binary export file
typedef struct {
	uint64_t when;
	uint8_t bits;
} __attribute__((packed)) sample;

int main(int argc, char **argv) {
	swd_t swd;
	sample s;
	uint8_t last;
	swd_init(&swd);
	if (read(0, &s, sizeof(s)) != sizeof(s)) {
		return 0;
	}
	last = s.bits;	
	while (read(0, &s, sizeof(s)) == sizeof(s)) {
		if ((last & CLK) != (s.bits & CLK)) {
			swd_process(&swd, !!(s.bits & CLK), !!(s.bits & DAT), s.when);
#if 0
			switch (op) {
			case EMIT_HDR:
				printf("H %02x\n", swd.header);
				break;
			case EMIT_ACK:
				printf("A %x\n", swd.ack);
				break;
			case EMIT_TURN:
				printf("T\n");
				break;
			case EMIT_PARITY:
				printf("P\n");
				break;
			case EMIT_DATA:
				printf("D %08x\n", swd.payload);
				break;
			}
#endif
			last = s.bits;
		}
	}
	return 0;
}
#endif
