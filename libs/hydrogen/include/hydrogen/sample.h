/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef SAMPLE_BUFFER_H
#define SAMPLE_BUFFER_H

#include <string>

#include <hydrogen/globals.h>
#include <hydrogen/Object.h>

namespace H2Core
{

/**
\ingroup H2CORE
*/
class Sample : public Object
{
public:
	Sample( unsigned frames, const std::string& filename, float* data_L = NULL, float* data_R = NULL );
	~Sample();

	float* get_data_l() {
		return __data_l;
	}
	float* get_data_r() {
		return __data_r;
	}

	unsigned get_sample_rate() {
		return __sample_rate;
	}

	const std::string get_filename() {
		return __filename;
	}


	/// Returns the bytes number ( 2 channels )
	unsigned get_size() {
		return __n_frames * sizeof( float ) * 2;
	}

	/// Loads a sample from disk
	static Sample* load( const std::string& filename );

	unsigned get_n_frames() {
		return __n_frames;
	}

private:
	float *__data_l;	///< Left channel data
	float *__data_r;	///< Right channel data

	unsigned __sample_rate;		///< samplerate for this sample
	std::string __filename;		///< filename associated with this sample
	unsigned __n_frames;		///< Total number of frames in this sample.

	//static int __total_used_bytes;

	/// loads a wave file
	static Sample* load_wave( const std::string& filename );

	/// loads a FLAC file
	static Sample* load_flac( const std::string& filename );
};

};

#endif