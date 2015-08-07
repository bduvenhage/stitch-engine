/*
 * $Id: Photon.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Photon.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2009/09/17.
 *  Copyright $Date: 2015-08-07 16:57:10 +0200 (Fri, 07 Aug 2015) $ Bernardt Duvenhage. All rights reserved.
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

#ifndef STITCH_PHOTON_H
#define STITCH_PHOTON_H


namespace stitch {
	class Photon;
}

#include "Math/Vec3.h"
#include "Math/Colour.h"
#include "BoundingVolume.h"
#include "Object.h"

namespace stitch
{
    //!Additional photon payload data. Used in experimental light beam tracer.
    class PhotonPayload
    {
    public:
        PhotonPayload(stitch::Object const * const generator, const Vec3 &origin, const float convolvedOriginSD) :
        origin_(origin),
        convolvedOriginSD_(convolvedOriginSD)
        {}
        
#ifdef USE_CXX11
        PhotonPayload(stitch::Object const * const generator, Vec3 &&origin, const float convolvedOriginSD) :
        origin_(std::move(origin)),
        convolvedOriginSD_(convolvedOriginSD)
        {}
#endif// USE_CXX11
        
        //!Copy operator.
        PhotonPayload(const PhotonPayload &lvalue) :
        origin_(lvalue.origin_),
        convolvedOriginSD_(lvalue.convolvedOriginSD_)
        {}
        
#ifdef USE_CXX11
        //!Move operator.
        PhotonPayload(PhotonPayload &&rvalue) noexcept:
        origin_(std::move(rvalue.origin_)),
        convolvedOriginSD_(rvalue.convolvedOriginSD_)
        {}
#endif// USE_CXX11
        
        PhotonPayload & operator =(const PhotonPayload &lvalue)
        {
            origin_=lvalue.origin_;
            convolvedOriginSD_=lvalue.convolvedOriginSD_;
            
            return *this;
        }
        
#ifdef USE_CXX11
        PhotonPayload & operator =(PhotonPayload &&rvalue) noexcept
        {
            origin_=std::move(rvalue.origin_);
            convolvedOriginSD_=rvalue.convolvedOriginSD_;
            
            return *this;
        }
#endif// USE_CXX11
        
        stitch::Vec3 origin_;
        float convolvedOriginSD_;
    };
    
    
    //! Models a single photon.
	class Photon : public BoundingVolume
	{
	public:
		Photon(const Vec3 &orig, const Vec3 &normDir, const Colour_t &energy, uint8_t scatterCount) :
		BoundingVolume(orig, 0.0f),
		normDir_(normDir),
        energy_(energy),
        scatterCount_(scatterCount),
        payload_(nullptr)
		{}
		
#ifdef USE_CXX11
        //!A move semantic constructor...
		Photon(Vec3 &&orig, const Vec3 &normDir, const Colour_t &energy, uint8_t scatterCount) :
		BoundingVolume(orig, 0.0f),
		normDir_(normDir),
        energy_(energy),
        scatterCount_(scatterCount),
        payload_(nullptr)
		{}
#endif// USE_CXX11
        
		virtual ~Photon()
		{
            if (payload_)
            {
                delete payload_;
            }
        }
        
		//!Copy constructor
		Photon(const Photon &lValue) :
		BoundingVolume(lValue),
		normDir_(lValue.normDir_),
        energy_(lValue.energy_),
        scatterCount_(lValue.scatterCount_),
        payload_(nullptr)
		{
            if (lValue.payload_)
            {
                payload_=new PhotonPayload(*lValue.payload_);
            }
        }
		
#ifdef USE_CXX11
		//!Move constructor
		Photon(Photon &&lValue) :
		BoundingVolume(std::move(lValue)),
		normDir_(std::move(lValue.normDir_)),
        energy_(std::move(lValue.energy_)),
        scatterCount_(lValue.scatterCount_),
        payload_(lValue.payload_)
		{
            lValue.payload_=nullptr;
        }
#endif// USE_CXX11
		
		inline Photon & operator = (const Photon &lValue)
		{
            if (&lValue!=this)
            {
                BoundingVolume::operator=(lValue);
                
                normDir_=lValue.normDir_;
                energy_=lValue.energy_;
                scatterCount_=lValue.scatterCount_;
                
                if (payload_)
                {
                    delete payload_;
                    payload_=nullptr;
                }
                
                if (lValue.payload_)
                {
                    payload_=new PhotonPayload(*lValue.payload_);
                }
            }
			return (*this);
		}
		
#ifdef USE_CXX11
		inline Photon & operator = (Photon &&lValue)
		{
            BoundingVolume::operator=(lValue);
            
			normDir_=std::move(lValue.normDir_);
            energy_=std::move(lValue.energy_);
			scatterCount_=lValue.scatterCount_;
            
            //swap the payloads.
            PhotonPayload* temp=payload_;
            payload_=lValue.payload_;
            lValue.payload_=temp;
            
            return (*this);
		}
#endif// USE_CXX11
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual Photon * clone() const
        {
            return new Photon(*this);
        }
        
		Vec3 normDir_;
        
        Colour_t energy_;//In Joule.
        
        uint8_t scatterCount_;//How many times has this photon been scattered.
		
        /*! Static value for the quanta of energy of a photon. */
		static float quantumEnergy_;//In Joule.
        
        static Vec3 rgb(float lambda);
        
        PhotonPayload *payload_;
    };
	
}

#endif// STITCH_PHOTON_H
