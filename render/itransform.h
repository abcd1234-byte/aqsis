//------------------------------------------------------------------------------
/**
 *	@file	itransform.h
 *	@author	Authors name
 *	@brief	Brief description of the file contents
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/10/31 11:51:12 $
 */ 
//------------------------------------------------------------------------------
#ifndef	___itransform_Loaded___
#define	___itransform_Loaded___

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

struct IqTransform
{
	virtual	~IqTransform()
	{}

	/** Get a writable copy of this, if the reference count is greater than 1
	 * create a new copy and retirn that.
	 */
	virtual void	SetCurrentTransform( TqFloat time, const CqMatrix& matTrans ) = 0;;
	virtual	void	ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans ) = 0;
	virtual	const CqMatrix&	matObjectToWorld( TqFloat time = 0.0f ) const = 0;

	virtual	void	Release() = 0;
	virtual	void	AddRef() = 0;
};




END_NAMESPACE( Aqsis )


#endif	//	___itransform_Loaded___
