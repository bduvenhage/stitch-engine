/*
 * $Id: exr.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  exr.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2013-09-13.
 *  Copyright $Date: 2015-03-25 15:00:40 +0200 (Wed, 25 Mar 2015) $ Bernardt Duvenhage. All rights reserved.
 *
 *
 *  This file is part of StitchEngine.
 
 *  StitchEngine is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 
 *  StitchEngine is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with StitchEngine.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef STITCH_EXR_H
#define STITCH_EXR_H

#ifdef USE_OPENEXR

#include "RadianceMap.h"

namespace stitch
{
    //!Class to load and save radiance maps using the OpenExr format and lib developed by Industrial Light & Magic.
    class Exr
    {
    public:
    /*! load an EXR file from disk */
        static RadianceMap *loadExr(const std::string &filename);
    
    /*! save an EXR file to disk */
        static void saveExr(RadianceMap *map, const std::string &filename);
    };
}

#endif

#endif
