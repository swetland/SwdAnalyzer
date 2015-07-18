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

#include "SwdAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "SwdAnalyzer.h"
#include "SwdAnalyzerSettings.h"
#include <iostream>
#include <fstream>
#include <string.h>

#include <stdint.h>
#include "decode-swd.h"

SwdAnalyzerResults::SwdAnalyzerResults(SwdAnalyzer* analyzer, SwdAnalyzerSettings* settings)
:	AnalyzerResults(),
	mSettings(settings),
	mAnalyzer(analyzer)
{
}

SwdAnalyzerResults::~SwdAnalyzerResults()
{
}

void SwdAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base)
{
	char tmp[128];
	ClearResultStrings();
	Frame frame = GetFrame(frame_index);

	switch (frame.mType) {
	case EMIT_HDR:
		swd_header(frame.mData1, tmp);
		//sprintf(tmp, "H=%02x", (unsigned) frame.mData1);
		break;
	case EMIT_TURN:
		strcpy(tmp,"T");
		break;
	case EMIT_ACK:
		switch(frame.mData1) {
		case 1: strcpy(tmp,"OK"); break;
		case 2: strcpy(tmp,"WAIT"); break;
		case 4: strcpy(tmp,"FAULT"); break;
		default: strcpy(tmp,"INVAL"); break;
		}
		break;
	case EMIT_DATA:
		sprintf(tmp, "DATA: %08x", (unsigned) frame.mData1);
		break;
	case EMIT_PARITY:
		strcpy(tmp,"P");
		break;
	default:
		strcpy(tmp,"?");
	}
	AddResultString(tmp);
}

void SwdAnalyzerResults::GenerateExportFile(const char* file, DisplayBase display_base, U32 export_type_user_id)
{
}

void SwdAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
}

void SwdAnalyzerResults::GeneratePacketTabularText(U64 packet_id, DisplayBase display_base)
{
	ClearResultStrings();
	AddResultString("not supported");
}

void SwdAnalyzerResults::GenerateTransactionTabularText(U64 transaction_id, DisplayBase display_base)
{
	ClearResultStrings();
	AddResultString("not supported");
}
