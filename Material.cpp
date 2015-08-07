/*
 * $Id: Material.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Material.cpp
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

#include "Material.h"
#include "Photon.h"

const stitch::Colour_t stitch::Material::zero_;

//=======================================================================//
stitch::Photon stitch::Material::scatterPhoton_reject_samp(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const
{
    Vec3 dirB;
    Colour_t energyMask;
    //! @todo BRDF and scatter calculations per colour component.

    //! Absorbtion used for Russian roulette type propagation of photon.
    float albedoLength=getAlbedo().cavrg();

    if ( GlobalRand::uniformSampler() > albedoLength )
    {//Terminate photon.
        energyMask.setZeros();
    } else
    {//Scatter photon.
        const Vec3 dirA=photon.normDir_ * (-1.0f);
        
        const Vec3 BSDFPeakDir=getBSDFPeak(dirA, normal);
        const Colour_t BSDFPeakValue=BSDF(worldPosition, dirA, BSDFPeakDir, normal)*fabsf(BSDFPeakDir*normal);
        
        //! Simple uniform bound distribution for rejection sampling.
        const float BSDFPeakLength=BSDFPeakValue.cavrg();
        
        Colour_t Q;
        float QLength;
        
        //!Rejection sampling of BRDF.
        do {
            dirB=Vec3::randNorm();
            //! @todo Does this work for the glossy BRDF?
            Q=BSDF(worldPosition, dirA, dirB, normal)*fabsf(dirB*normal);
            QLength=Q.cavrg();
        } while ((GlobalRand::uniformSampler()*BSDFPeakLength) > QLength);
        
        //! Mask of normalised scat colour.
        energyMask=Q.rouletteMask(QLength);
    }
    
    const Colour_t scatteredEnergy=photon.energy_.cmult(energyMask);
    if (scatteredEnergy.lengthSq()==0.0f) dirB.setZeros();
    
    return Photon(worldPosition, dirB, scatteredEnergy, photon.scatterCount_+1);
}

