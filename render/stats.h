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
		\brief Declares CqStats class for holding global renderer statistics information.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef STATS_H_INCLUDED
#define STATS_H_INCLUDED 1

#include  <time.h>
#include  <iostream>
#include	"ri.h"

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \enum EqState
 * Current process identifiers.
 */

enum EqState
{
    State_Parsing,    		///< Parsing a RIB file.
    State_Shadows,    		///< Processing shadows.
    State_Rendering,    	///< Rendering image.
    State_Complete,    		///< Rendering complete.
};


//----------------------------------------------------------------------
/** \class CqStatTimer
 * Class to handle timing of rendering processes as intervals.
 */
class CqStatTimer
{
	public:
		CqStatTimer() : m_timeStart( 0 ), m_timeTotal( 0 ), m_cStarted( 0 )
		{}
		~CqStatTimer()
		{}


		/** Get the total time that ths timer has recorded.
		 */
		clock_t	TimeTotal() const
		{
			return ( m_timeTotal );
		}

		/** Start the timer, asserts that the timer has not already been started, nesting is not allowed.
		 */
		void	Start()
		{
			if ( m_cStarted == 0 )
				m_timeStart = clock();
			m_cStarted++;
		}

		/** Stop the timer and update the total time for this timer. Asserts that the timer was actually running.
		 */
		void	Stop()
		{
			assert( m_cStarted > 0 );
			m_cStarted--;
			if ( m_cStarted == 0 )
				m_timeTotal += clock() - m_timeStart;
		}

		/** Reset the total time for this timer, asserts that the timer is not running.
		 */
		void	Reset()
		{
			m_timeTotal = 0;
			m_cStarted = 0;
		}

		/** Check if this timer is started.
		 */
		TqBool	isStarted()
		{
			return( m_cStarted > 0 );
		}

		/** Add two timers together
		 */
		CqStatTimer& operator+=( const CqStatTimer& from )
		{
			m_timeTotal += from.m_timeTotal;
			return(*this);
		}

	private:
		clock_t	m_timeStart;		///< Time at which the current interval started.
		clock_t	m_timeTotal;		///< Total time recorded by this timer.
		TqInt	m_cStarted;		///< Count of how many times the timer has been started.
}
;


//----------------------------------------------------------------------
/** \class CqStats
   Class containing statistics information.
 
   Before a rendering session the method Initialise() has to be called
	 (it is also called by the constructor). Before each individual frame 
	 the variables have to be reset by calling InitialiseFrame().
	 After that the counters can be increased by calling the appropriate
	 IncXyz()-Method. To measure various times there are several pairs
	 of StartXyzTimer() and StopXyZTimer() methods.
	 The statistics for each frame can be printed with PrintStats().
 */

class CqStats
{
	public:
		CqStats()
		{
			Initialise();
		}

		~CqStats()
		{ }

		void Initialise();
		void InitialiseFrame();

		/** Get the process identifier.
		 */
		EqState	State() const
		{
			return ( m_State );
		}
		/** Set the current process.
		 */
		void	SetState( const EqState State )
		{
			m_State = State;
		}

		/** Get the percentage complete.
		 */
		TqFloat	Complete() const
		{
			return ( m_Complete );
		}
		/** Set the percentage complete.
		 */
		void	SetComplete( TqFloat complete )
		{
			m_Complete = complete;
		}

		/// \name Increasing counters
		//@{

		/** Increase the micropolygons allocated count by 1.
		 */
		void	IncMPGsAllocated()
		{
			m_cMPGsAllocated++;
			m_cMPGsCurrent++;
			m_cMPGsPeak = ( m_cMPGsCurrent > m_cMPGsPeak ) ? m_cMPGsCurrent : m_cMPGsPeak;
		}
		/** Increase the micropolygons deallocated count by 1.
		 */
		void	IncMPGsDeallocated()
		{
			m_cMPGsDeallocated++;
			m_cMPGsCurrent--;
		}
		/** Increase the number of micropolygons pushed forward.
		 */
		void	IncMPGsPushedForward()
		{
			m_cMPGsPushedForward++;
		}
		/** Increase the number of micropolygons pushed down.
		 */
		void	IncMPGsPushedDown()
		{
			m_cMPGsPushedDown++;
		}
		/** Increase the number of micropolygons pushed down more than 1 row.
		 */
		void	IncMPGsPushedFarDown()
		{
			m_cMPGsPushedFarDown++;
		}
		/** Increase the sample count by 1.
		 */
		void	IncSamples()
		{
			m_cSamples++;
		}
		/** Increase the sample bound hit count by 1.
		 */
		void	IncSampleBoundHits()
		{
			m_cSampleBoundHits++;
		}
		/** Increase the sample hit count by 1.
		 */
		void	IncSampleHits()
		{
			m_cSampleHits++;
		}
		/** Increase the shader variables allocated count by 1.
		 */
		void	IncVariablesAllocated()
		{
			m_cVariablesAllocated++;
			m_cVariablesCurrent++;
			m_cVariablesPeak = ( m_cVariablesCurrent > m_cVariablesPeak ) ? m_cVariablesCurrent : m_cVariablesPeak;
		}
		/** Increase the shader variables deallocated count by 1.
		 */
		void	IncVariablesDeallocated()
		{
			m_cVariablesDeallocated++;
			m_cVariablesCurrent--;
		}
		/** Increase the surface parameters allocated count by 1.
		 */
		void	IncParametersAllocated()
		{
			m_cParametersAllocated++;
			m_cParametersCurrent++;
			m_cParametersPeak = ( m_cParametersCurrent > m_cParametersPeak ) ? m_cParametersCurrent : m_cParametersPeak;
		}
		/** Increase the surface parameters deallocated count by 1.
		 */
		void	IncParametersDeallocated()
		{
			m_cParametersDeallocated++;
			m_cParametersCurrent--;
		}
		/** Increase the micropolygrids allocated count by 1.
		 */
		void	IncGridsAllocated()
		{
			m_cGridsAllocated++;
			m_cGridsCurrent++;
			m_cGridsPeak = ( m_cGridsCurrent > m_cGridsPeak ) ? m_cGridsCurrent : m_cGridsPeak;
		}
		/** Increase the micropolygrids deallocated count by 1.
		 */
		void	IncGridsDeallocated()
		{
			m_cGridsDeallocated++;
			m_cGridsCurrent--;
		}
		/** Increase the GPrim count by 1.
		    These counter should only be increased when adding a toplevel gprim
				(e.g. in the Ri...() calls).
		 */
		void	IncGPrims()
		{
			m_cGPrims++;
		}
		/** Increase the total GPrim count by 1.
		    Here \em every gprim is counted (including those resulting from a split).
		 */
		void	IncTotalGPrims()
		{
			m_cTotalGPrims++;
		}
		/** Decrease the total GPrim count by 1.
		    This method should be called when a gprim is split since the
				gprim will be replaced by a number of smaller gprims and therefore 
				shouldn't appear in the statistics.
		 */
		void	DecTotalGPrims()
		{
			m_cTotalGPrims--;
		}
		/** Increase the culled GPrim count by 1.
		 */
		void	IncCulledGPrims()
		{
			m_cCulledGPrims++;
		}
		/** Increase the culled micropoly grid count by 1.
		 */
		void	IncCulledGrids()
		{
			m_cCulledGrids++;
		}
		/** Increase the culled micropoly count by n.
		    This counter should only be increased if the according grid
				was \em not culled.
		 */
		void	IncCulledMPGs( TqInt n = 1 )
		{
			m_cCulledMPGs += n;
		}
		/** Increase the missed MPG count by 1.
		 */
		void	IncMissedMPGs()
		{
			m_cMissedMPGs++;
		}

		/** Increase the texture memory used.
		 */
		void	IncTextureMemory( TqInt n = 0 )
		{
			m_cTextureMemory += n;
		}
		void IncTextureHits( TqInt primary, TqInt which )
		{
			m_cTextureHits[ primary ][ which ] ++;
		}
		void IncTextureMisses( TqInt which )
		{
			m_cTextureMisses[ which ] ++;
		}

		/** Get the texture memory used.
		 */
		TqInt GetTextureMemory()
		{
			return m_cTextureMemory;
		}


		/** Start the current frame timer.
		 */
		void StartFrameTimer();

		/** Start the current frame timer.
		 */
		void StopFrameTimer();

		/** Get the surface timer.
		*/
		CqStatTimer& SurfaceTimer()
		{
			return ( m_timeSurface );
		};

		/** Get the displacement timer.
		*/
		CqStatTimer& DisplacementTimer()
		{
			return ( m_timeDisplacement );
		};
		/** Get the Imager timer.
		   	 */
		CqStatTimer& ImagerTimer()
		{
			return ( m_timeImager );
		};

		/** Get the atmosphere timer.
		*/
		CqStatTimer& AtmosphereTimer()
		{
			return ( m_timeAtmosphere );
		};

		/** Get the splits timer.
		*/
		CqStatTimer& SplitsTimer()
		{
			return ( m_timeSplits );
		};

		/** Get the dicing timer.
		*/
		CqStatTimer& DicingTimer()
		{
			return ( m_timeDicing );
		};
		/** Get the texturesampling timer.
		*/
		CqStatTimer& TextureMapTimer()
		{
			return ( m_timeTM );
		};
		/** Get the render MPGs timer.
		*/
		CqStatTimer& RenderMPGsTimer()
		{
			return ( m_timeRenderMPGs );
		};
		/** Get the occlusion culling timer.
		*/
		CqStatTimer& OcclusionCullTimer()
		{
			return ( m_timeOcclusionCull );
		};
		/** Get the diceable timer.
		*/
		CqStatTimer& DiceableTimer()
		{
			return ( m_timeDiceable );
		};
		/** Get the MakeTextureV, MakeShadowV, ... timers
		*/
		CqStatTimer& MakeTextureTimer()
		{
			return ( m_timeMakeTexture );
		};
		CqStatTimer& MakeShadowTimer()
		{
			return ( m_timeMakeShadow );
		};
		CqStatTimer& MakeEnvTimer()
		{
			return ( m_timeMakeEnv );
		};
		CqStatTimer& MakeFilterBucket()
		{
			return ( m_timeFB );
		};
		CqStatTimer& MakeDisplayBucket()
		{
			return ( m_timeDB );
		};
		CqStatTimer& MakeCombine()
		{
			return ( m_timeCombine );
		};
		CqStatTimer& MakeParse()
		{
			return ( m_timeParse );
		};
		CqStatTimer& MakeProject()
		{
			return ( m_timeProject );
		};
		CqStatTimer& Others()
		{
			return ( m_timeOthers );
		};

		TqInt GetCurrentMPGsAllocated() const
		{
			return ( m_cMPGsCurrent );
		}
		TqInt GetMPGsAllocated() const
		{
			return ( m_cMPGsAllocated );
		}
		TqInt GetMPGsDeallocated() const
		{
			return ( m_cMPGsDeallocated );
		}

		TqInt GetCurrentGridsAllocated() const
		{
			return ( m_cGridsCurrent );
		}
		TqInt GetGridsAllocated() const
		{
			return ( m_cGridsAllocated );
		}
		TqInt GetGridsDeallocated() const
		{
			return ( m_cGridsDeallocated );
		}

		TqInt GetCurrentParametersAllocated() const
		{
			return ( m_cParametersCurrent );
		}
		TqInt GetParametersAllocated() const
		{
			return ( m_cParametersAllocated );
		}
		TqInt GetParametersDeallocated() const
		{
			return ( m_cParametersDeallocated );
		}

		//@}

		void PrintStats( TqInt level ) const;
		void PrintInfo() const;

	private:
		std::ostream& TimeToString( std::ostream& os, TqFloat t, TqFloat tot ) const;

	private:
		EqState	m_State;						///< Current process identifier.
		TqFloat	m_Complete;						///< Current percentage comlpete.

		TqInt	m_cMPGsAllocated;				///< Count of micropolygons allocated.
		TqInt	m_cMPGsDeallocated;				///< Count of microplygons dallocated.
		TqInt	m_cMPGsCurrent;					///< Current count of allocated MPGs.
		TqInt	m_cMPGsPeak;					///< Peak count of allocated MPGs.
		TqInt	m_cMPGsPushedForward;			///< Number of micropolygons pushed forward in the bucket list.
		TqInt	m_cMPGsPushedDown;				///< Number of micropolygons pushed down in the bucket list.
		TqInt	m_cMPGsPushedFarDown;			///< Number of micropolygons pushed down additional rows in the bucket list.
		TqInt	m_cSamples;						///< Count of samples tested.
		TqInt	m_cSampleBoundHits;				///< Count of sample boundary hits.
		TqInt	m_cSampleHits;					///< Count of sample micropolygon hits.
		TqInt	m_cVariablesAllocated;			///< Count of shader variables allocated.
		TqInt	m_cVariablesDeallocated;		///< Count of shader variables deallocated.
		TqInt	m_cVariablesCurrent;			///< Current count of variables allocated.
		TqInt	m_cVariablesPeak;				///< Peak count of variables allocated.
		TqInt	m_cParametersAllocated;			///< Count of surface parameters allocated.
		TqInt	m_cParametersDeallocated;		///< Count of surface parameters deallocated.
		TqInt	m_cParametersCurrent;			///< Current count of parameters allocated.
		TqInt	m_cParametersPeak;				///< Peak count of parameters allocated.
		TqInt	m_cGridsAllocated;				///< Count of micropolygrids allocated.
		TqInt	m_cGridsDeallocated;			///< Count of micropolygrids deallocated.
		TqInt	m_cGridsCurrent;				///< Current count of grids allocated.
		TqInt	m_cGridsPeak;					///< Peak count of grids allocated.
		TqInt	m_cGPrims;						///< Count of GPrims.

		TqInt	m_cTotalGPrims;					///< Count of total GPrims (including gprims resulting from splits).
		TqInt	m_cCulledGPrims;				///< Count of culled GPrims.
		TqInt	m_cCulledGrids;					///< Count of culled micro poly grids.
		TqInt	m_cCulledMPGs;					///< Count of culled micro polys.
		TqInt	m_cMissedMPGs;				///< Count of missed MPGs.
		TqInt m_cTextureMemory;     ///< Count of the memory used by texturemap.cpp
		TqInt m_cTextureHits[ 2 ][ 5 ];     ///< Count of the hits encountered used by texturemap.cpp
		TqInt m_cTextureMisses[ 5 ];     ///< Count of the hits encountered used by texturemap.cpp

		CqStatTimer	m_timeTotal;				///< Total time spent on the entire animation.
		CqStatTimer m_timeTotalFrame;			///< Time spent on processing one individual frame.

		CqStatTimer m_timeSurface;				///< Time spent on surface shading.
		CqStatTimer m_timeImager;			///< Time spent on imager shading.
		CqStatTimer m_timeDisplacement;			///< Time spent on displacement shading.
		CqStatTimer m_timeAtmosphere;			///< Time spent on volume shading (atmosphere).
		CqStatTimer m_timeSplits;				///< Time spent on surface splitting.
		CqStatTimer m_timeDicing;				///< Time spent on surface dicing.
		CqStatTimer m_timeRenderMPGs;			///< Time spent on rendering MPGs.
		CqStatTimer m_timeOcclusionCull;		///< Time spent on occlusion culling.
		CqStatTimer m_timeDiceable;				///< Time spent on diceable checking.
		CqStatTimer m_timeTM;				///< Time spent on SampleMipMap checking.
		CqStatTimer m_timeMakeTexture;			///< Time spent on MakeTextureV call.
		CqStatTimer m_timeMakeShadow;			///< Time spent on MakeShadowV call.
		CqStatTimer m_timeMakeEnv;		    	///< Time spent on MakeCubeEnvironmenV call.
		CqStatTimer m_timeFB;		    	    ///< Time spent on Filter the Bucket call.
		CqStatTimer m_timeDB;		    	    ///< Time spent on sending the Bucket information to the display.
		CqStatTimer m_timeCombine;		    ///< Time spent on combining the bucket subpixels
		CqStatTimer m_timeParse;		    ///< Time spent on sending the parsing RIB file or Ri Calls
		CqStatTimer m_timeProject;		    ///< Time spent on sending the Project grids to raster space
		CqStatTimer m_timeOthers;		    ///< Time spent on init. the buckets

}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !STATS_H_INCLUDED
