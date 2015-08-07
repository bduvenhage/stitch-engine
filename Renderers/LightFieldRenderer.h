/*
 * $Id: LightFieldRenderer.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  LightFieldRenderer.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2014/12/08.
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

#ifndef STITCH_LIGHT_FIELD_RENDERER_H
#define STITCH_LIGHT_FIELD_RENDERER_H

namespace stitch {
    class Scene;
}

#include "PhotonMap.h"
#include "Renderer.h"


namespace stitch {
    
    /*! Light field renderer. */
    class LightFieldRenderer : public ForwardRenderer
    {
    public:
        LightFieldRenderer(Scene * const scene);
        
        virtual ~LightFieldRenderer()
        {
            //=== Delete previous photons and clear the vector...
            std::vector<stitch::Photon *>::const_iterator photonIter=inFlightPhotonVector_.begin();
            for (; photonIter!=inFlightPhotonVector_.end(); ++photonIter)
            {
                delete (*photonIter);
            }
            inFlightPhotonVector_.clear();//radiated and scattered photons.
            //===
            
            photonMap_->clear();
            delete photonMap_;
        }
        
    private:
        stitch::PhotonMap *photonMap_;
        std::vector<stitch::Photon *> inFlightPhotonVector_;
        
        size_t tracePhotons(const float frameDeltaTime, const size_t photonTreeChunkSize);
        
    protected:
        virtual void preForwardRender(RadianceMap &radianceMap,
                                      const stitch::Camera * const camera,
                                      const float frameDeltaTime);
        
        virtual void gather(Ray &ray) const;
    };
}

#endif// STITCH_LIGHT_FIELD_RENDERER_H
