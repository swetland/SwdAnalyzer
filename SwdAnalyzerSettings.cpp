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

#include "SwdAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

SwdAnalyzerSettings::SwdAnalyzerSettings()
:	mClockChannel(UNDEFINED_CHANNEL),
	mDataChannel(UNDEFINED_CHANNEL)
{
	mClockChannelInterface.reset( new AnalyzerSettingInterfaceChannel());
	mClockChannelInterface->SetTitleAndTooltip("Clock", "Standard SWD");
	mClockChannelInterface->SetChannel( mClockChannel );

	mDataChannelInterface.reset( new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface->SetTitleAndTooltip("Data", "Standard SWD");
	mDataChannelInterface->SetChannel( mDataChannel );

	AddInterface(mClockChannelInterface.get());
	AddInterface(mDataChannelInterface.get());

	AddExportOption(0, "Export as text/csv file");
	AddExportExtension(0, "text", "txt");
	AddExportExtension(0, "csv", "csv");

	ClearChannels();
	AddChannel(mClockChannel, "Clock", false);
	AddChannel(mDataChannel, "Data", false);
}

SwdAnalyzerSettings::~SwdAnalyzerSettings()
{
}

bool SwdAnalyzerSettings::SetSettingsFromInterfaces()
{
	mClockChannel = mClockChannelInterface->GetChannel();
	mDataChannel = mDataChannelInterface->GetChannel();

	ClearChannels();
	AddChannel(mClockChannel, "SWDCLK", true);
	AddChannel(mDataChannel, "SWDIO", true);

	return true;
}

void SwdAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mClockChannelInterface->SetChannel(mClockChannel);
	mDataChannelInterface->SetChannel(mDataChannel);
}

void SwdAnalyzerSettings::LoadSettings(const char* settings)
{
	SimpleArchive text_archive;
	text_archive.SetString(settings);

	text_archive >> mClockChannel;
	text_archive >> mDataChannel;

	ClearChannels();
	AddChannel(mClockChannel, "SWDCLK", true);
	AddChannel(mDataChannel, "SWDIO", true);

	UpdateInterfacesFromSettings();
}

const char* SwdAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mClockChannel;
	text_archive << mDataChannel;

	return SetReturnString(text_archive.GetString());
}
