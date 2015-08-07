/*
 * $Id: RadianceMap.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  RadianceMap.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2009/09/17.
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

#include "RadianceMap.h"

const stitch::Colour_t stitch::RadianceMap::zero_;

void stitch::RadianceMap::generateToneMap()
{
    for (size_t i=0; i<8192; ++i)
    {
        toneMap_[i]=powf(i/8191.0f, tone_)*255.0f;
    }
}
