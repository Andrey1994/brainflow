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

#ifndef __eemagine_sdk_factory_h__
#define __eemagine_sdk_factory_h__

// system
#include <vector>
// eemagine
#include <eemagine/sdk/amplifier.h>

// It has to be either dynamic or static
#if !(defined(EEGO_SDK_BIND_DYNAMIC) ^ defined(EEGO_SDK_BIND_STATIC))
#pragma message ("Neither EEGO_SDK_BIND_DYNAMIC nor EEGO_SDK_BIND_STATIC has been defined. Default to EEGO_SDK_BIND_DYNAMIC.")
#define EEGO_SDK_BIND_DYNAMIC
#endif

// make sure that some string encoding is defined. Be it either UNICODE or MBCS or UTF8. It currently is only valid for the single input string
#if !(defined(_UNICODE) ^ defined(_MBCS))
#pragma message ("Neither _UNICODE nor _MBCS has been defined. Default to _UNICODE.")
#define _UNICODE
#endif


namespace eemagine {
	namespace sdk {
		/// entry point for the Eemagine SDK. The factory builds amplifiers
		class factory {
		public:
			///////////////////////////////////////////////////////////////////////////////
			/**
			 * constructor
			 */
#ifdef EEGO_SDK_BIND_STATIC
			factory(void * data = NULL);
#else
#ifdef _WIN32
			factory(const std::wstring& path, void * data = NULL);
#endif
			factory(const std::string& path, void * data = NULL);
#endif // EEGO_SDK_BIND_STATIC

			/**
			 * destructor
			 */
			~factory();
			///////////////////////////////////////////////////////////////////////////////
			/**
			 * Returns a pointer to all available amplifiers.
			 * caller is responsible to delete the pointers.
			 * @return vector of pointers to amplifiers
			 */
			std::vector<eemagine::sdk::amplifier *> getAmplifiers();
			///////////////////////////////////////////////////////////////////////////////
			/**
			* Returns a pointer to the first connected amplifier.
			* caller is responsible to delete the pointer.
			* this function may throw an eemagine::sdk::exceptions::notFound exception
			* if no amplifiers are found.
			* @return pointer to an available amplifier
			*/
			eemagine::sdk::amplifier * getAmplifier();
			///////////////////////////////////////////////////////////////////////////////
			/**
			 * Get version information
			 */
			struct version {
				int major;
				int minor;
				int micro;
				int build;
			};
			version getVersion() const;
		};
	}
}

#endif
