/*
 Copyright (c) 2015, The Cinder Project

 This code is intended to be used with the Cinder C++ library, http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "cinder/audio/linux/DeviceManagerPulseAudio.h"
#include <pulse/pulseaudio.h>

// Similar to Windows - there doesn't seem to be a way to get
// the preferred frame size in PulseAudio.
#define PREFERRED_FRAMES_PER_BLOCK 512

namespace cinder { namespace audio { namespace linux {

DeviceManagerPulseAudio::DeviceManagerPulseAudio()
{

}

DeviceManagerPulseAudio::~DeviceManagerPulseAudio()
{
	
}

const std::vector<DeviceRef>& DeviceManagerPulseAudio::getDevices()
{
	if( mDevices.empty() ) {
	    parseDevices( DeviceInfo::INPUT );
	    parseDevices( DeviceInfo::OUTPUT );
	}

	return mDevices;	
}

DeviceRef DeviceManagerPulseAudio::getDefaultOutput()
{
	
}

DeviceRef DeviceManagerPulseAudio::getDefaultInput()
{
	
}

std::string DeviceManagerPulseAudio::getName( const DeviceRef &device )
{
	return getDeviceInfo( device ).mName;	
}

size_t DeviceManagerPulseAudio::getNumInputChannels( const DeviceRef &device )
{
	auto& devInfo = getDeviceInfo( device );
	if( devInfo.mUsage != DeviceInfo::Usage::INPUT ) {
		return 0;
    }

	return devInfo.mNumChannels;	
}

size_t DeviceManagerPulseAudio::getNumOutputChannels( const DeviceRef &device )
{
	auto& devInfo = getDeviceInfo( device );
	if( devInfo.mUsage != DeviceInfo::Usage::OUTPUT ) {
		return 0;
    }

	return devInfo.mNumChannels;	
}

size_t DeviceManagerPulseAudio::getSampleRate( const DeviceRef &device )
{
	return getDeviceInfo( device ).mSampleRate;	
}

size_t DeviceManagerPulseAudio::getFramesPerBlock( const DeviceRef &device )
{
	size_t frames = getDeviceInfo( device ).mFramesPerBlock;
	return frames;	
}

void DeviceManagerPulseAudio::setSampleRate( const DeviceRef &device, size_t sampleRate )
{
	getDeviceInfo( device ).mSampleRate = sampleRate;
	emitParamsDidChange( device );
}

void DeviceManagerPulseAudio::setFramesPerBlock( const DeviceRef &device, size_t framesPerBlock )
{
	getDeviceInfo( device ).mFramesPerBlock = framesPerBlock;
	emitParamsDidChange( device );
}

size_t DeviceManagerPulseAudio::getFramesPerBlockHardware( const DeviceRef &device )
{
	// TODO: Fix this - a temp solution for now.
	return getFramesPerBlock( device );    
}

// ----------------------------------------------------------------------------------------------------
// MARK: - Private
// ----------------------------------------------------------------------------------------------------

DeviceManagerPulseAudio::DeviceInfo& DeviceManagerPulseAudio::getDeviceInfo( const DeviceRef &device )
{
	return mDeviceInfoSet.at( device );
}

namespace pulse {

using DeviceInfo = DeviceManagerPulseAudio::DeviceInfo;

struct StateData {
	enum Request { INVALID, SINKS, SOURCES, DONE };
	Request					request = StateData::INVALID;	
	pa_mainloop_api 		*mainLoopApi = nullptr;
	std::vector<DeviceInfo>	*deviceInfos = nullptr;
};

void drainContextComplete( pa_context *context, void *userData )
{
	pa_context_disconnect( context );
}

void drainContext( pa_context *context, StateData* stateData )
{
	pa_operation *op = pa_context_drain( context, drainContextComplete, nullptr );
	if( ! op ) {
		pa_context_disconnect( context );
	}
	else {
		pa_operation_unref( op );
	}
}

void checkComplete( pa_context *context, StateData* stateData )
{
	if( StateData::DONE == stateData->request ) {
		drainContext( context, stateData );
	}
}

template <typename PulseInfoT>
DeviceInfo createDeviceInfo( const PulseInfoT* info )
{
	DeviceInfo result;
	result.mName 			= std::string( info->name );
	result.mIndex			= info->index;
	result.mCard			= info->card;
	result.mUsage			= DeviceInfo::INPUT;
	result.mNumChannels		= info->channel_map.channels;
	result.mSampleRate		= info->sample_spec.rate;
	result.mFramesPerBlock	= PREFERRED_FRAMES_PER_BLOCK;
	result.mDescription		= std::string( info->description );
	return result;
}

void processSinkInfo( pa_context *context, const pa_sink_info *info, int eol, void *userData )
{
	StateData* stateData = reinterpret_cast<StateData*>( userData );

	if( eol < 0 ) {
		return;
	}

	if( eol ) {
		stateData->request = StateData::DONE;
	}
	else {
		DeviceInfo devInfo = createDeviceInfo<pa_sink_info>( info );
		stateData->deviceInfos->push_back( devInfo );
	}
	
	checkComplete( context, stateData );
}

void processSourceInfo( pa_context *context, const pa_source_info *info, int eol, void *userData )
{
	StateData* stateData = reinterpret_cast<StateData*>( userData );

	if( eol < 0 ) {
		return;
	}

	if( eol ) {
		stateData->request = StateData::DONE;
	}
	else {
		DeviceInfo devInfo = createDeviceInfo<pa_source_info>( info );
		stateData->deviceInfos->push_back( devInfo );
	}
	
	checkComplete( context, stateData );
}

void processContextState( pa_context *context, void *userData )
{
	StateData* stateData = reinterpret_cast<StateData*>( userData );
	pa_context_state_t state = ::pa_context_get_state( context );
	switch( state ) {
		case PA_CONTEXT_READY: 
			{
				switch( stateData->request ) {
					case StateData::SINKS:
						pa_operation_unref( pa_context_get_sink_info_list( context, processSinkInfo, userData ) );
						break;
					case StateData::SOURCES:
						pa_operation_unref( pa_context_get_source_info_list( context, processSourceInfo, userData ) );
						break;
					case StateData::DONE:
						stateData->mainLoopApi->quit( stateData->mainLoopApi, 0 );
						break;
					case StateData::INVALID:
						stateData->mainLoopApi->quit( stateData->mainLoopApi, -1 );
						break;
				}
			}
			break;
		case PA_CONTEXT_TERMINATED:
			stateData->mainLoopApi->quit( stateData->mainLoopApi, 0 );
			break;
		case PA_CONTEXT_FAILED:
			stateData->mainLoopApi->quit( stateData->mainLoopApi, -1 );
			break;
		default:
			break;
	}
}

std::vector<DeviceInfo> getDeviceInfos( DeviceInfo::Usage usage )
{
	std::vector<DeviceInfo> result;

	// Create mainloop
	pa_mainloop *mainLoop = pa_mainloop_new();
	// Create mainloop API
	pa_mainloop_api *mainLoopApi = pa_mainloop_get_api( mainLoop );
	// Create context
	pa_context *context = pa_context_new( mainLoopApi, "test" );
	// Set context state callback
	StateData stateData;
	stateData.mainLoopApi = mainLoopApi;
	stateData.deviceInfos = &result;
	switch( usage ) {
		case DeviceInfo::OUTPUT : stateData.request = StateData::SINKS;   break;
		case DeviceInfo::INPUT  : stateData.request = StateData::SOURCES; break;
	}
	pa_context_set_state_callback( context, processContextState, reinterpret_cast<void*>( &stateData ) );
	// Connect context
	if( pa_context_connect( context, nullptr, (pa_context_flags_t)0, nullptr ) >= 0 ) {
		// Run mainloop
		int result = 0;
		if( pa_mainloop_run( mainLoop, &result ) < 0 ) {
			// Handle error
		}
	}
	pa_context_unref( context );
	pa_mainloop_free( mainLoop );

	return result;
}

} // namespace pulse

void DeviceManagerPulseAudio::parseDevices( DeviceInfo::Usage usage )
{
	std::vector<DeviceInfo> devInfos = pulse::getDeviceInfos( usage );

	for( auto& devInfo : devInfos ) {
		DeviceRef addedDevice = addDevice( devInfo.mKey );
		mDeviceInfoSet.insert( std::make_pair( addedDevice, devInfo ) );
	}
}

} } } // namespace cinder::audio::linux

