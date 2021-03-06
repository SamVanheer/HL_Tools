#pragma once

#include <glm/vec2.hpp>

#include "graphics/OpenGL.hpp"

/**
*	@file sprite.h Sprite file and memory definitions
*/

/**
*	@defgroup Sprite 2D Sprite
*
*	<pre>
*	Sprite file format:
*	dsprite_t header
*	If 16 bit mode is specified:
*		short paletteColors = 256
*		byte palette[ paletteColors * 3 ]
*	Endif
*	For( Range( 0 -> header.numframes ) )
*		spriteframetype_t frameType
*		If frameType == SINGLE:
*			dspriteframe_t frame
*			byte pixels[ frame.width * frame.height ]
*		Else (GROUP):
*			dspritegroup_t group
*			dspriteinterval_t intervals[ group.numframes ]
*			{
*				dspriteframe_t frame
*				byte pixels[ frame.width * frame.height ]
*			} [ group.numframes ]
*		Endif
*	EndFor
*	</pre>
*
*	@{
*/

namespace sprite
{
namespace Type
{
/**
*	Sprite orientation type.
*/
enum Type : int
{
	FIRST					= 0,

	/**
	*	The sprite faces upward. The sprite's right vector is parallel to the viewer's viewplane.
	*/
	VP_PARALLEL_UPRIGHT		= FIRST,

	/**
	*	The sprite faces upward. The sprite's right vector is perpendicular to the entity's origin.
	*/
	FACING_UPRIGHT			= 1,

	/**
	*	The sprite always faces the viewer. When the viewer moves, the sprite rotates to face the viewer.
	*/
	VP_PARALLEL				= 2,

	/**
	*	The sprite is oriented according to the entity's angles.
	*/
	ORIENTED				= 3,

	/**
	*	The sprite faces the viewer, but rotates in the axis that points to the viewer.
	*/
	VP_PARALLEL_ORIENTED	= 4,

	LAST = VP_PARALLEL_ORIENTED,

	COUNT
};
}

/**
*	Returns the string representation of the given type.
*	@param type Type.
*	@return String representation.
*/
const char* TypeToString( const Type::Type type );

/**
*	Attempts to convert a string to a type.
*	@param pszString String. Must be non-null.
*	@param pbSuccess If not null, contains whether the conversion succeeded or not.
*	@return Type. Only valid if pbSuccess is true.
*/
Type::Type StringToType( const char* const pszString, bool* pbSuccess );

namespace TexFormat
{
/**
*	Texture format.
*/
enum TexFormat : int
{
	FIRST			= 0,

	/**
	*	No transparency or translucency.
	*	This sprite will appear as a rectangle in the world, animating with whatever design you create it with.
	*/
	SPR_NORMAL		 = FIRST,

	/**
	*	Looks best for explosions, steam, lasers, or other gaseous or light effect.
	*	It acts as a brightness map in the scene, brightening whatever appears behind it.
	*	What this amounts to is a white pixel in your sprite is essentially totally opaque and white, and black pixel becomes transparent,
	*	and every value in between is appropriately translucent, modulo the background behind the sprite.
	*	The lighting of the surrounding area will not directly affect the lighting of this sprite,
	*	so in extremely dark areas the sprite may appear to be self-illuminating (which is often desirable, especially for explosions and lens flares).
	*	The brighter the background behind the sprite, the brighter the sprite will be.
	*/
	SPR_ADDITIVE	 = 1,

	/**
	*	Similar to Additive in that it has varying grades of opacity from totally opaque to totally transparent,
	*	but the blending within the world is done differently and the opacity is controlled not by the value of a given pixel,
	*	but by the palette register of the color of that pixel.
	*	Any pixel within the sprite that is colored by the first color in your palette will be transparent,
	*	and any pixel within the sprite that is colored by the last color in the palette will be opaque.
	*/
	SPR_INDEXALPHA	 = 2,

	/**
	*	This is a totally opaque sprite with one key color that is invisible.
	*	This sort of sprite can look jaggy and less realistic than Additive or Indexalpha sprites,
	*	but it renders significantly faster than either of those two, and so can be very useful for situations where speed is more important than appearance.
	*	For instance, if you are doing sprites for a multiplayer mod,
	*	you might choose to use Alphatest sprites for explosion effects even though they look worse than Additive,
	*	because you don't want the framerate to get too bad for any given player who is seeing a lot of sprites on their screen.
	*	The appearance of Alphatest sprites is similar to that of Masked Textures used on brushmodels,
	*	excepting that Masked Textures respond to the light level of their situations while Alphatest sprites do not.
	*/
	SPR_ALPHTEST	 = 3,

	LAST = SPR_ALPHTEST,

	COUNT
};
}

/**
*	Converts a texture format to its string representation.
*	@param format Texture format.
*	@return String representation.
*/
const char* TexFormatToString( const TexFormat::TexFormat format );

/**
*	Attempts to convert a string to a texture format.
*	@param pszString String. Must be non-null.
*	@param pbSuccess If not null, contains whether the conversion succeeded or not.
*	@return Texture format. Only valid if pbSuccess is true.
*/
TexFormat::TexFormat StringToTexFormat( const char* const pszString, bool* pbSuccess );

/**
*	The type of a frame.
*/
enum class spriteframetype_t
{
	/**
	*	A single frame. This type of frame will animate at the given framerate.
	*/
	SINGLE = 0,

	/**
	*	A frame that is part of a group. An interval specifies when to show the frame.
	*/
	GROUP
};

enum
{
	/**
	*	The maximum dimension that a texture can be in width and height.
	*/
	MAX_SPRITE_TEXTURE_DIMS = 256
};

/**
*	Sprite sync type.
*/
enum class synctype_t
{
	/**
	*	Animate in sync with all other sprites.
	*/
	SYNC = 0,

	/**
	*	Use random start frame.
	*/
	RAND
};

/**
*	@defgroup Sprite_File Sprite file type definitions.
*
*	@{
*/

/**
*	Sprite header for disk storage.
*/
struct dsprite_t final
{
	/**
	*	File type identifier. Must be SPRITE_ID.
	*	@see SPRITE_ID
	*/
	int ident;

	/**
	*	File version. Must be SPRITE_VERSION.
	*	@see SPRITE_VERSION
	*/
	int version;

	/**
	*	Sprite orientation type.
	*/
	Type::Type type;

	/**
	*	Texture format. This indicates how to interpret the sprite's data.
	*/
	TexFormat::TexFormat texFormat;

	/**
	*	Radius of a sphere that covers the sprite. This sphere touches the edges of the sprite at the middle of the widest side.
	*	<pre>
	*	e.g. a sprite that is 2 x 16 has a bounding radius of:
	*	sqrt( ( ( 2 / 2 ) * ( 2 / 2 ) ) + ( ( 16 / 2 ) * ( 16 / 2 ) ) )
	*	=> sqrt( ( 1 * 1 ) + ( 8 * 8 ) )
	*	=> sqrt( 1 + 64 )
	*	=> sqrt( 65 )
	*	=> 8.06
	*	</pre>
	*/
	float boundingradius;

	/**
	*	Maximum sprite frame width. Is a multiple of 8.
	*/
	int	width;

	/**
	*	Maximum sprite frame height. Is a multiple of 8.
	*/
	int height;

	/**
	*	Number of frames in this sprite.
	*/
	int numframes;

	/**
	*	Obsolete. Used in Quake for sprite rotation.
	*/
	float beamlength;

	/**
	*	Sprite synchronization type.
	*	Automatic animation (torches, etc) can be either all together or randomized.
	*/
	synctype_t synctype;
};

/**
*	A single sprite frame.
*/
struct dspriteframe_t final
{
	/**
	*	Coordinates of the upper left point in the texture where the sprite starts.
	*/
	glm::ivec2 origin;

	/**
	*	Width of this frame. Is a multiple of 8.
	*/
	int width;

	/**
	*	Height of this frame. Is a multiple of 8.
	*/
	int height;
};

/**
*	Sprite groups allow a sequence of sprites to have a variable framerate by specifying intervals at which they should be shown.
*/
struct dspritegroup_t final
{
	/**
	*	The number of frames in this group.
	*/
	int numframes;
};

/**
*	Specifies the interval for a frame in a group.
*/
struct dspriteinterval_t final
{
	/**
	*	Interval at which the associated frame should be shown.
	*/
	float interval;
};

/** @} */

/**
*	@defgroup Sprite_Memory Sprite memory type definitions.
*
*	@{
*/

/**
*	A single sprite frame.
*/
struct mspriteframe_t final
{
	/**
	*	Width of this frame. Is a multiple of 8.
	*/
	int		width;

	/**
	*	Height of this frame. Is a multiple of 8.
	*/
	int		height;

	/**
	*	Texture coordinates for this frame. Range [0, 1].
	*/
	float	up, down, left, right;

	/**
	*	OpenGL texture ID.
	*/
	GLuint	gl_texturenum;
};

/**
*	A single sprite frame group.
*/
struct mspritegroup_t final
{
	/**
	*	Number of frames in this group.
	*/
	int numframes;

	/**
	*	Pointer to the array of intervals.
	*/
	float* intervals;

	float GetInterval( const size_t uiIndex ) const { return intervals[ uiIndex ]; }

	/**
	*	Array of frames. Has numframes elements.
	*	@see numframes
	*/
	mspriteframe_t* frames[ 1 ];

	const	mspriteframe_t* GetFrame( const size_t uiIndex ) const	{ return frames[ uiIndex ]; }
			mspriteframe_t* GetFrame( const size_t uiIndex )		{ return frames[ uiIndex ]; }
};

/**
*	Contains a frame, and the type of that frame.
*/
struct mspriteframedesc_t final
{
	/**
	*	The type of the frame.
	*/
	spriteframetype_t type;

	/**
	*	Pointer to the frame itself. If type == spriteframetype_t::GROUP, this points to an mspritegroup_t instead.
	*/
	mspriteframe_t* frameptr;

	const	mspriteframe_t* GetFrame() const	{ return frameptr; }
			mspriteframe_t* GetFrame()			{ return frameptr; }

	const	mspritegroup_t* GetGroup() const	{ return reinterpret_cast<const mspritegroup_t*>( frameptr ); }
			mspritegroup_t* GetGroup()			{ return reinterpret_cast<mspritegroup_t*>( frameptr ); }
};

/**
*	A single sprite.
*/
struct msprite_t final
{
	/**
	*	The type of the sprite.
	*/
	Type::Type type;

	/**
	*	The texture format used by the sprite.
	*/
	TexFormat::TexFormat texFormat;

	/**
	*	Largest width that any frame in this sprite has.
	*/
	int maxwidth;

	/**
	*	Largest height that any frame in this sprite has.
	*/
	int maxheight;

	/**
	*	The number of frames in this sprite.
	*/
	int numframes;

	/**
	*	Obsolete.
	*	@see dsprite_t::beamlength
	*/
	float beamlength;	

	/**
	*	Obsolete.
	*/
	void* cachespot;

	/**
	*	Array of frame descriptors. Has numframes elements.
	*	@see numframes
	*/
	mspriteframedesc_t frames[ 1 ];

	const	mspriteframedesc_t* GetFrameDescriptor( const size_t uiIndex ) const	{ return &frames[ uiIndex ]; }
			mspriteframedesc_t* GetFrameDescriptor( const size_t uiIndex )			{ return &frames[ uiIndex ]; }
};

/** @} */
}

/**
*	Sprite file format version.
*/
#define SPRITE_VERSION 2

/**
*	Little-endian "IDSP"
*/
#define SPRITE_ID (('P'<<24)+('S'<<16)+('D'<<8)+'I')

/** @} */
