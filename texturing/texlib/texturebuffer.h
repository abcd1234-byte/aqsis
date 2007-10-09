// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/**
 * \file
 *
 * \brief Declare a texture buffer class and associated machinery.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */


#ifndef TEXTUREBUFFER_H_INCLUDED
#define TEXTUREBUFFER_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_array.hpp>

#include "smartptr.h"
#include "samplevector.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A class to hold the data from a single texture tile.
 *
 * This class is a container for tiles of texture data for which each pixel
 * holds the same number of channels as every other pixel.
 */
template<typename T>
class CqTextureBuffer : public CqIntrusivePtrCounted
{
	public:
		/** \brief The pointer type for use with texture tiles.
		 *
		 * We use intrusive_ptr here rather than shared_ptr since it's lightweight for
		 * the construction of CqSampleVector objects.
		 */
		typedef boost::intrusive_ptr<CqTextureBuffer<T> > TqPtr;

		/** \brief Construct a texture tile
		 *
		 * \todo Investigate the use of an allocator or such for the TIFF data.
		 *
		 * \param width - tile width
		 * \param height - tile height
		 * \param channelsPerPixel - number of channels of type T per pixel.
		 */
		CqTextureBuffer(const TqInt width, const TqInt height,
				const TqInt channelsPerPixel);

		/** \brief Resize the buffer
		 *
		 * \param width  - new width
		 * \param height - new height
		 * \param bytesPerPixel - number of bytes per pixel
		 */
		virtual void resize(TqInt width, TqInt height, TqInt bytesPerPixel);

		/// Get a pointer to the underlying raw data
		inline TqUchar* rawDataPtr();

		/** \brief Get a reference to the tile channels at the given position.
		 *
		 * Positions are in image coordinates, not tile coordinates, counting
		 * from zero in the top-left
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \return a lightweight vector holding a reference to the channels data
		 */
		inline const CqSampleVector<T> operator()(const TqInt x, const TqInt y) const;

		/// Get the buffer width
		inline TqInt width() const;

		/// Get the buffer height
		inline TqInt height() const;

		/// Get the number of channels per pixel
		inline TqInt channelsPerPixel() const;
	private:
		/** \brief Get a pointer to the sample at the given coordinates.
		 *
		 * Positions are in image coordinates, not tile coordinates, counting
		 * from zero in the top-left
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 */
		inline T* samplePtr(TqInt x, TqInt y) const;

		boost::shared_array<T> m_pixelData;	///< Pointer to the underlying pixel data.
		TqInt m_width;				///< Width of the buffer
		TqInt m_height;				///< Height of the buffer
		TqInt m_channelsPerPixel;	///< Number of channels per pixel
};



//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

// Inline functions/templates for CqTextureBuffer

template<typename T>
CqTextureBuffer<T>::CqTextureBuffer(TqInt width, TqInt height, TqInt channelsPerPixel)
	: m_pixelData(new T[width * height * channelsPerPixel]),
	m_width(width),
	m_height(height),
	m_channelsPerPixel(channelsPerPixel)
{ }

template<typename T>
inline const CqSampleVector<T> CqTextureBuffer<T>::operator()(const TqInt x, const TqInt y) const
{
	return CqSampleVector<T>(samplePtr(x,y),
			boost::intrusive_ptr<const CqIntrusivePtrCounted>(this));
}

template<typename T>
void CqTextureBuffer<T>::resize(TqInt width, TqInt height, TqInt bytesPerPixel)
{
	if(bytesPerPixel % sizeof(T) != 0)
		throw XqInternal("Cannot resize buffer to required number of bytes per pixel",
				__FILE__, __LINE__);
	TqInt channelsPerPixel = bytesPerPixel/sizeof(T);
	TqInt newSize = width * height * channelsPerPixel;
	if(newSize != m_width * m_height * m_channelsPerPixel);
		m_pixelData.reset(new T[newSize]);
	// Set new buffer sizes
	m_width = width;
	m_height = height;
	m_channelsPerPixel = channelsPerPixel;
}

template<typename T>
inline TqUchar* CqTextureBuffer<T>::rawDataPtr()
{
	return reinterpret_cast<TqUchar*>(m_pixelData.get());
}

template<typename T>
inline TqInt CqTextureBuffer<T>::width() const
{
	return m_width;
}

template<typename T>
inline TqInt CqTextureBuffer<T>::height() const
{
	return m_height;
}

template<typename T>
inline TqInt CqTextureBuffer<T>::channelsPerPixel() const
{
	return m_channelsPerPixel;
}

template<typename T>
inline T* CqTextureBuffer<T>::samplePtr(TqInt x, TqInt y) const
{
	assert(x >= 0);
	assert(x < m_width);
	assert(y >= 0);
	assert(y < m_height);
	return m_pixelData.get() + (y*m_width + x)*m_channelsPerPixel;
}


} // namespace Aqsis

#endif // TEXTUREBUFFER_H_INCLUDED
