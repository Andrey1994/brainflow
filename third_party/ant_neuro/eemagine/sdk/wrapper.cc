/*
 * License: eego amplifier Interface SDK, rev. 1.3
 *
 *
 * Copyright 2015, eemagine Medical Imaging Solutions GmbH
 *
 *
 * 1. Redistributions of source code must retain the copyright notice this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * 3. All advertising materials mentioning features or use of this software must display the following acknowledgement: This product includes software developed by the eemagine Medical Imaging Solutions GmbH.
 *
 * 4. Neither the name of the eemagine Medical Imaging Solutions GmbH nor the names of its contributors or products may be used to endorse or promote products derived from this software without specific prior written permission by eemagine
 *
 *
 * This Software is provided by eemagine Medical Imaging Solutions GmbH ''As Is'' and any express or implied warranties, including, but not limited to, the implied warranties merchantability and fitness for a particular purpose are disclaimed. In no event shall eemagine be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including but not limited to, procurement of substitute goods or services, loss of use, data, or profits, or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
 *
 * The eego amplifier comes with its own user manual, which must be read carefully before making use of this SDK or any derived software.
 *
 *
 * You need to run full testing and qualification based on your own requirements to claim any performance of the combined system.
 *
 *
 * Please be especially advised that any certification holding for the eego amplifier is not valid for a combined system of your application software and the eego amplifier. You must obtain your own certification for a combined system of amplifier and software.
 */

// system
#pragma warning(disable: 4996)
#include <string.h>
#include <iostream>
#include <map>
#include <sstream>

// eemagine
#include <eemagine/sdk/wrapper.h>
#include <eemagine/sdk/factory.h>
#include <eemagine/sdk/version.h>

// It has to be either dynamic or static
#if !(defined(EEGO_SDK_BIND_DYNAMIC) ^ defined(EEGO_SDK_BIND_STATIC))
#pragma message ("Neither EEGO_SDK_BIND_DYNAMIC nor EEGO_SDK_BIND_STATIC has been defined. Default to EEGO_SDK_BIND_DYNAMIC.")
#define EEGO_SDK_BIND_DYNAMIC
#endif

#ifdef EEGO_SDK_BIND_DYNAMIC
#if !(defined(_UNICODE) ^ defined(_MBCS))
#pragma message ("Neither _UNICODE nor _MBCS has been defined. Default to _UNICODE.")
#define _UNICODE
#endif

// Find out what operating system we are using and default to _WIN32
#if !(defined(_WIN32) ^ defined(__unix__))
#pragma message ("Neither _WIN32 nor __unix__ has been defined. Default to _WIN32.")
#define _WIN32
#endif

// For windows we are using UTF16 in any case!
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifdef _WIN32
#  include <Windows.h> // For library loading
#endif
#ifdef __unix__
#  ifdef EEGO_SDK_BIND_DYNAMIC
#    include <dlfcn.h>
#  endif
#endif
#endif

// Function pointers to API functions to enable late bindings
class _sdkBindings
{
public:
	bool initialized;
	/**
	* @brief setup sdk library
	*/
	eemagine_sdk_setup_t setup;
	/**
	* @brief initialze sdk library
	*/
	eemagine_sdk_init_t init;
	/**
	* finalize sdk library
	*/
	eemagine_sdk_exit_t exit;
	/**
	* get the current sdk version
	*/
	eemagine_sdk_get_version_t get_version;
	/**
	* @brief get an array of connected amplifiers
	*/
	eemagine_sdk_get_amplifiers_info_t get_amplifiers_info;
	/**
	* get the channel list from an amplifier
	*/
	eemagine_sdk_get_amplifier_channel_list_t get_amplifier_channel_list;
	/**
	* close amplifier
	*/
	eemagine_sdk_close_amplifier_t close_amplifier;
	/**
	* get the serial from an amplifier
	*/
	eemagine_sdk_get_amplifier_serial_t get_amplifier_serial;
	/**
	* get the firmware version of the amplifier
	*/
	eemagine_sdk_get_amplifier_version_t get_amplifier_version;
	/**
	* get the type for this amplifier
	*/
	eemagine_sdk_get_amplifier_type_t get_amplifier_type;
	/**
	* get a list of sampling rates that are available
	*/
	eemagine_sdk_get_amplifier_sampling_rates_available_t get_amplifier_sampling_rates_available;
	/**
	* get a list of reference ranges that are available
	*/
	eemagine_sdk_get_amplifier_reference_ranges_available_t get_amplifier_reference_ranges_available;
	/**
	* get a list of bipolar ranges that are available
	*/
	eemagine_sdk_get_amplifier_bipolar_ranges_available_t get_amplifier_bipolar_ranges_available;
	/**
	* open eeg stream, returns handle on success. if handle is negative, it indicates an error
	*/
	eemagine_sdk_open_eeg_stream_t open_eeg_stream;
	/**
	* open impedance stream, returns handle on success. if handle is negative, it indicates an error
	*/
	eemagine_sdk_open_impedance_stream_t open_impedance_stream;
	/**
	* close stream
	*/
	eemagine_sdk_close_stream_t close_stream;
	/**
	* get the channel list from a stream
	*/
	eemagine_sdk_get_stream_channel_list_t get_stream_channel_list;
	/**
	* get the channel count from a stream
	*/
	eemagine_sdk_get_stream_channel_count_t get_stream_channel_count;
	/**
	 * prefetch
	 */
	eemagine_sdk_prefetch_t prefetch;
	/**
	* get data
	*/
	eemagine_sdk_get_data_t get_data;
	/**
	* get error string
	*/
	eemagine_sdk_get_error_string_t get_error_string;

	// Mark a freshly constructed class as unintialized
	_sdkBindings()
		: initialized(false)
	{}
};

// The binding itself
_sdkBindings _sdk;

///////////////////////////////////////////////////////////////////////////////
// Creates an exception from an returned error value
int
_return_value_guard(int rv) {
	char msg[1024];
	switch (rv) {
	case EEMAGINE_SDK_NOT_CONNECTED:
		_sdk.get_error_string(msg, 1024);
		throw(eemagine::sdk::exceptions::notConnected(msg));
		break;
	case EEMAGINE_SDK_ALREADY_EXISTS:
		_sdk.get_error_string(msg, 1024);
		throw(eemagine::sdk::exceptions::alreadyExists(msg));
		break;
	case EEMAGINE_SDK_NOT_FOUND:
		_sdk.get_error_string(msg, 1024);
		throw(eemagine::sdk::exceptions::notFound(msg));
		break;
	case EEMAGINE_SDK_INCORRECT_VALUE:
		_sdk.get_error_string(msg, 1024);
		throw(eemagine::sdk::exceptions::incorrectValue(msg));
		break;
	case EEMAGINE_SDK_UNKNOWN:
		_sdk.get_error_string(msg, 1024);
		throw(eemagine::sdk::exceptions::unknown(msg));
		break;
	}
	return rv;
}

std::vector<eemagine::sdk::channel> _channelArrayToVector(eemagine_sdk_channel_info* channelPtr, unsigned int channelCount)
{
	std::vector<eemagine::sdk::channel> rv; // the list to be filled
	rv.reserve(channelCount);

	for (unsigned channel = 0; channel < channelCount; ++channel)
	{
		eemagine_sdk_channel_info* read_ptr = channelPtr + channel;
		if (read_ptr->index < 0)
		{
			throw eemagine::sdk::exceptions::unknown("Channel index is below zero: " + read_ptr->index);
		}
		unsigned index = read_ptr->index;
		switch (read_ptr->type) {
		case EEMAGINE_SDK_CHANNEL_TYPE_REFERENCE:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::reference));
			break;
		case EEMAGINE_SDK_CHANNEL_TYPE_BIPOLAR:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::bipolar));
			break;
		case EEMAGINE_SDK_CHANNEL_TYPE_TRIGGER:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::trigger));
			break;
		case EEMAGINE_SDK_CHANNEL_TYPE_SAMPLE_COUNTER:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::sample_counter));
			break;
		case EEMAGINE_SDK_CHANNEL_TYPE_IMPEDANCE_REFERENCE:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::impedance_reference));
			break;
		case EEMAGINE_SDK_CHANNEL_TYPE_IMPEDANCE_GROUND:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::impedance_ground));
			break;
		case EEMAGINE_SDK_CHANNEL_TYPE_ACCELEROMETER:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::accelerometer));
			break;
		case EEMAGINE_SDK_CHANNEL_TYPE_GYROSCOPE:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::gyroscope));
			break;
		case EEMAGINE_SDK_CHANNEL_TYPE_MAGNETOMETER:
			rv.push_back(eemagine::sdk::channel(index, eemagine::sdk::channel::magnetometer));
			break;
		default:
			throw eemagine::sdk::exceptions::unknown("Channel type unknown: " + read_ptr->type);
		}
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
class _sdk_stream : public eemagine::sdk::stream
{
public:
	_sdk_stream(int stream_id)
		: _stream_id(stream_id)
		, _channelCount(_return_value_guard(_sdk.get_stream_channel_count(_stream_id)))
	{
	}

	~_sdk_stream() {
		_sdk.close_stream(_stream_id);
	}

	unsigned int getChannelCount() {
		return _channelCount;
	}

	eemagine::sdk::buffer getData() {
		int bytes_to_allocate(_return_value_guard(_sdk.prefetch(_stream_id)));
		int double_count(bytes_to_allocate / sizeof(double));
		eemagine::sdk::buffer rv(getChannelCount(), double_count / getChannelCount());
		_return_value_guard(_sdk.get_data(_stream_id, rv.data(), bytes_to_allocate));
		return rv;
	}

	std::vector<eemagine::sdk::channel> getChannelList() const
	{
		// get amount of channels first and transform it into external format
		eemagine_sdk_channel_info channel_info_array[1024];
		int channel_count = _return_value_guard(_sdk.get_stream_channel_list(_stream_id, channel_info_array, 1024));
		return _channelArrayToVector(channel_info_array, channel_count);
	}

protected:
	const int           _stream_id;
	const unsigned int  _channelCount;
};

///////////////////////////////////////////////////////////////////////////////

void increment_amplifier_reference_count(const eemagine_sdk_amplifier_info & info);
void decrement_amplifier_reference_count(const eemagine_sdk_amplifier_info & info);

///////////////////////////////////////////////////////////////////////////////
class _sdk_amplifier : public eemagine::sdk::amplifier {
public:
	_sdk_amplifier(eemagine_sdk_amplifier_info info) : _amplifier_info(info) {
    }
	~_sdk_amplifier() {
		_sdk.close_amplifier(_amplifier_info.id);
		decrement_amplifier_reference_count(_amplifier_info);
	}

	std::string getSerialNumber() const override {
		char serial[1024];
		_return_value_guard(_sdk.get_amplifier_serial(_amplifier_info.id, serial, 1024));
		return std::string(serial);
	}

	int getFirmwareVersion() const override {
		return _return_value_guard(_sdk.get_amplifier_version(_amplifier_info.id));
	}

	std::string getType() const override {
		char type[1024];
		_return_value_guard(_sdk.get_amplifier_type(_amplifier_info.id, type, 1024));
		return std::string(type);
	}

	std::vector<eemagine::sdk::channel> getChannelList() const override {
		std::vector<eemagine::sdk::channel> rv;
		eemagine_sdk_channel_info channel_info_array[1024];
		int channel_count = _return_value_guard(_sdk.get_amplifier_channel_list(_amplifier_info.id, channel_info_array, 1024));
		return _channelArrayToVector(channel_info_array, channel_count);
	}

	std::vector<int> getSamplingRatesAvailable() const override {
		std::vector<int> rv;
		int sampling_rate_array[1024];
		int sampling_rate_count = _return_value_guard(_sdk.get_amplifier_sampling_rates_available(_amplifier_info.id, sampling_rate_array, 1024));
		rv.resize(sampling_rate_count);
		std::vector<int>::iterator rv_iter(rv.begin());
		const int * read_ptr(sampling_rate_array);
		while (sampling_rate_count) {
			*rv_iter = *read_ptr;
			++rv_iter;
			++read_ptr;
			--sampling_rate_count;
		}
		return rv;
	}

	std::vector<double> getReferenceRangesAvailable() const override {
		std::vector<double> rv;
		double reference_range_array[1024];
		int reference_range_count = _return_value_guard(_sdk.get_amplifier_reference_ranges_available(_amplifier_info.id, reference_range_array, 1024));
		rv.resize(reference_range_count);
		std::vector<double>::iterator rv_iter(rv.begin());
		const double * read_ptr(reference_range_array);
		while (reference_range_count) {
			*rv_iter = *read_ptr;
			++rv_iter;
			++read_ptr;
			--reference_range_count;
		}
		return rv;
	}

	std::vector<double> getBipolarRangesAvailable() const override {
		std::vector<double> rv;
		double bipolar_range_array[1024];
		int bipolar_range_count = _return_value_guard(_sdk.get_amplifier_bipolar_ranges_available(_amplifier_info.id, bipolar_range_array, 1024));
		rv.resize(bipolar_range_count);
		std::vector<double>::iterator rv_iter(rv.begin());
		const double * read_ptr(bipolar_range_array);
		while (bipolar_range_count) {
			*rv_iter = *read_ptr;
			++rv_iter;
			++read_ptr;
			--bipolar_range_count;
		}
		return rv;
	}

	eemagine::sdk::stream * OpenEegStream(int sampling_rate, double reference_range, double bipolar_range, unsigned long long ref_mask, unsigned long long bip_mask) {
		int stream_id = _return_value_guard(_sdk.open_eeg_stream(_amplifier_info.id, sampling_rate, reference_range, bipolar_range, ref_mask, bip_mask));
		return new _sdk_stream(stream_id);
	}

	eemagine::sdk::stream * OpenImpedanceStream(unsigned long long ref_mask) {
		int stream_id = _return_value_guard(_sdk.open_impedance_stream(_amplifier_info.id, ref_mask));
		return new _sdk_stream(stream_id);
	}
protected:
	eemagine_sdk_amplifier_info _amplifier_info;
};
///////////////////////////////////////////////////////////////////////////////
class _sdk_guard {
public:
	_sdk_guard() {
        _sdk.init();
	}
	~_sdk_guard() {
		_sdk.exit();
	}

	void decrementAmplifierReferenceCount(const eemagine_sdk_amplifier_info & info) {
		for (std::vector<_amp_ref_count>::iterator i = _amplifier_reference_count_vector.begin();
			i != _amplifier_reference_count_vector.end();
			++i)
		{
			if (info.id != i->info.id) {
				++i->ref_count;
				return;
			}
		}
	}
	void incrementAmplifierReferenceCount(const eemagine_sdk_amplifier_info & info) {
		for (std::vector<_amp_ref_count>::iterator i = _amplifier_reference_count_vector.begin();
			i != _amplifier_reference_count_vector.end();
			++i)
		{
			if (info.id == i->info.id) {
				++i->ref_count;
				return;
			}
		}
		// if we got here, we didn't find one, so create
		_amp_ref_count arc;
		arc.ref_count = 2;
		arc.info.id = info.id;
		strncpy(arc.info.serial, info.serial, 64);
		arc.amp = new _sdk_amplifier(info);
		_amplifier_reference_count_vector.push_back(arc);
	}

	void sweepAmplifierReferenceCounts() {
		for (std::vector<_amp_ref_count>::iterator i = _amplifier_reference_count_vector.begin(); i != _amplifier_reference_count_vector.end();) {
			--i->ref_count;
			if (i->ref_count == 0) {
				i = _amplifier_reference_count_vector.erase(i);
			}
			else {
				++i;
			}
		}
	}

	std::vector<eemagine::sdk::amplifier *> getAmplifiers() {
		std::vector<eemagine::sdk::amplifier *> rv;
		for (std::vector<_amp_ref_count>::iterator i = _amplifier_reference_count_vector.begin(); i != _amplifier_reference_count_vector.end(); ++i) {
			rv.push_back(i->amp);
		}
		return rv;
	}

protected:
	struct _amp_ref_count {
		int                           ref_count;
		eemagine_sdk_amplifier_info   info;
		eemagine::sdk::amplifier    * amp;
	};
	std::vector<_amp_ref_count> _amplifier_reference_count_vector;
};
///////////////////////////////////////////////////////////////////////////////
_sdk_guard *
_get_sdk_guard_singleton(bool release = false) {
#if _WIN32 && (_MSC_VER < 1900)
	// singleton to sdk initializer guard
	static _sdk_guard * _guard = NULL;

	if (release) {
		if (_guard != NULL) {
			delete _guard;
			_guard = NULL;
		}
	}
	else {
		if (_guard == NULL) {
			_guard = new _sdk_guard();
		}
	}
	return _guard;
#else
	static _sdk_guard _guard_instance;
	return & _guard_instance;
#endif
}
///////////////////////////////////////////////////////////////////////////////
void increment_amplifier_reference_count(const eemagine_sdk_amplifier_info & info) {
	_get_sdk_guard_singleton()->incrementAmplifierReferenceCount(info);
}
void decrement_amplifier_reference_count(const eemagine_sdk_amplifier_info & info) {
	_get_sdk_guard_singleton()->decrementAmplifierReferenceCount(info);
	_get_sdk_guard_singleton()->sweepAmplifierReferenceCounts();
}
///////////////////////////////////////////////////////////////////////////////
#ifdef EEGO_SDK_BIND_STATIC
// trivial case: The dll is bound during compile time. All we have to do is
// assign the defined symbols to the _sdk structure.
eemagine::sdk::factory::factory(void * data)
{
	// Check version first
	if (eemagine_sdk_get_version() != EEGO_SDK_VERSION)
	{
		throw(exceptions::incorrectValue("Eego SDK version mismatch"));
	}

	if (!_sdk.initialized) {
		_sdk.setup = eemagine_sdk_setup;
		_sdk.init = eemagine_sdk_init;
		_sdk.exit = eemagine_sdk_exit;
		_sdk.get_version = eemagine_sdk_get_version;
		_sdk.get_amplifiers_info = eemagine_sdk_get_amplifiers_info;
		_sdk.close_amplifier = eemagine_sdk_close_amplifier;
		_sdk.get_amplifier_channel_list = eemagine_sdk_get_amplifier_channel_list;
		_sdk.get_amplifier_serial = eemagine_sdk_get_amplifier_serial;
		_sdk.get_amplifier_version = eemagine_sdk_get_amplifier_version;
		_sdk.get_amplifier_type = eemagine_sdk_get_amplifier_type;
		_sdk.get_amplifier_sampling_rates_available = eemagine_sdk_get_amplifier_sampling_rates_available;
		_sdk.get_amplifier_reference_ranges_available = eemagine_sdk_get_amplifier_reference_ranges_available;
		_sdk.get_amplifier_bipolar_ranges_available = eemagine_sdk_get_amplifier_bipolar_ranges_available;
		_sdk.open_eeg_stream = eemagine_sdk_open_eeg_stream;
		_sdk.open_impedance_stream = eemagine_sdk_open_impedance_stream;
		_sdk.close_stream = eemagine_sdk_close_stream;
		_sdk.get_stream_channel_list = eemagine_sdk_get_stream_channel_list;
		_sdk.get_stream_channel_count = eemagine_sdk_get_stream_channel_count;
		_sdk.prefetch = eemagine_sdk_prefetch;
		_sdk.get_data = eemagine_sdk_get_data;
		_sdk.get_error_string = eemagine_sdk_get_error_string;
		_sdk.setup(data);
		_sdk.initialized = true;
	}
}
#else
// More complicated case. The path to the dll is provided by call to the factory.
// That DLL has to be loaded and the function pointers have to be searched in there.
// When found they are assigned to the right place in the _sdk structure.
// If they are not found raise a runtime exception

///////////////////////////////////////////////////////////////////////////////////////////////////////
/// All the diffenrent string encodings have to be normalized to utf-8 string for internal handling.
/// To connect to windows methods wide character strings have to be used.
/// String handling methods follow.
///////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
static std::string UTF16toUTF8(const std::wstring& utf16String)
{
	// empty
	if (utf16String.empty())
		return std::string();

	// get length of conversion first or see if there is an error
	const int utf8length = ::WideCharToMultiByte(
		CP_UTF8, WC_ERR_INVALID_CHARS, utf16String.data(), (int)utf16String.length(), // Input
		NULL, 0, // just check
		NULL, NULL); // invalid for utf8

	if (utf8length == 0) // Must be error, as empty string should not happen here anymore
	{
		const DWORD error = ::GetLastError();
		if (error == ERROR_NO_UNICODE_TRANSLATION)
			throw eemagine::sdk::exceptions::incorrectValue("Failure of string conversion from utf16, invalid characters.");

		throw eemagine::sdk::exceptions::incorrectValue("Failure to classify utf16 string. errorcode: " + error);
	}

	std::string retvalUTF8;
	retvalUTF8.resize(utf8length);

	const int convertedLength = ::WideCharToMultiByte(
		CP_UTF8, 0, utf16String.data(), (int)utf16String.length(), // Input
		&retvalUTF8[0], (int)retvalUTF8.length(),
		NULL, NULL); // invalid for utf8

	if (convertedLength == 0)
	{
		throw eemagine::sdk::exceptions::incorrectValue("Failure to convert utf16 string.");
	}

	return retvalUTF8;
}

static std::wstring MBCStoUTF16(const std::string& mbcsString)
{
	// empty
	if (mbcsString.empty())
		return std::wstring();

	// get length of conversion first or see if there is an error
	const int wideLength = ::MultiByteToWideChar(
		CP_ACP, MB_ERR_INVALID_CHARS, mbcsString.data(), (int)mbcsString.length(), // Input
		NULL, 0); // just check
	if (wideLength == 0) // Must be error, as empty string should not happen here anymore
	{
		const DWORD error = ::GetLastError();
		if (error == ERROR_NO_UNICODE_TRANSLATION)
			throw eemagine::sdk::exceptions::incorrectValue("Failure of string conversion from mbcsString, invalid characters: " + mbcsString);

		std::stringstream errormsg;
		errormsg << "Failure to convert mbcsString: " << mbcsString << " Error code: " << error;
		throw eemagine::sdk::exceptions::incorrectValue(errormsg.str());
	}

	// conversion exists! Let's do it
	std::wstring returnStringUTF16;
	returnStringUTF16.resize(wideLength);

	const int convertResult = ::MultiByteToWideChar(
		CP_ACP, 0, mbcsString.data(), (int)mbcsString.length(), // Input
		&returnStringUTF16[0], (int)returnStringUTF16.length()); // fill it

	if (convertResult == 0) // We already verified the string. Not recoverable
	{
		const DWORD error = ::GetLastError();
		std::stringstream errormsg;
		errormsg << "Conversion failed from mbcsString: " << mbcsString << " Error code: " << error;
		throw eemagine::sdk::exceptions::incorrectValue(errormsg.str());
	}

	return returnStringUTF16;
}

static std::wstring UTF8ToUTF16(const std::string& utf8String)
{
	// empty
	if (utf8String.empty())
		return std::wstring();

	// get length of conversion first or see if there is an error
	const int wideLength = ::MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS, utf8String.data(), (int)utf8String.length(), // Input
		NULL, 0); // just check
	if (wideLength == 0) // Must be error, as empty string should not happen here anymore
	{
		const DWORD error = ::GetLastError();
		if (error == ERROR_NO_UNICODE_TRANSLATION)
			throw eemagine::sdk::exceptions::incorrectValue("Failure of string conversion from utf8, invalid characters: " + utf8String);

		std::stringstream errormsg;
		errormsg << "Failure to convert utf8 string: " << utf8String << " Error code: " << error;
		throw eemagine::sdk::exceptions::incorrectValue(errormsg.str());
	}

	// conversion exists! Let's do it
	std::wstring returnStringUTF16;
	returnStringUTF16.resize(wideLength);

	const int convertResult = ::MultiByteToWideChar(
		CP_UTF8, 0, utf8String.data(), (int)utf8String.length(), // Input
		&returnStringUTF16[0], (int)returnStringUTF16.length()); // fill it

	if (convertResult == 0) //
	{
		const DWORD error = ::GetLastError();
		std::stringstream errormsg;
		errormsg << "Conversion failed from utf8: " << utf8String << " Error code: " << error;
		throw eemagine::sdk::exceptions::incorrectValue(errormsg.str());
	}

	return returnStringUTF16;
}

static std::string MBCStoUTF8(const std::string& mbcsString)
{
	return UTF16toUTF8(MBCStoUTF16(mbcsString));
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// String conversion ends here.
///
/// In the following functions we will load the exported functions from the DLL loaded from the path provided by the user.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Helper class for library loading:
// Find funcName in dll specified by libHandle
// If the function can not be found or another error has occured raise runtime_error.
#ifdef _WIN32
typedef HINSTANCE library_handle_type;
template <typename T>
static T getFunc(library_handle_type libHandle, const char* funcName)
{
	// load
	T func = (T)::GetProcAddress(libHandle, funcName);

	// check
	if (func == NULL)
	{
		throw(eemagine::sdk::exceptions::incorrectValue(std::string("Can't load function: ") + funcName));
	}

	return func;
}
library_handle_type
LoadSDKLibrary(const std::string &libPathUtf8) {
	const std::wstring libPathUTF16(UTF8ToUTF16(libPathUtf8));

	library_handle_type oLibHandle = ::LoadLibrary(libPathUTF16.c_str());
	if (oLibHandle == NULL)
	{
		throw(eemagine::sdk::exceptions::incorrectValue("Could not load Library: " + libPathUtf8));
	}
	return oLibHandle;
}
#endif
#ifdef __unix__
typedef void * library_handle_type;
template <typename T>
static T getFunc(library_handle_type libHandle, const char* funcName)
{
	void * symbol(dlsym(libHandle, funcName));
	if (symbol == NULL)
	{
		throw(eemagine::sdk::exceptions::incorrectValue(std::string("Can't load function: ") + funcName));
	}

	return (T)(symbol);
}
library_handle_type
LoadSDKLibrary(const std::string &filename) {
	library_handle_type rv(dlopen(filename.c_str(), RTLD_LAZY));
	if (rv == NULL)
	{
		throw(eemagine::sdk::exceptions::incorrectValue("Could not load Library: " + filename + " error: " + dlerror()));
	}
	return rv;
}
#endif

static void loadLibraryUTF8(const std::string& libPathUTF8, void * data)
{
	// Only load functions once
	if (_sdk.initialized == false)
	{
		// Load DLL and check
		library_handle_type oLibHandle = LoadSDKLibrary(libPathUTF8);

		// Let us load the get version pointer first to have more meaningful error messages
		// if some functions do not exist.
		_sdk.get_version = getFunc<eemagine_sdk_get_version_t>(oLibHandle, "eemagine_sdk_get_version");
		if (_sdk.get_version() != EEGO_SDK_VERSION)
		{
			throw(eemagine::sdk::exceptions::incorrectValue("Eego SDK version mismatch"));
		}


		// Load function pointers from DLL;
		_sdk.setup = getFunc<eemagine_sdk_setup_t>(oLibHandle, "eemagine_sdk_setup");
		_sdk.init = getFunc<eemagine_sdk_init_t>(oLibHandle, "eemagine_sdk_init");
		_sdk.exit = getFunc<eemagine_sdk_exit_t>(oLibHandle, "eemagine_sdk_exit");
		_sdk.get_amplifiers_info = getFunc<eemagine_sdk_get_amplifiers_info_t>(oLibHandle, "eemagine_sdk_get_amplifiers_info");
		_sdk.close_amplifier = getFunc<eemagine_sdk_close_amplifier_t>(oLibHandle, "eemagine_sdk_close_amplifier");
		_sdk.get_amplifier_channel_list = getFunc<eemagine_sdk_get_amplifier_channel_list_t>(oLibHandle, "eemagine_sdk_get_amplifier_channel_list");
		_sdk.get_amplifier_serial = getFunc<eemagine_sdk_get_amplifier_serial_t>(oLibHandle, "eemagine_sdk_get_amplifier_serial");
		_sdk.get_amplifier_version = getFunc<eemagine_sdk_get_amplifier_version_t>(oLibHandle, "eemagine_sdk_get_amplifier_version");
		_sdk.get_amplifier_type = getFunc<eemagine_sdk_get_amplifier_type_t>(oLibHandle, "eemagine_sdk_get_amplifier_type");
		_sdk.get_amplifier_sampling_rates_available = getFunc<eemagine_sdk_get_amplifier_sampling_rates_available_t>(oLibHandle, "eemagine_sdk_get_amplifier_sampling_rates_available");
		_sdk.get_amplifier_reference_ranges_available = getFunc<eemagine_sdk_get_amplifier_reference_ranges_available_t>(oLibHandle, "eemagine_sdk_get_amplifier_reference_ranges_available");
		_sdk.get_amplifier_bipolar_ranges_available = getFunc<eemagine_sdk_get_amplifier_bipolar_ranges_available_t>(oLibHandle, "eemagine_sdk_get_amplifier_bipolar_ranges_available");
		_sdk.open_eeg_stream = getFunc<eemagine_sdk_open_eeg_stream_t>(oLibHandle, "eemagine_sdk_open_eeg_stream");
		_sdk.open_impedance_stream = getFunc<eemagine_sdk_open_impedance_stream_t>(oLibHandle, "eemagine_sdk_open_impedance_stream");
		_sdk.close_stream = getFunc<eemagine_sdk_close_stream_t>(oLibHandle, "eemagine_sdk_close_stream");
		_sdk.get_stream_channel_list = getFunc<eemagine_sdk_get_stream_channel_list_t>(oLibHandle, "eemagine_sdk_get_stream_channel_list");
		_sdk.get_stream_channel_count = getFunc<eemagine_sdk_get_stream_channel_count_t>(oLibHandle, "eemagine_sdk_get_stream_channel_count");
		_sdk.prefetch = getFunc<eemagine_sdk_prefetch_t>(oLibHandle, "eemagine_sdk_prefetch");
		_sdk.get_data = getFunc<eemagine_sdk_get_data_t>(oLibHandle, "eemagine_sdk_get_data");
		_sdk.get_error_string = getFunc<eemagine_sdk_get_error_string_t>(oLibHandle, "eemagine_sdk_get_error_string");
		_sdk.setup(data);
		_sdk.initialized = true;
	}

	// all ok!
}

#ifdef _WIN32
eemagine::sdk::factory::factory(const std::wstring& libPath, void * data)
{
	loadLibraryUTF8(UTF16toUTF8(libPath), data);
}
#endif

eemagine::sdk::factory::factory(const std::string& libPath, void * data)
{
#if defined(_MBCS)
	loadLibraryUTF8(MBCStoUTF8(libPath), data);
#elif defined(_UNICODE)
	loadLibraryUTF8(libPath, data);
#else
#error "Undefined string handling"
#endif
}
#endif

///////////////////////////////////////////////////////////////////////////////
eemagine::sdk::factory::~factory() {
    try {
        _get_sdk_guard_singleton()->sweepAmplifierReferenceCounts();
        _get_sdk_guard_singleton(true);
	}
	catch (const std::exception &e) {
		std::cerr << "exception in ~factory: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "exception in ~factory" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
std::vector<eemagine::sdk::amplifier *>
eemagine::sdk::factory::getAmplifiers() {
	_sdk_guard * sg = _get_sdk_guard_singleton();

	// get list from c layer first
	eemagine_sdk_amplifier_info amplifier_info_array[64];
	int amplifier_count = _sdk.get_amplifiers_info(amplifier_info_array, 64);

	// mark
	for (int amp_id = 0; amp_id < amplifier_count; ++amp_id) {
		sg->incrementAmplifierReferenceCount(amplifier_info_array[amp_id]);
	}

	// sweep
	sg->sweepAmplifierReferenceCounts();

	return sg->getAmplifiers();
}

///////////////////////////////////////////////////////////////////////////////
eemagine::sdk::amplifier *
eemagine::sdk::factory::getAmplifier() {
	eemagine::sdk::amplifier * rv = NULL;
	std::vector<eemagine::sdk::amplifier *> amplifier_list = getAmplifiers();

	if (amplifier_list.empty()) {
		throw(eemagine::sdk::exceptions::notFound("amplifier list is empty"));
	}

	std::vector<eemagine::sdk::amplifier *>::iterator iter = amplifier_list.begin();
	rv = *iter;
	// clean references
	while (++iter != amplifier_list.end()) {
		delete *iter;
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
eemagine::sdk::factory::version
eemagine::sdk::factory::getVersion() const {
	eemagine::sdk::factory::version rv;
	rv.major = EEGO_SDK_VERSION_MAJOR;
	rv.minor = EEGO_SDK_VERSION_MINOR;
	rv.micro = EEGO_SDK_VERSION_MICRO;
	rv.build = EEGO_SDK_VERSION;
	return rv;
}
