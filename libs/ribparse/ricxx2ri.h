// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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

#include "ricxx.h"

#include <vector>

#include <boost/shared_ptr.hpp>

#ifndef AQSIS_RICXX2RI_INCLUDED
#define AQSIS_RICXX2RI_INCLUDED

namespace Aqsis {

/// Create a translation object from the RiCxx interface to the traditional RI.
boost::shared_ptr<Ri::RendererServices> createRiCxxToRi();


/// Create an object which serializes Ri::Renderer calls into a RIB stream.
boost::shared_ptr<Ri::RendererServices> createRibWriter(
        std::ostream& out, bool interpolateArchives, bool useBinary,
        bool useGzip);

boost::shared_ptr<Ri::Renderer> createRiCxxValidate(Ri::Renderer& out,
                                                    Ri::RendererServices& serv);

boost::shared_ptr<Ri::Renderer> createFrameDropFilter(
                        Ri::RendererServices& serv,
                        Ri::Renderer& out,
                        const Ri::ParamList& pList);

}

#endif // AQSIS_RICXX2RI_INCLUDED
// vi: set et:
