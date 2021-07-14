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

#ifndef __eemagine_sdk_h__
#define __eemagine_sdk_h__

#ifdef EEGO_SDK_EXPORT
#  ifdef _WIN32
#    define EEGO_SDK_API __declspec(dllexport)
#  endif
#  ifdef __unix__
#    define EEGO_SDK_API __attribute__ ((visibility ("default")))
#  endif
#else // We are importing
#  define EEGO_SDK_API
#endif // #ifdef EEGO_SDK_EXPORT

#ifdef __cplusplus
extern "C" {
#endif
	/**
	 * error codes
	 */
	enum eemagine_sdk_error {
		EEMAGINE_SDK_NOT_CONNECTED = -1,
		EEMAGINE_SDK_ALREADY_EXISTS = -2,
		EEMAGINE_SDK_NOT_FOUND = -3,
		EEMAGINE_SDK_INCORRECT_VALUE = -4,
		EEMAGINE_SDK_UNKNOWN = -5
	};
	/**
	 * @brief structure to support amplifier info
	 */
	typedef struct {
		int      id;
		char     serial[64];
	} eemagine_sdk_amplifier_info;
	/**
	 * @brief channel type enum
	 */
	typedef enum {
		EEMAGINE_SDK_CHANNEL_TYPE_REFERENCE,
		EEMAGINE_SDK_CHANNEL_TYPE_BIPOLAR,
		EEMAGINE_SDK_CHANNEL_TYPE_ACCELEROMETER,
		EEMAGINE_SDK_CHANNEL_TYPE_GYROSCOPE,
		EEMAGINE_SDK_CHANNEL_TYPE_MAGNETOMETER,
		EEMAGINE_SDK_CHANNEL_TYPE_TRIGGER,
		EEMAGINE_SDK_CHANNEL_TYPE_SAMPLE_COUNTER,
		EEMAGINE_SDK_CHANNEL_TYPE_IMPEDANCE_REFERENCE,
		EEMAGINE_SDK_CHANNEL_TYPE_IMPEDANCE_GROUND,
	} eemagine_sdk_channel_type;
	/**
	 * @brief structure to hold channel information
	 */
	typedef struct {
		int                       index;
		eemagine_sdk_channel_type type;
	} eemagine_sdk_channel_info;
	/**
	* @brief setup sdk library
	*/
	typedef void(*eemagine_sdk_setup_t)(void *);
	EEGO_SDK_API void eemagine_sdk_setup(void *);
	/**
	 * @brief initialze sdk library
	 */
	typedef void(*eemagine_sdk_init_t)(void);
	EEGO_SDK_API void eemagine_sdk_init();
	/**
	 * finalize sdk library
	 */
	typedef void(*eemagine_sdk_exit_t)(void);
	EEGO_SDK_API void eemagine_sdk_exit();
	/**
	 * get the current sdk version
	 */
	typedef int(*eemagine_sdk_get_version_t)(void);
	EEGO_SDK_API int eemagine_sdk_get_version();
	/**
	 * @brief get an array of connected amplifiers
	 */
	typedef int(*eemagine_sdk_get_amplifiers_info_t)(eemagine_sdk_amplifier_info * amplifier_info_array, int amplifier_info_array_size);
	EEGO_SDK_API int eemagine_sdk_get_amplifiers_info(eemagine_sdk_amplifier_info * amplifier_info_array, int amplifier_info_array_size);
	/**
	* close amplifier
	*/
	typedef int(*eemagine_sdk_close_amplifier_t)(int amplifier_id);
	EEGO_SDK_API int eemagine_sdk_close_amplifier(int amplifier_id);
	/**
	* @brief get the serial number from the amplifier
	*/
	typedef int(*eemagine_sdk_get_amplifier_serial_t)(int amplifier_id, char * serial, int serial_size);
	EEGO_SDK_API int eemagine_sdk_get_amplifier_serial(int amplifier_id, char * serial, int serial_size);
	/**
	* @brief get the firmware version for this amplifier
	*/
	typedef int(*eemagine_sdk_get_amplifier_version_t)(int amplifier_id);
	EEGO_SDK_API int eemagine_sdk_get_amplifier_version(int amplifier_id);
	/**
	* @brief get the type of the amplifier
	*/
	typedef int(*eemagine_sdk_get_amplifier_type_t)(int amplifier_id, char * type, int type_size);
	EEGO_SDK_API int eemagine_sdk_get_amplifier_type(int amplifier_id, char * type, int type_size);
	/**
	* @brief get the channel list from an amplifier
	*/
	typedef int(*eemagine_sdk_get_amplifier_channel_list_t)(int amplifier_id, eemagine_sdk_channel_info * channel_info_array, int channel_info_array_size);
	EEGO_SDK_API int eemagine_sdk_get_amplifier_channel_list(int amplifier_id, eemagine_sdk_channel_info * channel_info_array, int channel_info_array_size);
	/**
	* @brief get a list of sampling rates that are available
	*/
	typedef int(*eemagine_sdk_get_amplifier_sampling_rates_available_t)(int amplifier_id, int * sampling_rate_array, int sampling_rate_array_size);
	EEGO_SDK_API int eemagine_sdk_get_amplifier_sampling_rates_available(int amplifier_id, int * sampling_rate_array, int sampling_rate_array_size);
	/**
	* @brief get a list of reference ranges that are available
	*/
	typedef int(*eemagine_sdk_get_amplifier_reference_ranges_available_t)(int amplifier_id, double * reference_range_array, int reference_range_array_size);
	EEGO_SDK_API int eemagine_sdk_get_amplifier_reference_ranges_available(int amplifier_id, double * reference_range_array, int reference_range_array_size);
	/**
	* @brief get a list of bipolar ranges that are available
	*/
	typedef int(*eemagine_sdk_get_amplifier_bipolar_ranges_available_t)(int amplifier_id, double * bipolar_range_array, int bipolar_range_array_size);
	EEGO_SDK_API int eemagine_sdk_get_amplifier_bipolar_ranges_available(int amplifier_id, double * bipolar_range_array, int bipolar_range_array_size);
	/**
	 * open eeg stream, returns handle on success. if handle is negative, it indicates an error
	 */
	typedef int(*eemagine_sdk_open_eeg_stream_t)(int amplifier_id, int sampling_rate, double reference_range, double bipolar_range, unsigned long long ref_mask, unsigned long long bip_mask);
	EEGO_SDK_API int eemagine_sdk_open_eeg_stream(int amplifier_id, int sampling_rate, double reference_range, double bipolar_range, unsigned long long ref_mask, unsigned long long bip_mask);
	/**
	* open impedance stream, returns handle on success. if handle is negative, it indicates an error
	*/
	typedef int(*eemagine_sdk_open_impedance_stream_t)(int amplifier_id, unsigned long long ref_mask);
	EEGO_SDK_API int eemagine_sdk_open_impedance_stream(int amplifier_id, unsigned long long ref_mask);
	/**
	 * close stream
	 */
	typedef int(*eemagine_sdk_close_stream_t)(int stream_id);
	EEGO_SDK_API int eemagine_sdk_close_stream(int stream_id);
	/**
	* @brief get the channel list from a stream
	*/
	typedef int(*eemagine_sdk_get_stream_channel_list_t)(int stream_id, eemagine_sdk_channel_info * channel_info_array, int channel_info_array_size);
	EEGO_SDK_API int eemagine_sdk_get_stream_channel_list(int stream_id, eemagine_sdk_channel_info * channel_info_array, int channel_info_array_size);
	/**
	* @brief get the channel count from a stream
	*/
	typedef int(*eemagine_sdk_get_stream_channel_count_t)(int stream_id);
	EEGO_SDK_API int eemagine_sdk_get_stream_channel_count(int stream_id);
	/**
	 * @brief prefetch will return the number of bytes the next call to get_data will need;
	 */
	typedef int(*eemagine_sdk_prefetch_t)(int stream_id);
	EEGO_SDK_API int eemagine_sdk_prefetch(int stream_id);
	/**
	 * get data
	 */
	typedef int(*eemagine_sdk_get_data_t)(int stream_id, double *buffer, int buffer_size_in_bytes);
	EEGO_SDK_API int eemagine_sdk_get_data(int stream_id, double *buffer, int buffer_size_in_bytes);
	/**
	* get error string
	*/
	typedef int(*eemagine_sdk_get_error_string_t)(char *error_string, int error_string_size);
	EEGO_SDK_API int eemagine_sdk_get_error_string(char *error_string, int error_string_size);

#ifdef __cplusplus
}
#endif

#endif
