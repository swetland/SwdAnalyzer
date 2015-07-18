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

#include "SwdAnalyzer.h"
#include "SwdAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

#include <stdint.h>
#include "decode-swd.h"

SwdAnalyzer::SwdAnalyzer()
:	Analyzer(),  
	mSettings(new SwdAnalyzerSettings())
{
	SetAnalyzerSettings(mSettings.get());
}

SwdAnalyzer::~SwdAnalyzer()
{
	KillThread();
}

void SwdAnalyzer::WorkerThread()
{
	U64 last, next;
	int op;
	swd_t swd;
	swd_init(&swd);

	mResults.reset(new SwdAnalyzerResults(this, mSettings.get()));
	SetAnalyzerResults(mResults.get());
	mResults->AddChannelBubblesWillAppearOn(mSettings->mDataChannel);

	mClock = GetAnalyzerChannelData(mSettings->mClockChannel);
	mData = GetAnalyzerChannelData(mSettings->mDataChannel);

	// find first +edge
	mClock->AdvanceToNextEdge();
	if (mClock->GetBitState() == BIT_LOW)
		mClock->AdvanceToNextEdge();
	next = mClock->GetSampleNumber();
	mData->AdvanceToAbsPosition(next);

	Frame f;

	for (;;) {
		last = next;
		op = swd_process(&swd, 1, mData->GetBitState() == BIT_HIGH, last);

		if (op) {
			f.mStartingSampleInclusive = swd.start;
			f.mType = op;
			f.mFlags = 0;
			switch (op) {
			case EMIT_HDR:
				f.mData1 = swd.header;
				break;
			case EMIT_ACK:
				f.mData1 = swd.ack;
				break;
			case EMIT_DATA:
				f.mData1 = swd.payload;
				break;
			}
		}
			
		mClock->AdvanceToNextEdge();
		next = mClock->GetSampleNumber();
		if (next == last) break;
		mData->AdvanceToAbsPosition(next);
		last = next;

		swd_process(&swd, 0, mData->GetBitState() == BIT_HIGH, next);

		mClock->AdvanceToNextEdge();
		next = mClock->GetSampleNumber();
		if (next == last) break;
		mData->AdvanceToAbsPosition(next);

		if (op) {
			if (op == EMIT_HDR) {
				mResults->CommitPacketAndStartNewPacket();
			}
			f.mEndingSampleInclusive = next - 1;
			mResults->AddFrame(f);
			mResults->CommitResults();
			ReportProgress(f.mEndingSampleInclusive);
		}
	}
}

bool SwdAnalyzer::NeedsRerun()
{
	return false;
}

U32 SwdAnalyzer::GenerateSimulationData(U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	return 0;
}

U32 SwdAnalyzer::GetMinimumSampleRateHz()
{
	return 1000;
}

const char* SwdAnalyzer::GetAnalyzerName() const
{
	return "SWD";
}

const char* GetAnalyzerName()
{
	return "SWD";
}

Analyzer* CreateAnalyzer()
{
	return new SwdAnalyzer();
}

void DestroyAnalyzer(Analyzer* analyzer)
{
	delete analyzer;
}
