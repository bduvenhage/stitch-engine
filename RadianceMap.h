/*
 * $Id: RadianceMap.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  RadianceMap.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/06/01.
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

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

namespace stitch {
    class RadianceMap;
}

#include "Math/Colour.h"
#include "Math/MathUtil.h"
#include "KDTree.h"
#include "Timer.h"

#ifdef USE_CXX11
#include <mutex>
#else
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#endif

#include <iostream>

#ifdef USE_CXX11
#include <cstdint>
#include <random>
#else
#include <stdint.h>
#include <boost/random.hpp>
#endif

namespace stitch {
    
    //! Models a raster and Voronoi radiance map with radiance pixel elements of type Colour_t.
    class RadianceMap
    {
    public:
        class PixelBoundingVolume : public BoundingVolume
        {
        public:
            PixelBoundingVolume(const Vec3 &orig, const Colour_t &c) :
            BoundingVolume(orig, 0.7072f),
            colour_(c)
            {
            }
            
#ifdef USE_CXX11
            //!A move semantic constructor...
            PixelBoundingVolume(const Vec3 &orig, Colour_t &&c) :
            BoundingVolume(orig, 0.7072f),
            colour_(c)
            {}
            
            PixelBoundingVolume(Vec3 &&orig, Colour_t &&c) :
            BoundingVolume(orig, 0.7072f),
            colour_(c)
            {}
#endif// USE_CXX11
            
            //!Copy constructor
            PixelBoundingVolume(const PixelBoundingVolume &lValue) :
            BoundingVolume(lValue),
            colour_(lValue.colour_)
            {
            }
            
#ifdef USE_CXX11
            //!Move constructor
            PixelBoundingVolume(PixelBoundingVolume &&lValue) :
            BoundingVolume(std::move(lValue)),
            colour_(std::move(lValue.colour_))
            {
            }
#endif// USE_CXX11
            
            inline PixelBoundingVolume & operator = (const PixelBoundingVolume &lValue)
            {
                BoundingVolume::operator=(lValue);
                
                colour_=lValue.colour_;
                
                return (*this);
            }
            
#ifdef USE_CXX11
            inline PixelBoundingVolume & operator = (PixelBoundingVolume &&lValue)
            {
                BoundingVolume::operator=(lValue);
                
                colour_=std::move(lValue.colour_);
                
                return (*this);
            }
#endif// USE_CXX11
            
            /*! Virtual constructor idiom. Clone operator. */
            virtual PixelBoundingVolume * clone() const
            {
                return new PixelBoundingVolume(*this);
            }
            
            Colour_t const & getColour() const
            {
                return colour_;
            }
            
        private:
            Colour_t colour_;
        };
        
        
        //Must be ((power of two) - 1) for use as bit mask power-of-two modulus!!!
#define CONCURRENT_BACK_BUFFERS_BM 7
        
        
        RadianceMap(const size_t width, const size_t height, const Colour_t &initRadiance) :
        width_(width), height_(height), exposure_(1.0), tone_(1.0),
        map_(new Colour_t[width_*height_]),
        displayBuffer_(new uint8_t[width_*height_*4])
        {
            mapFrontKDTree_.reserveLinear(width_*height_);//max storage required.
            
            for (size_t backBufferNum=0; backBufferNum<=((size_t)CONCURRENT_BACK_BUFFERS_BM); backBufferNum++)
            {
                mapBackBufferArray_[backBufferNum].reserve(width_*height_);
            }
            
            clear(initRadiance);
            
            generateToneMap();
            
            randomOffsetVect_.reserve(width_*height_);
            for (size_t i=0; i<(width_*height_); ++i)
            {
                randomOffsetVect_.push_back(i);
            }
        }
        
        
        /*! Copy constructor. */
        RadianceMap(const RadianceMap &lValue) :
        width_(lValue.width_), height_(lValue.height_),
        exposure_(lValue.exposure_), tone_(lValue.tone_),
        map_(new Colour_t[width_*height_]),
        displayBuffer_(new uint8_t[width_*height_*4]),
        mapFrontKDTree_(lValue.mapFrontKDTree_)
        {
            size_t numPixels=width_*height_;
            
            for (size_t pixelNum=0; pixelNum<numPixels; ++pixelNum)
            {
                map_[pixelNum]=lValue.map_[pixelNum];
                
                displayBuffer_[(pixelNum<<2)+0]=lValue.displayBuffer_[(pixelNum<<2)+0];
                displayBuffer_[(pixelNum<<2)+1]=lValue.displayBuffer_[(pixelNum<<2)+1];
                displayBuffer_[(pixelNum<<2)+2]=lValue.displayBuffer_[(pixelNum<<2)+2];
                displayBuffer_[(pixelNum<<2)+3]=lValue.displayBuffer_[(pixelNum<<2)+3];
            }
            
            randomOffsetVect_=lValue.randomOffsetVect_;
        }
        
        /*! Assignment operator. */
        RadianceMap & operator=( const RadianceMap &lValue)
        {
            if (&lValue!=this)
            {
                width_=lValue.width_;
                height_=lValue.height_;
                exposure_=lValue.exposure_;
                tone_=lValue.tone_;
                
                delete [] map_;
                map_=new Colour_t[width_*height_];
                
                delete [] displayBuffer_;
                displayBuffer_=new uint8_t[width_*height_*4];
                
                mapFrontKDTree_=lValue.mapFrontKDTree_;
                
                size_t numPixels=width_*height_;
                
                for (size_t pixelNum=0; pixelNum<numPixels; ++pixelNum)
                {
                    map_[pixelNum]=lValue.map_[pixelNum];
                    
                    displayBuffer_[(pixelNum<<2)+0]=lValue.displayBuffer_[(pixelNum<<2)+0];
                    displayBuffer_[(pixelNum<<2)+1]=lValue.displayBuffer_[(pixelNum<<2)+1];
                    displayBuffer_[(pixelNum<<2)+2]=lValue.displayBuffer_[(pixelNum<<2)+2];
                    displayBuffer_[(pixelNum<<2)+3]=lValue.displayBuffer_[(pixelNum<<2)+3];
                }
                
                randomOffsetVect_=lValue.randomOffsetVect_;
            }
            
            return (*this);
        }
        
        void clear(const Colour_t &initRadiance)
        {
            for (size_t backBufferNum=0; backBufferNum<=((size_t)CONCURRENT_BACK_BUFFERS_BM); backBufferNum++)
            {
#ifdef USE_CXX11
                std::lock_guard<std::mutex> scopedLock(mapBackMutexArray_[backBufferNum]);
#else
                boost::lock_guard<boost::mutex> scopedLock(mapBackMutexArray_[backBufferNum]);
#endif
                mapBackBufferArray_[backBufferNum].clear();
            }
            
            {
#ifdef USE_CXX11
                std::lock_guard<std::mutex> scopedLock(mapFrontMutex_);
#else
                boost::lock_guard<boost::mutex> scopedLock(mapFrontMutex_);
#endif
                mapFrontKDTree_.clear();
            }
            
            for (size_t i=0; i<(width_*height_); ++i)
            {
                map_[i]=initRadiance;
                
                displayBuffer_[(i<<2)+0]=0;
                displayBuffer_[(i<<2)+1]=0;
                displayBuffer_[(i<<2)+2]=0;
                displayBuffer_[(i<<2)+3]=0;
            }
        }
        
        //!Shuffle the offset map used to select a random pixel to render.
        void shuffleOffsetMap()
        {
#ifdef USE_CXX11
            std::shuffle(randomOffsetVect_.begin(), randomOffsetVect_.end(), stitch::GlobalRand::rndGen);
#else
            // Fisher–Yates shuffle a.k.a. Knuth shuffle.
            const size_t numItems=width_*height_;
            
            for (size_t i=(numItems-1); i>=1; --i)
            {
                //j <= random integer with 0 ≤ j ≤ i
//#ifdef USE_CXX11
                //std::uniform_int_distribution<> dist(0, i); //[0,i]
//#else
                boost::uniform_int_distribution<> dist(0, i); //[0,i]
//#endif
                size_t j=dist(stitch::GlobalRand::rndGen);
                
                std::swap(randomOffsetVect_[i], randomOffsetVect_[j]);
            }
#endif
        }
        
        ~RadianceMap()
        {
            delete [] displayBuffer_;
            delete [] map_;
        }
        
        inline size_t getWidth()
        {
            return width_;
        }
        
        inline size_t getHeight()
        {
            return height_;
        }
        
        void setExposure(const float exposure)
        {
            exposure_=exposure;
        }
        
        void setTone(const float tone)
        {
            tone_=tone;
            generateToneMap();
        }
        
        void updateDisplayBuffer()//Update uint8 frame buffer.
        {
            stitch::Timer timer;
            stitch::Timer_t startTick, endTick;
            
            startTick=timer.tick();
            {
                const size_t numPixels=width_*height_;
                size_t i=0,j=0;
                for (; i<numPixels; ++i)
                {
                    displayBuffer_[j++]=pixelToneMap(map_[i][0]*exposure_);
                    displayBuffer_[j++]=pixelToneMap(map_[i][1]*exposure_);
                    displayBuffer_[j++]=pixelToneMap(map_[i][2]*exposure_);
                    displayBuffer_[j++]=0;
                }
            }
            endTick=timer.tick();
            //std::cout << " RadianceMap::updateDisplayBuffer::update_display " << timer.delta_m(startTick, endTick) << " ms.\n";
            //std::cout.flush();
        }
        
        void updateVoronoiDisplayBuffer()//Update uint8 frame buffer.
        {
#ifdef USE_CXX11
            std::lock_guard<std::mutex> scopedLock(mapFrontMutex_);
#else
            boost::lock_guard<boost::mutex> scopedLock(mapFrontMutex_);
#endif
            
            //stitch::Timer timer;
            //stitch::Timer_t startTick=0.0, endTick=0.0;
            //startTick=timer.tick();
            
            for (size_t backBufferNum=0; backBufferNum<=((size_t)CONCURRENT_BACK_BUFFERS_BM); backBufferNum++)
            {
#ifdef USE_CXX11
                std::lock_guard<std::mutex> scopedLock(mapBackMutexArray_[backBufferNum]);
#else
                boost::lock_guard<boost::mutex> scopedLock(mapBackMutexArray_[backBufferNum]);
#endif
                
                while (!mapBackBufferArray_[backBufferNum].empty())
                {
                    mapFrontKDTree_.addItem(mapBackBufferArray_[backBufferNum].back());
                    mapBackBufferArray_[backBufferNum].pop_back();
                }
            }
            
            std::vector<stitch::Vec3> splitAxisVec;
            splitAxisVec.reserve(2);
            splitAxisVec.push_back(Vec3(1.0f, 0.0f, 0.0f));
            splitAxisVec.push_back(Vec3(0.0f, 1.0f, 0.0f));
            
            mapFrontKDTree_.build(8, 0, 1000, splitAxisVec);//Ensure kdTree is optimised for search.
            
            //endTick=timer.tick();
            //std::cout << " RadianceMap::updateVoronoiDisplayBuffer::build_mapFrontKDTree_ " << timer.delta_m(startTick, endTick) << " ms.\n";
            //std::cout.flush();
            
            
            //startTick=timer.tick();
            {
                for (size_t y=0; y<height_; ++y)
                {
                    size_t displayBufferOffset=(y*width_)<<2;
                    
                    for (size_t x=0; x<width_; ++x)
                    {
                        Colour_t mapValue=getMapVoronoiValue(x+0.5f,y+0.5f);
                        mapValue*=exposure_;
                        
                        displayBuffer_[displayBufferOffset++]=pixelToneMap(mapValue.x());
                        displayBuffer_[displayBufferOffset++]=pixelToneMap(mapValue.y());
                        displayBuffer_[displayBufferOffset++]=pixelToneMap(mapValue.z());
                        displayBuffer_[displayBufferOffset++]=0;
                    }
                }
            }
            //endTick=timer.tick();
            //std::cout << " RadianceMap::updateVoronoiDisplayBuffer::update_display " << timer.delta_m(startTick, endTick) << " ms.\n";
            //std::cout.flush();
        }
        
        //!Get a random pixel to render.
        inline void getShuffledXY(const size_t iX, const size_t iY,
                                  size_t &oX, size_t &oY) const
        {
            const size_t iOffset=iX + iY*width_;
            const size_t oOffset=randomOffsetVect_[iOffset];
            
            oY=oOffset/width_;
            oX=oOffset-oY*width_;
        }
        
        Colour_t const & getMapValue(const size_t x, const size_t y) const
        {
            return map_[x+y*width_];
        }
        
        void addToMapValue(const float fx, const float fy, Colour_t const & colour, const size_t backBufferNum=0)
        {
            /*
             PixelBoundingVolume *pbv=new PixelBoundingVolume(stitch::Vec3(fx, fy, 0.0f), colour);
             
             mapBackMutexArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].lock();
             mapBackBufferArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].push_back(pbv);
             mapBackMutexArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].unlock();
             */
            
            const size_t ix=(size_t)fx;
            const size_t iy=(size_t)fy;
            
            map_[ix+iy*width_]+=colour;
        }
        
        inline void setMapValue(const size_t x, const size_t y, Colour_t const & colour, const size_t backBufferNum=0)
        {
#ifdef USE_OSG
            PixelBoundingVolume *pbv=new PixelBoundingVolume(stitch::Vec3(x+0.5f, y+0.5f, 0.0f), colour);
            
            mapBackMutexArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].lock();
            mapBackBufferArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].push_back(pbv);
            mapBackMutexArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].unlock();
#endif//USE_OSG
            
            map_[x+y*width_]=colour;
        }
        
#ifdef USE_CXX11
        inline void setMapValue(const size_t x, const size_t y, Colour_t && colour, const size_t backBufferNum=0)
        {
#ifdef USE_OSG
            PixelBoundingVolume *pbv=new PixelBoundingVolume(stitch::Vec3(x+0.5f, y+0.5f, 0.0f), colour);
            
            mapBackMutexArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].lock();
            mapBackBufferArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].push_back(pbv);
            mapBackMutexArray_[backBufferNum & ((size_t)CONCURRENT_BACK_BUFFERS_BM)].unlock();
#endif//USE_OSG
            map_[x+y*width_]=colour;
        }
#endif
        
        uint8_t const * const getDisplayBuffer() const
        {
            return displayBuffer_;
        }
        
    private:
        const static Colour_t zero_;
        size_t width_, height_;
        
        float toneMap_[8192];
        void generateToneMap();
        
        
        inline uint8_t pixelToneMap(const float pixelValue) const
        {
            //return (uint8_t)( toneMap_[(size_t)( stitch::MathUtil::min(pixelValue, 1.0f) * 8191.0f + 0.5f )] );
            return stitch::MathUtil::min(powf(pixelValue, tone_), 1.0f)*255.0f + 0.5f;
        }
        
        
        //Note: Made private because it is assumed that the calling function has called 'mapFrontMutex_.lock()' !!!
        //      Could alternatively use a recursive_mutex.
        inline Colour_t const & getMapVoronoiValue(const float x, const float y) const
        {
            //Note: Made private because it is assumed that the calling function has called 'mapFrontMutex_.lock()' !!!
            
            float searchRadius=((float)FLT_MAX);
            
            stitch::BoundingVolume const * const bv=mapFrontKDTree_.getNearest(stitch::Vec3(x,y,0.0f), searchRadius);
            
            if (bv!=nullptr)
            {
                return (static_cast<PixelBoundingVolume const * const>(bv))->getColour();
            } else
            {
                return zero_;
            }
        }
        
        
        float exposure_;
        float tone_;
        
        Colour_t * map_;
        uint8_t * displayBuffer_;
        std::vector<size_t> randomOffsetVect_;
        
        
        stitch::KDTree mapFrontKDTree_;//Keep a tame kd-tree on the side for efficient searching of map pixels.
#ifdef USE_CXX11
        mutable std::mutex mapFrontMutex_;
#else
        boost::mutex mapFrontMutex_;
#endif
        
        //! Radiance map back buffers. A number of backbuffers is allocated so that multiple threads may push data with minimum blocking locking.
        std::vector< PixelBoundingVolume * > mapBackBufferArray_[CONCURRENT_BACK_BUFFERS_BM+1];
#ifdef USE_CXX11
        mutable std::mutex mapBackMutexArray_[CONCURRENT_BACK_BUFFERS_BM+1];
#else
        boost::mutex mapBackMutexArray_[CONCURRENT_BACK_BUFFERS_BM+1];
#endif
    };
}


#endif// FRAME_BUFFER_H
