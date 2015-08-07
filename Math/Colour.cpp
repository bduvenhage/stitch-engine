/*
 * $Id: Colour.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Colour.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/09/15.
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

#include "Colour.h"

float stitch::ColourMono::abyss_=0.0f;

stitch::ColourMono stitch::ColourMono::allZeroes()
{
    return ColourMono();//Zeroed in constructor;
}

stitch::ColourMono stitch::ColourMono::allOnes()
{
    return ColourMono().oneMinus();
}

stitch::ColourMono stitch::ColourMono::rand()
{
    return ColourMono(GlobalRand::uniformSampler()*2.0f-1.0f);
}

stitch::ColourMono stitch::ColourMono::randBall()
{
    return ColourMono(GlobalRand::uniformSampler()*2.0f-1.0f);
}

stitch::ColourMono stitch::ColourMono::randNorm()
{
    const float r=GlobalRand::uniformSampler();
    return (r<0.5f) ? ColourMono(-1.0f) : ColourMono(1.0f);
}
