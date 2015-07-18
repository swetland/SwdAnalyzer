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

#ifndef SWD_ANALYZER_H
#define SWD_ANALYZER_H

#include <Analyzer.h>
#include "SwdAnalyzerResults.h"

class SwdAnalyzerSettings;
class ANALYZER_EXPORT SwdAnalyzer : public Analyzer
{
public:
	SwdAnalyzer();
	virtual ~SwdAnalyzer();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels);
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected:
	std::auto_ptr<SwdAnalyzerSettings> mSettings;
	std::auto_ptr< SwdAnalyzerResults > mResults;
	AnalyzerChannelData* mClock;
	AnalyzerChannelData* mData;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif
