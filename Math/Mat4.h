/*
 * $Id: Mat4.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Mat4.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2013/10/17.
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

#ifndef STITCH_MAT4_H
#define STITCH_MAT4_H

namespace stitch {
	class Mat4;
	struct Vec3;
	class Vec4;
}

#include <utility> // for std::move
#include <cmath>
//===
namespace stitch {
    
    //#define STITCH_MAT4_USE_DYNAMIC 1
    
    //! 4x4 Matrix class.
	class Mat4
	{
	public:
        
        //! Identity constructor.
		Mat4()
#ifdef STITCH_MAT4_USE_DYNAMIC
        : v_(new float[16])
#endif
		{
            memset(v_, 0, 64);
			v_[0]=v_[5]=v_[10]=v_[15]=1.0f;
        }
        
        //! Matrix constructor.
		Mat4(const float v00, const float v01, const float v02, const float v03,
             const float v10, const float v11, const float v12, const float v13,
             const float v20, const float v21, const float v22, const float v23,
             const float v30, const float v31, const float v32, const float v33)
#ifdef STITCH_MAT4_USE_DYNAMIC
        : v_(new float[16])
#endif
		{
            v_[0]=v00; v_[1]=v01; v_[2]=v02; v_[3]=v03;
            v_[4]=v10; v_[5]=v11; v_[6]=v12; v_[7]=v13;
            v_[8]=v20; v_[9]=v21; v_[10]=v22; v_[11]=v23;
            v_[12]=v30; v_[13]=v31; v_[14]=v32; v_[15]=v33;
        }
        
        //! Copy constructor.
		Mat4(const Mat4 &lValue)
#ifdef STITCH_MAT4_USE_DYNAMIC
        : v_(new float[16])
#endif
		{
            memcpy(v_, lValue.v_, 64);
        }
        
#ifdef USE_CXX11
#ifdef STITCH_MAT4_USE_DYNAMIC
        //! Move constructor.
		Mat4(Mat4 &&rValue) noexcept :
        v_(rValue.v_)
		{
            rValue.v_=nullptr;
        }
#endif
#endif// USE_CXX11
        
        ~Mat4()
        {
#ifdef STITCH_MAT4_USE_DYNAMIC
            delete [] v_;
#endif
        }
        
        //! Equals overload.
		inline Mat4 & operator = (const Mat4 &lValue)
		{
            if (&lValue!=this)
            {
                memcpy(v_, lValue.v_, 64);
            }
			return (*this);
		}
        
#ifdef USE_CXX11
#ifdef STITCH_MAT4_USE_DYNAMIC
        //! Equals move overload.
		inline Mat4 & operator = (Mat4 &&rValue) noexcept
		{
            delete [] v_;
            v_=rValue.v_;
            rValue.v_=nullptr;
			return (*this);
		}
#endif
#endif// USE_CXX11
        
		inline Mat4 &set(const Mat4 &lValue)
		{
            if (&lValue!=this)
            {
                memcpy(v_, lValue.v_, 64);
            }
			return (*this);
		}
		
        //===
        
        //! Component access index operator.
        inline float operator () (const size_t i) const
        {
            return v_[i];
        }
        
        //! Component reference access index operator.
        inline float & operator () (const size_t i)
        {
            return v_[i];
        }
        
        //! Component access index operator.
        inline float operator () (const size_t j, const size_t i) const
        {
            return v_[(j<<2)+i];
        }
        
        //! Component reference access index operator.
        inline float & operator () (const size_t j, const size_t i)
        {
            return v_[(j<<2)+i];
        }
        
        //! Component access index operator.
        inline float cell(const size_t j, const size_t i) const
        {
            return v_[(j<<2)+i];
        }
        
        //! Component reference access index operator.
        inline float & cell(const size_t j, const size_t i)
        {
            return v_[(j<<2)+i];
        }
        
		inline Mat4 &set(const float v00, const float v01, const float v02, const float v03,
                         const float v10, const float v11, const float v12, const float v13,
                         const float v20, const float v21, const float v22, const float v23,
                         const float v30, const float v31, const float v32, const float v33)
		{
            v_[0]=v00; v_[1]=v01; v_[2]=v02; v_[3]=v03;
            v_[4]=v10; v_[5]=v11; v_[6]=v12; v_[7]=v13;
            v_[8]=v20; v_[9]=v21; v_[10]=v22; v_[11]=v23;
            v_[12]=v30; v_[13]=v31; v_[14]=v32; v_[15]=v33;
            
			return (*this);
		}
        
        inline void setZeros()
        {
            memset(v_, 0, 64);
        }
        
        inline void setOnes()
        {
			v_[0]=v_[1]=v_[2]=v_[3]=v_[4]=v_[5]=v_[6]=v_[7]=v_[8]=v_[9]=v_[10]=v_[11]=v_[12]=v_[13]=v_[14]=v_[15]=1.0f;
        }
        
        inline void setIdentity()
        {
            memset(v_, 0, 64);
			v_[0]=v_[5]=v_[10]=v_[15]=1.0f;
        }
        
        //! Post-multiply.
		inline Vec4 operator * (const Vec4 &lValue) const
		{
            return Vec4(dot(lValue, 0), dot(lValue, 1), dot(lValue, 2), dot(lValue, 3));
        }
        
        //! Post-multiply.
		inline Mat4 operator * (const Mat4 &lValue) const
		{
			return Mat4(dot(lValue,0,0), dot(lValue,0,1), dot(lValue,0,2), dot(lValue,0,3),
                        dot(lValue,1,0), dot(lValue,1,1), dot(lValue,1,2), dot(lValue,1,3),
                        dot(lValue,2,0), dot(lValue,2,1), dot(lValue,2,2), dot(lValue,2,3),
                        dot(lValue,3,0), dot(lValue,3,1), dot(lValue,3,2), dot(lValue,3,3));
		}
        
    private:
#ifdef STITCH_MAT4_USE_DYNAMIC
        float *v_;
#else
        union {//Ensure proper alignment for SSE.
            __m128 row_[4];
            float v_[16];
        };
#endif
        
        inline float dot(const Vec4 &lValue, const size_t &r) const
        {
            return v_[(r<<2)+0]*lValue.x() + v_[(r<<2)+1]*lValue.y() + v_[(r<<2)+2]*lValue.z() + v_[(r<<2)+3]*lValue.w();
        }
        
        inline float dot(const Mat4 &lValue, const size_t &r, const size_t &c) const
        {
            return v_[(r<<2)+0]*lValue(0, c) + v_[(r<<2)+1]*lValue(1, c) + v_[(r<<2)+2]*lValue(2, c) + v_[(r<<2)+3]*lValue(3, c);
        }
        
	};
}

#endif// STITCH_MAT4_H
