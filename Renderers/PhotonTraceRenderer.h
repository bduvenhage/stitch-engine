/*
 * $Id: PhotonTraceRenderer.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  PhotonTraceRenderer.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/06/13.
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

#ifndef STITCH_PHOTON_TRACE_RENDERER_H
#define STITCH_PHOTON_TRACE_RENDERER_H

namespace stitch {
    class Scene;
}

#include "Renderer.h"

namespace stitch {
    
    /*! Light/Photon trace renderer. */
    class PhotonTraceRenderer : public Renderer
    {
    public:
        PhotonTraceRenderer(Scene * const scene);
        
        virtual ~PhotonTraceRenderer()
        {
            //=== Delete previous photons and clear the vector...
            std::vector<stitch::Photon *>::const_iterator inFlightPhotonIter=inFlightPhotonVector_.begin();
            for (; inFlightPhotonIter!=inFlightPhotonVector_.end(); ++inFlightPhotonIter)
            {
                delete (*inFlightPhotonIter);
            }
            inFlightPhotonVector_.clear();//radiated and scattered photons.
            //===
            
            
            std::vector<stitch::Photon *>::const_iterator photonIter=photonVector_.begin();
            for (; photonIter!=photonVector_.end(); ++photonIter)
            {
                delete (*photonIter);
            }
            photonVector_.clear();
        }
        
        virtual void render(RadianceMap &radianceMap,
                            const stitch::Camera * const camera,
                            const float frameDeltaTime);
        
    protected:
        std::vector<stitch::Photon *> photonVector_;
        std::vector<stitch::Photon *> inFlightPhotonVector_;
        
        size_t tracePhotons(const stitch::Camera * const camera, const float frameDeltaTime);
    };
}

#endif// STITCH_PHOTON_TRACE_RENDERER_H
