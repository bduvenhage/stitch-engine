/*
 * $Id: exr.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  exr.cpp
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

#ifdef USE_OPENEXR

#include "exr.h"

#include <ImfArray.h>
#include <ImfRgbaFile.h>


/*! save an EXR file to disk */
void stitch::Exr::saveExr(RadianceMap *map, const std::string &filename)
{
    size_t width=map->getWidth();
    size_t height=map->getHeight();
    
    Imf::Array2D<Imf::Rgba> imagepixels(height, width);
    
    for (size_t y=0; y<height; ++y)
    {
        for (size_t x=0; x<width; ++x)
        {
            Colour_t c =map->getMapValue(x,y);
            imagepixels[y][x] = Imf::Rgba(c.x(), c.y(), c.z(), 1.0f);
        }
    }
    
    Imf::RgbaOutputFile ofile(filename.c_str(), width, height, Imf::WRITE_RGBA);
    ofile.setFrameBuffer(&imagepixels[0][0], 1, width);
    ofile.writePixels(height);
}



/*! load an EXR file from disk */
stitch::RadianceMap *stitch::Exr::loadExr(const std::string &filename)
{
    Imf::RgbaInputFile ifile(filename.c_str());
    
    Imath::Box2i image = ifile.dataWindow();
    size_t width = image.max.x - image.min.x + 1;
    size_t height = image.max.y - image.min.y + 1;
    
    Imf::Array2D<Imf::Rgba> imagepixels(height, width);
    ifile.setFrameBuffer(&imagepixels[0][0] - image.min.x - image.min.y * width, 1, width);
    ifile.readPixels(image.min.y, image.max.y);
    
    RadianceMap *map = new RadianceMap(width,height,Colour_t());
    
    if (ifile.lineOrder() == Imf::INCREASING_Y)
    {
        for (size_t y=0; y<height; ++y)
        {
            for (size_t x=0; x<width; ++x)
            {
                Imf::Rgba c = imagepixels[y][x];
                map->setMapValue(x,y, Colour_t(c.r,c.g,c.b));
            }
        }
    }
    else
    {
        for (size_t y=0; y<height; ++y)
        {
            for (size_t x=0; x<width; ++x)
            {
                Imf::Rgba c = imagepixels[y][x];
                map->setMapValue(x,( (height - y) - 1 ),Colour_t(c.r,c.g,c.b));
            }
        }
    }
    
    return map;
}

#endif
