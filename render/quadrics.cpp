
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
		\brief Implements the standard RenderMan quadric primitive classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"quadrics.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"nurbs.h"

#include	"ri.h"

START_NAMESPACE( Aqsis )

#pragma warning(push, 3)

static TqBool IntersectLine( CqVector3D& P1, CqVector3D& T1, CqVector3D& P2, CqVector3D& T2, CqVector3D& P );
static void ProjectToLine( const CqVector3D& S, const CqVector3D& Trj, const CqVector3D& pnt, CqVector3D& p );

#define TOOLARGEQUADS 10000

CqQuadric::CqQuadric()
{
	m_uDiceSize = m_vDiceSize = 0;

}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqQuadric&	CqQuadric::operator=( const CqQuadric& From )
{
	CqSurface::operator=( From );
	m_matTx = From.m_matTx;
	m_matITTx = From.m_matITTx;
	m_uDiceSize = From.m_uDiceSize;
	m_vDiceSize = From.m_vDiceSize;

	return ( *this );
}


//---------------------------------------------------------------------
/** Transform the quadric primitive by the specified matrix.
 */

void	CqQuadric::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime )
{
	m_matTx *= matTx;
	m_matITTx *= matITTx;
}


//---------------------------------------------------------------------
/** Dice the quadric filling in as much information on the grid as possible.
 */

TqInt CqQuadric::DiceAll( CqMicroPolyGrid* pGrid )
{
	TqInt lUses = Uses();
	TqInt lDone = 0;

	CqVector3D	P, N;
	int v, u;

	CqParameterTypedVarying<TqFloat, type_float, TqFloat>* ps = s();
	CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pt = t();
	CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pu = this->u();
	CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pv = this->v();

	TqFloat s0,s1,s2,s3;
	if( USES( lUses, EnvVars_s ) && NULL != pGrid->s() && bHass() )
	{
		s0 = ps->pValue( 0 )[0];
		s1 = ps->pValue( 1 )[0];
		s2 = ps->pValue( 2 )[0];
		s3 = ps->pValue( 3 )[0];

		DONE( lDone, EnvVars_s );
	}

	TqFloat t0,t1,t2,t3;
	if( USES( lUses, EnvVars_t ) && NULL != pGrid->t() && bHast() )
	{
		t0 = pt->pValue( 0 )[0];
		t1 = pt->pValue( 1 )[0];
		t2 = pt->pValue( 2 )[0];
		t3 = pt->pValue( 3 )[0];

		DONE( lDone, EnvVars_t );
	}

	TqFloat u0,u1,u2,u3;
	if( USES( lUses, EnvVars_u ) && NULL != pGrid->u() && bHasu() )
	{
		u0 = pu->pValue( 0 )[0];
		u1 = pu->pValue( 1 )[0];
		u2 = pu->pValue( 2 )[0];
		u3 = pu->pValue( 3 )[0];

		DONE( lDone, EnvVars_u );
	}

	TqFloat v0,v1,v2,v3;
	if( USES( lUses, EnvVars_v ) && NULL != pGrid->v() && bHasv() )
	{
		v0 = pv->pValue( 0 )[0];
		v1 = pv->pValue( 1 )[0];
		v2 = pv->pValue( 2 )[0];
		v3 = pv->pValue( 3 )[0];

		DONE( lDone, EnvVars_v );
	}

	if( USES( lUses, EnvVars_P ) && NULL != pGrid->P() )
		DONE( lDone, EnvVars_P );
	if( USES( lUses, EnvVars_Ng ) && NULL != pGrid->Ng() )
		DONE( lDone, EnvVars_Ng );

	TqFloat du = 1.0 / uDiceSize();
	TqFloat dv = 1.0 / vDiceSize();
	for ( v = 0; v <= vDiceSize(); v++ )
	{
		TqFloat vf = v * dv;
		for ( u = 0; u <= uDiceSize(); u++ )
		{
			TqFloat uf = u * du;
			TqInt igrid = ( v * ( uDiceSize() + 1 ) ) + u;
			if( USES( lUses, EnvVars_P ) && NULL != pGrid->P() )
			{
				if( USES( lUses, EnvVars_Ng ) && NULL != pGrid->Ng() )
				{
					P = DicePoint( u, v, N );
					pGrid->P()->SetPoint( m_matTx * P, igrid );
					pGrid->Ng()->SetNormal( m_matITTx * N, igrid );

				}
				else
				{
					P = DicePoint( u, v );
					pGrid->P()->SetPoint( m_matTx * P, igrid );
				}
			}
			if( USES( lUses, EnvVars_s ) && NULL != pGrid->s() && bHass() )
			{
				TqFloat _s = BilinearEvaluate( s0, s1, s2, s3, uf, vf );
				pGrid->s()->SetFloat( _s, igrid );
			}
			if( USES( lUses, EnvVars_t ) && NULL != pGrid->t() && bHast() )
			{
				TqFloat _t = BilinearEvaluate( t0, t1, t2, t3, uf, vf );
				pGrid->t()->SetFloat( _t, igrid );
			}
			if( USES( lUses, EnvVars_u ) && NULL != pGrid->u() && bHasu() )
			{
				TqFloat _u = BilinearEvaluate( u0, u1, u2, u3, uf, vf );
				pGrid->u()->SetFloat( _u, igrid );
			}
			if( USES( lUses, EnvVars_v ) && NULL != pGrid->v() && bHasv() )
			{
				TqFloat _v = BilinearEvaluate( v0, v1, v2, v3, uf, vf );
				pGrid->v()->SetFloat( _v, igrid );
			}
		}
	}
	return( lDone );
}

//---------------------------------------------------------------------
/** Dice the quadric into a grid of MPGs for rendering.
 */

void CqQuadric::NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData )
{
	// Special case for "P", else normal bilinear dice for all others.

	TqLong hash = CqParameter::hash(pData->strName().c_str());
	if ( hash == CqParameter::hash("P") )
	{
		CqVector3D	P;
		int v, u;
		for ( v = 0; v <= vDiceSize; v++ )
		{
			for ( u = 0; u <= uDiceSize; u++ )
			{
				TqInt igrid = ( v * ( uDiceSize + 1 ) ) + u;
				P = DicePoint( u, v );
				pData->SetPoint( m_matTx * P, igrid );
			}
		}
	}
	else
		CqSurface::NaturalDice( pParameter, uDiceSize, vDiceSize, pData );
}

//---------------------------------------------------------------------
/** Generate and store the geometric normals for this quadric.
 */

void CqQuadric::GenerateGeometricNormals( TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pNormals )
{
	int v, u;
	CqVector3D	N;
	for ( v = 0; v <= vDiceSize; v++ )
	{
		for ( u = 0; u <= uDiceSize; u++ )
		{
			TqInt igrid = ( v * ( uDiceSize + 1 ) ) + u;
			DicePoint( u, v, N );
			TqInt O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ];
			N = ( O == OrientationLH ) ? N : -N;
			pNormals->SetNormal( m_matITTx * N, igrid );
		}
	}
}


//---------------------------------------------------------------------
/** Determine whether the quadric is suitable for dicing.
 */


TqBool	CqQuadric::Diceable()
{
	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( TqFalse );

	TqInt toomuch = EstimateGridSize();

	m_SplitDir = ( m_uDiceSize > m_vDiceSize ) ? SplitDir_U : SplitDir_V;

	TqFloat gs = SqrtGridSize();

	if (toomuch > TOOLARGEQUADS) return TqFalse;

	if ( m_uDiceSize > gs) return TqFalse;
	if ( m_vDiceSize > gs) return TqFalse;
	

	return ( TqTrue );

}


//---------------------------------------------------------------------
/** Estimate the size of the micropolygrid required to dice this GPrim to a suitable shading rate.
 */

TqUlong CqQuadric::EstimateGridSize()
{
	TqFloat maxusize, maxvsize;
	maxusize = maxvsize = 0;

	CqMatrix matTx = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" ) * m_matTx;

	m_uDiceSize = m_vDiceSize = ESTIMATEGRIDSIZE;

	TqFloat udist, vdist;
	CqVector3D p, pum1, pvm1[ ESTIMATEGRIDSIZE ];

	int v, u;
	for ( v = 0; v <= ESTIMATEGRIDSIZE; v++ )
	{
		for ( u = 0; u <= ESTIMATEGRIDSIZE; u++ )
		{
			p = DicePoint( u, v );
			p = matTx * p;
			// If we are on row two or above, calculate the mp size.
			if ( v >= 1 && u >= 1 )
			{
				udist = ( p.x() - pum1.x() ) * ( p.x() - pum1.x() ) +
				        ( p.y() - pum1.y() ) * ( p.y() - pum1.y() );
				vdist = ( pvm1[ u - 1 ].x() - pum1.x() ) * ( pvm1[ u - 1 ].x() - pum1.x() ) +
				        ( pvm1[ u - 1 ].y() - pum1.y() ) * ( pvm1[ u - 1 ].y() - pum1.y() );

				maxusize = MAX( maxusize, udist );
				maxvsize = MAX( maxvsize, vdist );
			}
			if ( u >= 1 ) pvm1[ u - 1 ] = pum1;
			pum1 = p;
		}
	}
	maxusize = sqrt( maxusize );
	maxvsize = sqrt( maxvsize );

	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRateSqrt" ) [ 0 ];

	m_uDiceSize = ROUND( ESTIMATEGRIDSIZE * maxusize / ( ShadingRate ) );
	m_vDiceSize = ROUND( ESTIMATEGRIDSIZE * maxvsize / ( ShadingRate ) );

	// Ensure power of 2 to avoid cracking
	const TqInt *binary = pAttributes() ->GetIntegerAttribute( "dice", "binary" );
	if ( binary && *binary)
	{
		m_uDiceSize = CEIL_POW2( m_uDiceSize );
		m_vDiceSize = CEIL_POW2( m_vDiceSize );
	}

    return  (TqUlong) m_uDiceSize * m_vDiceSize;
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqSphere::CqSphere( TqFloat radius, TqFloat zmin, TqFloat zmax, TqFloat thetamin, TqFloat thetamax ) :
		m_Radius( radius ),
		m_ZMin( zmin ),
		m_ZMax( zmax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSphere&	CqSphere::operator=( const CqSphere& From )
{
	CqQuadric::operator=( From );
	m_Radius = From.m_Radius;
	m_ZMin = From.m_ZMin;
	m_ZMax = From.m_ZMax;
	m_ThetaMin = From.m_ThetaMin;
	m_ThetaMax = From.m_ThetaMax;

	return ( *this );
}

//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqSphere::Bound() const
{
	TqFloat phimin = ( m_ZMin > -m_Radius ) ? asin( m_ZMin / m_Radius ) : -( RI_PIO2 );
	TqFloat phimax = ( m_ZMax < m_Radius ) ? asin( m_ZMax / m_Radius ) : ( RI_PIO2 );

	std::vector<CqVector3D> curve;
	CqVector3D vA( 0, 0, 0 ), vB( 1, 0, 0 ), vC( 0, 0, 1 );
	Circle( vA, vB, vC, m_Radius, phimin, phimax, curve );

	CqMatrix matRot( RAD ( m_ThetaMin ), vC );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );

	CqBound	B( RevolveForBound( curve, vA, vC, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqSphere::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	TqFloat zcent = ( m_ZMin + m_ZMax ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;

	CqSphere* pNew1 = new CqSphere( *this );
	CqSphere* pNew2 = new CqSphere( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
	}
	else
	{
		pNew1->m_ZMax = zcent;
		pNew2->m_ZMin = zcent;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqSphere::DicePoint( TqInt u, TqInt v )
{
	TqFloat phimin = ( m_ZMin > -m_Radius ) ? asin( m_ZMin / m_Radius ) : RAD( -90.0 );
	TqFloat phimax = ( m_ZMax < m_Radius ) ? asin( m_ZMax / m_Radius ) : RAD( 90.0 );
	TqFloat phi = phimin + ( ( TqFloat ) v * ( phimax - phimin ) ) / m_vDiceSize;

	TqFloat cosphi = cos( phi );
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );

	return ( CqVector3D( ( m_Radius * cos( theta ) * cosphi ), ( m_Radius * sin( theta ) * cosphi ), ( m_Radius * sin( phi ) ) ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqSphere::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	CqVector3D	p( DicePoint( u, v ) );
	Normal = p;
	Normal.Unit();
	return ( p );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqCone::CqCone( TqFloat height, TqFloat radius, TqFloat thetamin, TqFloat thetamax, TqFloat zmin, TqFloat zmax ) :
		m_Height( height ),
		m_Radius( radius ),
		m_ZMin( zmin ),
		m_ZMax( zmax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqCone&	CqCone::operator=( const CqCone& From )
{
	CqQuadric::operator=( From );
	m_Height = From.m_Height;
	m_Radius = From.m_Radius;
	m_ZMin = From.m_ZMin;
	m_ZMax = From.m_ZMax;
	m_ThetaMin = From.m_ThetaMin;
	m_ThetaMax = From.m_ThetaMax;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqCone::Bound() const
{
	std::vector<CqVector3D> curve;
	CqVector3D vA( m_Radius, 0, m_ZMin ), vB( 0, 0, m_ZMax ), vC( 0, 0, 0 ), vD( 0, 0, 1 );
	curve.push_back( vA );
	curve.push_back( vB );
	CqMatrix matRot( RAD ( m_ThetaMin ), vD );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vC, vD, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqCone::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	TqFloat zcent = ( m_ZMin + m_ZMax ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;
	//TqFloat rcent=m_RMax*sqrt(zcent/m_ZMax);

	CqCone* pNew1 = new CqCone( *this );
	CqCone* pNew2 = new CqCone( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
	}
	else
	{
		pNew1->m_ZMax = zcent;
		pNew2->m_ZMin = zcent;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqCone::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );

	TqFloat z = m_ZMin + ( ( TqFloat ) v * ( m_ZMax - m_ZMin ) ) / m_vDiceSize;
	TqFloat r = m_Radius * ( 1.0 - z / m_Height );

	return ( CqVector3D( r * cos( theta ), r * sin( theta ), z ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqCone::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	return ( DicePoint( u, v ) );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqCylinder::CqCylinder( TqFloat radius, TqFloat zmin, TqFloat zmax, TqFloat thetamin, TqFloat thetamax ) :
		m_Radius( radius ),
		m_ZMin( zmin ),
		m_ZMax( zmax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqCylinder&	CqCylinder::operator=( const CqCylinder& From )
{
	CqQuadric::operator=( From );
	m_Radius = From.m_Radius;
	m_ZMin = From.m_ZMin;
	m_ZMax = From.m_ZMax;
	m_ThetaMin = From.m_ThetaMin;
	m_ThetaMax = From.m_ThetaMax;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqCylinder::Bound() const
{
	std::vector<CqVector3D> curve;
	CqVector3D vA( m_Radius, 0, m_ZMin ), vB( m_Radius, 0, m_ZMax ), vC( 0, 0, 0 ), vD( 0, 0, 1 );
	curve.push_back( vA );
	curve.push_back( vB );
	CqMatrix matRot( RAD ( m_ThetaMin ), vD );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vC, vD, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqCylinder::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	TqFloat zcent = ( m_ZMin + m_ZMax ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;

	CqCylinder* pNew1 = new CqCylinder( *this );
	CqCylinder* pNew2 = new CqCylinder( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
	}
	else
	{
		pNew1->m_ZMax = zcent;
		pNew2->m_ZMin = zcent;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqCylinder::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( m_ThetaMax - m_ThetaMin ) * ( TqFloat ) u ) / m_uDiceSize );

	TqFloat vz = m_ZMin + ( ( TqFloat ) v * ( m_ZMax - m_ZMin ) ) / m_vDiceSize;
	return ( CqVector3D( m_Radius * cos( theta ), m_Radius * sin( theta ), vz ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqCylinder::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	CqVector3D	p( DicePoint( u, v ) );
	Normal = p;
	Normal.z( 0 );
	Normal.Unit();

	return ( p );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqHyperboloid::CqHyperboloid( )
{
	m_Point1 = CqVector3D( 0.0f, 0.0f, 0.0f );
	m_Point2 = CqVector3D( 0.0f, 0.0f, 1.0f );
	m_ThetaMin = 0.0f;
	m_ThetaMax = 1.0f;
}

//---------------------------------------------------------------------
/** Constructor.
 */

CqHyperboloid::CqHyperboloid( CqVector3D& point1, CqVector3D& point2, TqFloat thetamin, TqFloat thetamax ) :
		m_Point1( point1 ),
		m_Point2( point2 ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqHyperboloid&	CqHyperboloid::operator=( const CqHyperboloid& From )
{
	CqQuadric::operator=( From );
	m_Point1 = From.m_Point1;
	m_Point2 = From.m_Point2;
	m_ThetaMin = From.m_ThetaMin;
	m_ThetaMax = From.m_ThetaMax;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqHyperboloid::Bound() const
{
	std::vector<CqVector3D> curve;
	curve.push_back( m_Point1 );
	curve.push_back( m_Point2 );
	CqVector3D vA( 0, 0, 0 ), vB( 0, 0, 1 );
	CqMatrix matRot( RAD ( m_ThetaMin ), vB );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vA, vB, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqHyperboloid::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;
	CqVector3D midpoint = ( m_Point1 + m_Point2 ) / 2.0;

	CqHyperboloid* pNew1 = new CqHyperboloid( *this );
	CqHyperboloid* pNew2 = new CqHyperboloid( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
	}
	else
	{
		pNew1->m_Point2 = midpoint;
		pNew2->m_Point1 = midpoint;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqHyperboloid::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );

	CqVector3D p;
	TqFloat vv = static_cast<TqFloat>( v ) / m_vDiceSize;
	p = m_Point1 * ( 1.0 - vv ) + m_Point2 * vv;

	return ( CqVector3D( p.x() * cos( theta ) - p.y() * sin( theta ), p.x() * sin( theta ) + p.y() * cos( theta ), p.z() ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqHyperboloid::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	return ( DicePoint( u, v ) );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqParaboloid::CqParaboloid( TqFloat rmax, TqFloat zmin, TqFloat zmax, TqFloat thetamin, TqFloat thetamax ) :
		m_RMax( rmax ),
		m_ZMin( zmin ),
		m_ZMax( zmax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqParaboloid&	CqParaboloid::operator=( const CqParaboloid& From )
{
	CqQuadric::operator=( From );
	m_RMax = From.m_RMax;
	m_ZMin = From.m_ZMin;
	m_ZMax = From.m_ZMax;
	m_ThetaMin = From.m_ThetaMin;
	m_ThetaMax = From.m_ThetaMax;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqParaboloid::Bound() const
{
	/*	TqFloat xminang,yminang,xmaxang,ymaxang;
		xminang=yminang=MIN(m_ThetaMin,m_ThetaMax);
		xmaxang=ymaxang=MAX(m_ThetaMin,m_ThetaMax);
	 
	 
		// If start and end in same segement, just use the points.
		if(static_cast<TqInt>(m_ThetaMin/90)!=static_cast<TqInt>(m_ThetaMax/90))
		{
			if(yminang<90 && ymaxang>90)	yminang=90;
			if(yminang<270 && ymaxang>270)	ymaxang=270;
			if(xminang<180 && xmaxang>180)	xmaxang=180;
		}*/

	TqFloat x1 = m_RMax * cos( RAD( 0 ) );
	TqFloat x2 = m_RMax * cos( RAD( 180 ) );
	TqFloat y1 = m_RMax * sin( RAD( 90 ) );
	TqFloat y2 = m_RMax * sin( RAD( 270 ) );

	CqVector3D vecMin( MIN( x1, x2 ), MIN( y1, y2 ), MIN( m_ZMin, m_ZMax ) );
	CqVector3D vecMax( MAX( x1, x2 ), MAX( y1, y2 ), MAX( m_ZMin, m_ZMax ) );

	CqBound	B( vecMin, vecMax );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into smaller quadrics.
 */

TqInt CqParaboloid::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	TqFloat zcent = ( m_ZMin + m_ZMax ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;
	TqFloat rcent = m_RMax * sqrt( zcent / m_ZMax );

	CqParaboloid* pNew1 = new CqParaboloid( *this );
	CqParaboloid* pNew2 = new CqParaboloid( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
	}
	else
	{
		pNew1->m_ZMax = zcent;
		pNew1->m_RMax = rcent;
		pNew2->m_ZMin = zcent;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqParaboloid::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( m_ThetaMax - m_ThetaMin ) * ( TqFloat ) u ) / m_uDiceSize );

	TqFloat z = m_ZMin + ( ( TqFloat ) v * ( m_ZMax - m_ZMin ) ) / m_vDiceSize;
	TqFloat r = m_RMax * sqrt( z / m_ZMax );
	return ( CqVector3D( r * cos( theta ), r * sin( theta ), z ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqParaboloid::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	return ( DicePoint( u, v ) );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqTorus::CqTorus( TqFloat majorradius, TqFloat minorradius, TqFloat phimin, TqFloat phimax, TqFloat thetamin, TqFloat thetamax ) :
		m_MajorRadius( majorradius ),
		m_MinorRadius( minorradius ),
		m_PhiMin( phimin ),
		m_PhiMax( phimax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqTorus&	CqTorus::operator=( const CqTorus& From )
{
	CqQuadric::operator=( From );
	m_MajorRadius = From.m_MajorRadius;
	m_MinorRadius = From.m_MinorRadius;
	m_PhiMax = From.m_PhiMax;
	m_PhiMin = From.m_PhiMin;
	m_ThetaMin = From.m_ThetaMin;
	m_ThetaMax = From.m_ThetaMax;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqTorus::Bound() const
{
	std::vector<CqVector3D> curve;
	CqVector3D vA( m_MajorRadius, 0, 0 ), vB( 1, 0, 0 ), vC( 0, 0, 1 ), vD( 0, 0, 0 );
	Circle( vA, vB, vC, m_MinorRadius, RAD( m_PhiMin ), RAD( m_PhiMax ), curve );
	CqMatrix matRot( RAD ( m_ThetaMin ), vC );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vD, vC, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqTorus::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	TqFloat zcent = ( m_PhiMax + m_PhiMin ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;

	CqTorus* pNew1 = new CqTorus( *this );
	CqTorus* pNew2 = new CqTorus( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
	}
	else
	{
		pNew1->m_PhiMax = zcent;
		pNew2->m_PhiMin = zcent;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqTorus::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );
	TqFloat phi = RAD( m_PhiMin + ( ( TqFloat ) v * ( m_PhiMax - m_PhiMin ) ) / m_vDiceSize );

	TqFloat r = m_MinorRadius * cos( phi );
	TqFloat z = m_MinorRadius * sin( phi );
	return ( CqVector3D( ( m_MajorRadius + r ) * cos( theta ), ( m_MajorRadius + r ) * sin( theta ), z ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqTorus::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	return ( DicePoint( u, v ) );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqDisk::CqDisk( TqFloat height, TqFloat minorradius, TqFloat majorradius, TqFloat thetamin, TqFloat thetamax ) :
		m_Height( height ),
		m_MajorRadius( majorradius ),
		m_MinorRadius( minorradius ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqDisk&	CqDisk::operator=( const CqDisk& From )
{
	CqQuadric::operator=( From );
	m_Height = From.m_Height;
	m_MajorRadius = From.m_MajorRadius;
	m_MinorRadius = From.m_MinorRadius;
	m_ThetaMin = From.m_ThetaMin;
	m_ThetaMax = From.m_ThetaMax;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqDisk::Bound() const
{
	std::vector<CqVector3D> curve;
	CqVector3D vA( m_MajorRadius, 0, m_Height ), vB( m_MinorRadius, 0, m_Height ), vC( 0, 0, 0 ), vD( 0, 0, 1 );
	curve.push_back( vA );
	curve.push_back( vB );
	CqMatrix matRot( RAD ( m_ThetaMin ), vD );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vC, vD, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqDisk::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	TqFloat zcent = ( m_MajorRadius + m_MinorRadius ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;

	CqDisk* pNew1 = new CqDisk( *this );
	CqDisk* pNew2 = new CqDisk( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
	}
	else
	{
		pNew1->m_MinorRadius = zcent;
		pNew2->m_MajorRadius = zcent;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqDisk::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );
	TqFloat vv = m_MajorRadius - ( ( TqFloat ) v * ( m_MajorRadius - m_MinorRadius ) ) / m_vDiceSize;
	return ( CqVector3D( vv * cos( theta ), vv * sin( theta ), m_Height ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqDisk::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	Normal = CqVector3D( 0, 0, m_ThetaMax > 0 ? 1 : -1 );
	return ( DicePoint( u, v ) );
}


//------------------------------------------------------------------------------
/**
 *	Create the points which make up a NURBS circle control hull, for use during boundary
 *  generation.
 *
 *	@param	O.	Origin of the circle.
 *	@param	X.	X axis of the plane to generate the circle in.
 *	@param	Y.	Y axis of the plane to generate the circle in.
 *	@param	r.	Radius of the circle.
 *	@param	as.	Start angle of the circle.
 *	@param	ae.	End angle of the circle.
 *	@param	points.	Storage for the points of the circle.
 *
 *	@return	Description of the return value.
 */

void CqQuadric::Circle( const CqVector3D& O, const CqVector3D& X, const CqVector3D& Y, TqFloat r, TqFloat as, TqFloat ae, std::vector<CqVector3D>& points ) const
{
	TqFloat theta, angle, dtheta;
	TqUint narcs;

	while ( ae < as )
		ae += 2 * RI_PI;

	theta = ae - as;
	/*	if ( theta <= RI_PIO2 )
			narcs = 1;
		else
		{
			if ( theta <= RI_PI )
				narcs = 2;
			else
			{
				if ( theta <= 1.5 * RI_PI )
					narcs = 3;
				else*/
	narcs = 4;
	/*		}
		}*/
	dtheta = theta / static_cast<TqFloat>( narcs );
	TqUint n = 2 * narcs + 1;				// n control points ;

	CqVector3D P0, T0, P2, T2, P1;
	P0 = O + r * cos( as ) * X + r * sin( as ) * Y;
	T0 = -sin( as ) * X + cos( as ) * Y;		// initialize start values

	points.resize( n );

	points[ 0 ] = P0;
	TqUint index = 0;
	angle = as;

	TqUint i;
	for ( i = 1; i <= narcs; i++ )
	{
		angle += dtheta;
		P2 = O + r * cos( angle ) * X + r * sin( angle ) * Y;
		points[ index + 2 ] = P2;
		T2 = -sin( angle ) * X + cos( angle ) * Y;
		IntersectLine( P0, T0, P2, T2, P1 );
		points[ index + 1 ] = P1;
		index += 2;
		if ( i < narcs )
		{
			P0 = P2;
			T0 = T2;
		}
	}
}



CqBound CqQuadric::RevolveForBound( const std::vector<CqVector3D>& profile, const CqVector3D& S, const CqVector3D& Tvec, TqFloat theta ) const
{
	CqBound bound( FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );

	TqFloat angle, dtheta;
	TqUint narcs;
	TqUint i, j;

	if ( fabs( theta ) > 2.0 * RI_PI )
	{
		if ( theta < 0 )
			theta = -( 2.0 * RI_PI );
		else
			theta = 2.0 * RI_PI;
	}

	/*	if ( fabs( theta ) <= RI_PIO2 )
			narcs = 1;
		else
		{
			if ( fabs( theta ) <= RI_PI )
				narcs = 2;
			else
			{
				if ( fabs( theta ) <= 1.5 * RI_PI )
					narcs = 3;
				else*/
	narcs = 4;
	/*		}
		}*/
	dtheta = theta / static_cast<TqFloat>( narcs );

	TqUint n = 2 * narcs + 1;					// n control points ;

	std::vector<TqFloat> cosines( narcs + 1 );
	std::vector<TqFloat> sines( narcs + 1 );

	angle = 0.0;
	for ( i = 1; i <= narcs; i++ )
	{
		angle = dtheta * static_cast<TqFloat>( i );
		cosines[ i ] = cos( angle );
		sines[ i ] = sin( angle );
	}

	CqVector3D P0, T0, P2, T2, P1;
	CqVector3D vecTemp;

	for ( j = 0; j < profile.size(); j++ )
	{
		CqVector3D O;
		CqVector3D pj( profile[ j ] );

		ProjectToLine( S, Tvec, pj, O );
		CqVector3D X, Y;
		X = pj - O;

		TqFloat r = X.Magnitude();

		if ( r < 1e-7 )
		{
			bound.Encapsulate( O );
			continue;
		}

		X.Unit();
		Y = Tvec % X;
		Y.Unit();

		P0 = profile[ j ];
		bound.Encapsulate( P0 );

		T0 = Y;
		for ( i = 1; i <= narcs; ++i )
		{
			angle = dtheta * static_cast<TqFloat>( i );
			P2 = O + r * cosines[ i ] * X + r * sines[ i ] * Y;
			bound.Encapsulate( P2 );
			T2 = -sines[ i ] * X + cosines[ i ] * Y;
			IntersectLine( P0, T0, P2, T2, P1 );
			bound.Encapsulate( P1 );
			if ( i < narcs )
			{
				P0 = P2;
				T0 = T2;
			}
		}
	}
	return ( bound );
}


//---------------------------------------------------------------------
/** Find the point at which two infinite lines intersect.
 * The algorithm generates a plane from one of the lines and finds the 
 * intersection point between this plane and the other line.
 * \return TqFalse if they are parallel, TqTrue if they intersect.
 */

TqBool IntersectLine( CqVector3D& P1, CqVector3D& T1, CqVector3D& P2, CqVector3D& T2, CqVector3D& P )
{
	CqVector3D	v, px;

	px = T1 % ( P1 - T2 );
	v = px % T1;

	TqFloat	t = ( P1 - P2 ) * v;
	TqFloat vw = v * T2;
	if ( ( vw * vw ) < 1.0e-07 )
		return ( TqFalse );
	t /= vw;
	P = P2 + ( ( ( P1 - P2 ) * v ) / vw ) * T2 ;
	return ( TqTrue );
}


//---------------------------------------------------------------------
/** Project a point onto a line, returns the projection point in p.
 */

void ProjectToLine( const CqVector3D& S, const CqVector3D& Trj, const CqVector3D& pnt, CqVector3D& p )
{
	CqVector3D a = pnt - S;
	TqFloat fraction, denom;
	denom = Trj.Magnitude2();
	fraction = ( denom == 0.0 ) ? 0.0 : ( Trj * a ) / denom;
	p = fraction * Trj;
	p += S;
}

#pragma warning(pop)

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
