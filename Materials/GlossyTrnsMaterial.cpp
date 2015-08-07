/*
 * $Id: GlossyTrnsMaterial.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  GlossyTransMaterial.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2010/03/27.
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

#include "GlossyTrnsMaterial.h"
#include "Photon.h"



//=======================================================================//
stitch::Photon stitch::GlossyTransMaterial::scatterPhoton_direct(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const
{
    const Colour_t energyMask=trans_.rouletteMask(1.0f);
    const Colour_t scatteredEnergy=photon.energy_.cmult(energyMask);
    Vec3 scatteredDir;
    
    if (scatteredEnergy.lengthSq()>0.0f)
    {
        while (scatteredDir.lengthSq()==0.0f)
        {
            //const Vec3 litNormal=((normal*photon.normDir_)<0.0f) ? normal : normal.zeroMinus();
            
            const Vec3 scatterDir_relative_to_spec_refract=Vec3::randGaussianLobe(sd_);
            
            const Vec3 specRefractVector=whittedSpecRefractRay(photon.normDir_, normal);
            const Vec3 orthVecA=(specRefractVector.orthVec()).normalised();
            const Vec3 orthVecB=stitch::Vec3::crossNormalised(specRefractVector, orthVecA);
            
            scatteredDir=stitch::Vec3(orthVecA, orthVecB, specRefractVector,
                                      scatterDir_relative_to_spec_refract.x(), scatterDir_relative_to_spec_refract.y(), scatterDir_relative_to_spec_refract.z());
            
            //ToDo: Shadowing and masking to be investigated further for all methods of all BRDFs.
            //if ((scatteredDir*litNormal)>0.0f)
            //{
            //    scatteredDir.setZeros();
            //}
        }
    }
    
    return Photon(worldPosition, scatteredDir, scatteredEnergy, photon.scatterCount_+1);
}
