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

#ifndef __eemagine_sdk_buffer_h__
#define __eemagine_sdk_buffer_h__

// system
#include <string>
#include <vector>
// eemagine
#include <eemagine/sdk/exceptions.h>

namespace eemagine {
	namespace sdk {
		/// Wrapper around array to provide indexed access to values
		class buffer {
		public:
			/**
			 * \brief default constructor
			 */
			buffer(unsigned int channel_count = 0, unsigned int sample_count = 0)
				: _data(channel_count * sample_count)
				, _channel_count(channel_count)
				, _sample_count(sample_count)
			{
			}
			/**
			* \brief get the number of channels
			* @return number of channels
			*/
			const unsigned int& getChannelCount() const
			{
				return _channel_count;
			}

			/**
			* \brief get the number of samples
			* @return number of samples
			*/
			const unsigned int& getSampleCount() const
			{
				return _sample_count;
			}
			/**
			* \brief get sample value
			* @param channel the channel index(start indexing at zero)
			* @param sample the sample index(start indexing at zero)
			* @return returns that value for channel at sample
			*/
			const double& getSample(unsigned int channel, unsigned int sample) const
			{
				if (channel >= _channel_count || sample >= _sample_count) {
					throw(eemagine::sdk::exceptions::incorrectValue("invalid getSample index"));
				}
				return _data[channel + sample * _channel_count];
			}
			/**
			* \brief get number of samples
			* @return number of samples
			*/
			size_t size() const { return _data.size(); }
			/**
			 * \brief direct pointer to data
			 */
			double * data() {
				return _data.data();
			}
		protected:
			std::vector < double > _data;
			unsigned int           _channel_count;
			unsigned int           _sample_count;
		};
	}
}

#endif
