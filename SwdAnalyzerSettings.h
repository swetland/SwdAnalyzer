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

#ifndef SWD_ANALYZER_SETTINGS
#define SWD_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class SwdAnalyzerSettings : public AnalyzerSettings
{
public:
	SwdAnalyzerSettings();
	virtual ~SwdAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings(const char* settings);
	virtual const char* SaveSettings();
	
	Channel mClockChannel;
	Channel mDataChannel;

protected:
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mClockChannelInterface;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface;
};

#endif
