// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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


/** \file
		\brief The base class from which all piqsl images derive.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is ddclient.h included already?
#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED 1

#include	<vector>
#include	<string>
#include	<map>

#include	<boost/shared_ptr.hpp>
#include	<boost/function.hpp>
#include	<boost/thread/mutex.hpp>

#include	"aqsis.h"
#include	"sstring.h"
#include 	"tinyxml.h"

START_NAMESPACE( Aqsis )

class CqFramebuffer;

//---------------------------------------------------------------------
/** \class CqImage
 * Abstract base class for piqsl images.
 */

class CqImage
{
public:
	CqImage( const CqString& name ) : m_name(name), m_data(0), m_realData(0), m_frameWidth(0), m_frameHeight(0), m_imageWidth(0), m_imageHeight(0), m_originX(0), m_originY(0), m_elementSize(0)
	{} 
	CqImage() : m_data(0), m_realData(0), m_frameWidth(0), m_frameHeight(0), m_imageWidth(0), m_imageHeight(0), m_originX(0), m_originY(0), m_elementSize(0)
	{}
    virtual ~CqImage();

	/** Get the name of the image.
	 * \return			The name of the image.
	 */
    virtual CqString&	name()
    {
        return ( m_name );
    }
	/** Set the display name of the image.
	 * \param name		The new display name of the image.
	 */
    virtual void	setName( const CqString& name )
    {
        m_name = name;
    }
	/** Get the filename of the image.
	 * \return			The filename of the image.
	 */
    virtual CqString&	filename()
    {
        return ( m_fileName );
    }
	/** Set the filename of the image.
	 * \param name		The new filename of the image.
	 */
    virtual void	setFilename( const CqString& name )
    {
        m_fileName = name;
    }
	/** Get the frame width of the image.
	 * The frame width if the cropped rendered region, within the image.
	 * \return			The frame width of the image.
	 */
	virtual TqUlong	frameWidth()
	{
		return( m_frameWidth );
	}
	/** Get the frame height of the image.
	 * The frame height if the cropped rendered region, within the image.
	 * \return			The frame height of the image.
	 */
	virtual TqUlong frameHeight()
	{
		return( m_frameHeight );
	}
	/** Combined setter for frame size.
	 * \param width			The new frame width.
	 * \param height		The new frame height.
	 */
	virtual void setFrameSize(TqUlong width, TqUlong height)
	{
		m_frameWidth = width;
		m_frameHeight = height;
	}
	/** Get the name of the channel at the specified index.
	 * \param index			The index of the channel to query.
	 * \return				A string representation of the channel name.
	 */
	virtual const std::string& channelName(TqInt index)
	{
		assert(index < numChannels());
		return(m_channels[index].first);
	}
	/** Set the name of the indexed channel.
	 * \param index			The index of the channel to rename.
	 * \param name			The name to set the indexed channel to.
	 */
	virtual void setChannelName(TqInt index, const std::string& name)
	{
		assert(index < numChannels());
		m_channels[index].first = name;
	}
	/** Get the type of the indexed channel.
	 * See ndspy.h for details of the supported types.
	 * \param index			The index of the channel to query.
	 * \return				The type of the channel, as a PkDspy type.
	 */
	virtual TqInt channelType(TqInt index)
	{
		assert(index < numChannels());
		return(m_channels[index].second);
	}
	/** Set the type of the indexed channel.
	 * See ndspy.h for details of the supported types.
	 * \param index			The index of the channel to change.
	 * \param type			The type to set the indexed channel to.
	 */
	virtual void setChannelType(TqInt index, TqInt type)
	{
		assert(index < numChannels());
		m_channels[index].second = type;
	}
	/** Get the number of channels in this image.
	 * \return				The number of channels.
	 */
	virtual TqInt numChannels() const
	{
		return( m_channels.size() );
	}
	/** Add a channel to the end of the current list.
	 * \param name			The name to use for the new channel.
	 * \param type			The PkDspy type to use, see ndspy.h for supported types.
	 */
	virtual void addChannel(const std::string& name, TqInt type)
	{
		m_channels.push_back(std::pair<std::string, TqInt>(name,type));
	}
	/** Get the calculated size of a single pixel in the image.
	 * Calculated from the number of channels and their types.
	 * \return				The total number of bytes in a single image pixel.
	 */
	virtual TqInt elementSize() const
	{
		return(m_elementSize);
	}
	/** Get the calculated length of a complete row of the image in bytes.
	 * \return				The number of bytes in a complete row of the image.
	 */
	virtual TqInt rowLength() const
	{
		return(m_elementSize * m_imageWidth);
	}
	/** Get a pointer to the display data.
	 * The display buffer is simple 8 bits per channel data as displayable by an RGB image.
	 * \return				A pointer to the start of the display buffer.
	 */
	virtual unsigned char* data()
	{
		return( m_data );
	}
	/** Get the origin of the cropped frame within the total image.
	 * \return				The origin of the frame.
	 */
	virtual TqUlong originX() const
	{
		return( m_originX );
	}
	/** Get the origin of the cropped frame within the total image.
	 * \return				The origin of the frame.
	 */
	virtual TqUlong originY() const
	{
		return( m_originY );
	}
	/** The the origin of the frame within the image.
	 * \param originx		The x origin within the image of the rendered frame.
	 * \param originy		The y origin within the image of the rendered frame.
	 */
	virtual void setOrigin(TqUlong originX, TqUlong originY)
	{
		m_originX = originX;
		m_originY = originY;
	}
	/** Get the total width of the image.
	 * \return			The total width of the image.
	 */
	virtual TqUlong imageWidth() const
	{
		return( m_imageWidth );
	}
	/** Get the total height of the image.
	 * \return			The total height of the image.
	 */
	virtual TqUlong imageHeight() const
	{
		return( m_imageHeight );
	}
	/** Set the total image size.
	 * \param imageWidth	The total image width.
	 * \param imageHeight	The total image height.
	 */
	virtual void setImageSize(TqUlong imageWidth, TqUlong imageHeight)
	{
		m_imageWidth = imageWidth;
		m_imageHeight = imageHeight;
	}

	/** Setup the display and real buffers.
	 * Presuming the size and channels have been setup, allocate the two buffers used by the image.
	 */
	virtual void PrepareImageBuffer();
	
	/** Setup a callback to be called when the image changes.
	 * \param f			A function that will be called with the region that has changed.
	 */
	virtual void setUpdateCallback(boost::function<void(int,int,int,int)> f);

	/** Save the image to the given folder.
	 * \note Overridden by derivations that manage their image data differently.
	 */
	virtual void serialise(const std::string& folder)
	{}
	/** Create an XML element representing this image for serialising to library files.
	 * \return			A pointer to a new TinyXML element structure.
	 */
	virtual TiXmlElement* serialiseToXML();
	/** Save the image to a TIFF file.
	 * \param filename	The name of the file to save the image into.
	 */
	void saveToTiff(const std::string& filename);
	/** Load the image from a TIFF file on disk.
	 * \param filename	The name of the TIFF file to load the image data from.
	 */
	void loadFromTiff(const std::string& filename);

	/** Transfer the data from the real buffer to thd display buffer.
	 * This takes into accound the format of the channels in the real data and will attempt to make a 
	 * good guess at the intended quantisation for 8bit display.
	 */
	void transferData();

	/** Get a reference to the unique mutex for this image.
	 * Used when locking the image during multithreaded operation.
	 * \return			A reference to the unique mutex for this image.
	 */
	boost::mutex& mutex()
	{
		return(m_mutex);
	}

protected:
    CqString		m_name;			///< Display name.
    CqString		m_fileName;		///< File name.
	unsigned char*	m_data;			///< Buffer to store the 8bit data for display. 
	unsigned char*	m_realData;		///< Buffer to store the natural format image data.
	TqUlong			m_frameWidth;	///< The width of the frame within the whole image.
	TqUlong			m_frameHeight;	///< The height of the frame within the whole image.
	TqUlong			m_imageWidth;	///< The total image width.
	TqUlong			m_imageHeight;	///< The total image height.
	TqUlong			m_originX;		///< The origin of the frame within the whole image.
	TqUlong			m_originY;		///< The origin of the frame within the whole image.
	TqInt			m_elementSize;	///< The calcualated total size of a single pixel.
	std::vector<std::pair<std::string, TqInt> >			m_channels;	///< An array of channels, name and type.

	boost::function<void(int,int,int,int)> m_updateCallback;	///< A callback, called when an image changes.
	boost::mutex	m_mutex;		///< The unique mutex for this image.
};

END_NAMESPACE( Aqsis )

#endif	// IMAGE_H_INCLUDED
