/*
 * $Id: Vec3.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Vec3.h
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

#ifndef STITCH_VEC3_H
#define STITCH_VEC3_H

namespace stitch {
	struct Vec3;
	class Vec4;
	class VecN;
}

#include "GlobalRand.h"
#include <utility> // for std::move
#include <cmath>
#include <map>
#include <cstring>

/*
 <mmintrin.h>  MMX
 <xmmintrin.h> SSE
 <emmintrin.h> SSE2
 <pmmintrin.h> SSE3
 <tmmintrin.h> SSSE3
 <smmintrin.h> SSE4.1
 <nmmintrin.h> SSE4.2
 <ammintrin.h> SSE4A
 <wmmintrin.h> AES
 <immintrin.h> AVX
 */

#include <xmmintrin.h> //for __m128

#ifdef USE_VEC3_SSE
#ifdef __SSE4_1__
#define STITCH_VEC3_USE_SSE 1
#include <smmintrin.h>
#endif
#endif

//===
namespace stitch {
    
    //!Potentially fast, but approximate, reciprocal square root.
    inline float rsqrtf( const float x )
    {
#ifdef STITCH_VEC3_USE_SSE
        const __m128 a = _mm_set_ss(x);
        const __m128 r = _mm_rsqrt_ps(a);
        const __m128 c = _mm_add_ps(_mm_mul_ps(_mm_set_ps(1.5f, 1.5f, 1.5f, 1.5f), r),
                                    _mm_mul_ps(_mm_mul_ps(_mm_mul_ps(a, _mm_set_ps(-0.5f, -0.5f, -0.5f, -0.5f)), r), _mm_mul_ps(r, r)));
        return _mm_cvtss_f32(c);
#else
        return 1.0f/sqrtf(x);
#endif//STITCH_VEC3_USE_SSE
    }
    
    
    //! 3D vector class implemented as struct.
	struct Vec3
	{
	public:
        //! Nullary constructor.
		Vec3()
#ifdef STITCH_VEC3_USE_SSE
        : m128_(_mm_setzero_ps())//Always set to zero in default constructor!
#else
#ifdef USE_CXX11
        : v_{0.0, 0.0, 0.0}
#endif
#endif//STITCH_VEC3_USE_SSE
		{
#ifndef USE_CXX11
            v_[0]=0.0;
            v_[1]=0.0;
            v_[2]=0.0;
#endif
        }
		
        //! Scalar constructor.
		Vec3(const float lValue)
#ifdef STITCH_VEC3_USE_SSE
        : m128_(_mm_set1_ps(lValue))
#else
#ifdef USE_CXX11
        : v_{lValue, lValue, lValue}
#endif
#endif//STITCH_VEC3_USE_SSE
		{
#ifndef USE_CXX11
            v_[0]=lValue;
            v_[1]=lValue;
            v_[2]=lValue;
#endif
        }
        
#ifdef STITCH_VEC3_USE_SSE
        //! Vector SSE constructor.
		Vec3(const __m128 m128) :
        m128_(m128)
        {}
#endif
        
        //! Vector constructor.
		Vec3(const float x, const float y, const float z)
#ifdef USE_CXX11
        : v_{x, y, z}
#endif
		{
#ifndef USE_CXX11
            v_[0]=x;
            v_[1]=y;
            v_[2]=z;
#endif
        }
        
        //! Copy constructor.
        Vec3(const Vec3 &lValue)
#ifdef USE_CXX11
        : v_{lValue.v_[0], lValue.v_[1], lValue.v_[2]}
#endif
        {
#ifndef USE_CXX11
            v_[0]=lValue.v_[0];
            v_[1]=lValue.v_[1];
            v_[2]=lValue.v_[2];
#endif
        }
        
        //! Conversion constructor.
		explicit Vec3(const Vec4 &lValue);
        
        //! Conversion constructor.
		explicit Vec3(const VecN &lValue);
        
        
        //! Special constructor: Aa
		explicit Vec3(const Vec3 &A, const float a)
		{
            v_[0]=A.v_[0] * a;
            v_[1]=A.v_[1] * a;
            v_[2]=A.v_[2] * a;
        }
        
        //! Special constructor: A+Bt
		explicit Vec3(const Vec3 &A, const Vec3 &B, const float t)
		{
            v_[0]=A.v_[0] + B.v_[0] * t;
            v_[1]=A.v_[1] + B.v_[1] * t;
            v_[2]=A.v_[2] + B.v_[2] * t;
        }
        
        //! Special constructor: Au+Bv
		explicit Vec3(const Vec3 &A, const Vec3 &B, const float u, const float v)
		{
            v_[0]=A.v_[0] * u + B.v_[0] * v;
            v_[1]=A.v_[1] * u + B.v_[1] * v;
            v_[2]=A.v_[2] * u + B.v_[2] * v;
        }
        
        //! Special constructor: A.a+B.b+C.c
		explicit Vec3(const Vec3 &A, const Vec3 &B, const Vec3 &C, const float a, const float b, const float c)
        {
            v_[0]=A.v_[0]*a + B.v_[0]*b + C.v_[0]*c;
            v_[1]=A.v_[1]*a + B.v_[1]*b + C.v_[1]*c;
            v_[2]=A.v_[2]*a + B.v_[2]*b + C.v_[2]*c;
        }
        
        
        //! Special constructor: B - A
		explicit Vec3(const Vec3 &A, const Vec3 &B)
		{
            v_[0]=B.v_[0] - A.v_[0];
            v_[1]=B.v_[1] - A.v_[1];
            v_[2]=B.v_[2] - A.v_[2];
        }
        
        static size_t getNumComponents()
        {
            return 3;
        }
        //===
        
        //! Equals overload.
		inline Vec3 & operator = (const Vec3 &lValue)
		{
            //m128_=lValue.m128_;
            v_[0]=lValue.v_[0];
            v_[1]=lValue.v_[1];
            v_[2]=lValue.v_[2];
			return (*this);
		}
        
        //! Equals conversion overload.
		Vec3 & operator = (const Vec4 &lValue);
        
        //! Equals conversion overload.
		Vec3 & operator = (const VecN &lValue);
        
		inline Vec3 &set(const Vec3 &lValue)
		{
            //m128_=lValue.m128_;
            v_[0]=lValue.v_[0];
            v_[1]=lValue.v_[1];
            v_[2]=lValue.v_[2];
			return (*this);
		}
		
        //===
        
        inline float x() const
        {
            return v_[0];
        }
        inline float & x()
        {
            return v_[0];
        }
        
        inline float y() const
        {
            return v_[1];
        }
        inline float & y()
        {
            return v_[1];
        }
        
        inline float z() const
        {
            return v_[2];
        }
        inline float & z()
        {
            return v_[2];
        }
        
        //! Component access index operator.
        inline float operator [] (const size_t i) const
        {
            switch (i) {
                case 0:
                    return v_[0];
                    break;
                case 1:
                    return v_[1];
                    break;
                case 2:
                    return v_[2];
                    break;
                default:
                    return 0.0f;
                    break;
            }
        }
        
        //! Component reference access index operator.
        inline float & operator [] (const size_t i)
        {
            switch (i) {
                case 0:
                    return v_[0];
                    break;
                case 1:
                    return v_[1];
                    break;
                case 2:
                    return v_[2];
                    break;
                default:
                    return abyss_;
                    break;
            }
        }
        
        friend std::ostream & operator<< (std::ostream &stream, const Vec3 &lValue)
        {
            stream << lValue.v_[0] << " ";
            stream << lValue.v_[1] << " ";
            stream << lValue.v_[2] << " ";
            
            return stream;
        }
        
        friend std::istream & operator>> (std::istream &stream, Vec3 &lValue)
        {
            stream >> lValue.v_[0];
            stream >> lValue.v_[1];
            stream >> lValue.v_[2];
            
            return stream;
        }
        
		inline Vec3 &set(const float x, const float y, const float z)
		{
			v_[0]=x;
			v_[1]=y;
			v_[2]=z;
            
			return (*this);
		}
        
#ifdef STITCH_VEC3_USE_SSE
		inline Vec3 &set(const __m128 m128)
		{
            m128_ = m128;
			return (*this);
		}
#endif//STITCH_VEC3_USE_SSE
        
        inline void setZeros()
        {
#ifdef STITCH_VEC3_USE_SSE
            m128_=_mm_setzero_ps();//Always set to zero in default constructor!
#else
            v_[0]=v_[1]=v_[2]=0.0f;//Always set to zero in default constructor!
#endif//STITCH_VEC3_USE_SSE
        }
        
		inline void setScalar(const float s)
		{
			v_[0]=v_[1]=v_[2]=s;
		}
        
        inline void setOnes()
        {
			v_[0]=v_[1]=v_[2]=1.0f;
        }
        
        //! Create all zeroes object.
        static Vec3 allZeros();
        
        //! Create all ones object.
        static Vec3 allOnes();
        
        inline Vec3 oneMinus() const
        {
			return Vec3(1.0f-v_[0], 1.0f-v_[1], 1.0f-v_[2]);
        }
		
        inline Vec3 zeroMinus() const
        {
            return Vec3(-v_[0], -v_[1], -v_[2]);
        }
        
        //===
        
		inline Vec3 operator * (const float lValue) const
		{
			return Vec3(this->v_[0]*lValue, this->v_[1]*lValue, this->v_[2]*lValue);
		}
        
		inline Vec3 &operator *= (const float lValue)
		{
			this->v_[0]*=lValue;
            this->v_[1]*=lValue;
            this->v_[2]*=lValue;
            
            return *this;
		}
        
		inline Vec3 operator / (const float lValue) const
		{
            const float recipLValue=1.0f/lValue;
			return Vec3(this->v_[0]*recipLValue, this->v_[1]*recipLValue, this->v_[2]*recipLValue);
		}
        
		inline Vec3 &operator /= (const float lValue)
		{
            const float recipLValue=1.0f/lValue;
            
			this->v_[0]*=recipLValue;
            this->v_[1]*=recipLValue;
            this->v_[2]*=recipLValue;
            
            return *this;
		}
        
        //===
        //! Dot product.
		inline float operator * (const Vec3 &lValue) const
        {
#ifdef STITCH_VEC3_USE_SSE
            return _mm_cvtss_f32(_mm_dp_ps(m128_,lValue.m128_,0x71));
#else
            //Please mister compiler... do your best.
			return this->v_[0]*lValue.v_[0] + this->v_[1]*lValue.v_[1] + this->v_[2]*lValue.v_[2];
#endif
		}
        
        //! Dot product.
        inline static float dot(const Vec3 &lValue1, const Vec3 &lValue2)
        {
#ifdef STITCH_VEC3_USE_SSE
            return _mm_cvtss_f32(_mm_dp_ps(lValue1.m128_,lValue2.m128_,0x71));
#else
            //Please mister compiler... do your best.
            return lValue1.v_[0]*lValue2.v_[0] + lValue1.v_[1]*lValue2.v_[1] + lValue1.v_[2]*lValue2.v_[2];
#endif
        }
        
        //! Dot product and scale.
        inline static float dotscale(const Vec3 &lValue1, const Vec3 &lValue2, const float scale)
        {
#ifdef STITCH_VEC3_USE_SSE
            return (_mm_cvtss_f32(_mm_dp_ps(lValue1.m128_,lValue2.m128_,0x71)))*scale;
#else
            //Please mister compiler... do your best.
            return (lValue1.v_[0]*lValue2.v_[0] + lValue1.v_[1]*lValue2.v_[1] + lValue1.v_[2]*lValue2.v_[2])*scale;
#endif
        }
        
        //! Dot product (V1 - V3) dot (V2 - V3)
        inline static float dotmin(const Vec3 &lValue1, const Vec3 &lValue2, const Vec3 &lValue3)
        {
            return
            ((lValue1.v_[0]-lValue3.v_[0])*(lValue2.v_[0]-lValue3.v_[0]) +
             ((lValue1.v_[1]-lValue3.v_[1])*(lValue2.v_[1]-lValue3.v_[1]) +
              ((lValue1.v_[2]-lValue3.v_[2])*(lValue2.v_[2]-lValue3.v_[2]))));
        }
        
        //! Dot product (B - A) dot (O - A)
        inline static float dotorigin(const Vec3 &A, const Vec3 &B)
        {
            return
            ((B.v_[0]-A.v_[0])*(0.0f-A.v_[0]) +
             ((B.v_[1]-A.v_[1])*(0.0f-A.v_[1]) +
              ((B.v_[2]-A.v_[2])*(0.0f-A.v_[2]))));
        }
        
        //! Component wise multiply.
        inline Vec3 cmult(const Vec3 &lValue) const
        {
            return Vec3(this->v_[0]*lValue.v_[0], this->v_[1]*lValue.v_[1], this->v_[2]*lValue.v_[2]);
        }
        
        //! Component wise multiply.
        inline Vec3 &operator *= (const Vec3 &lValue)
        {
            this->v_[0]*=lValue.v_[0];
            this->v_[1]*=lValue.v_[1];
            this->v_[2]*=lValue.v_[2];
            
            return *this;
        }
        //===
        
        //! Cross product.
        inline Vec3 operator ^ (const Vec3 &lValue) const
        {
            return Vec3(this->v_[1]*lValue.v_[2]-this->v_[2]*lValue.v_[1],
                        -this->v_[0]*lValue.v_[2]+this->v_[2]*lValue.v_[0],
                        this->v_[0]*lValue.v_[1]-this->v_[1]*lValue.v_[0]);
        }
        
        //! Cross product.
        static inline Vec3 cross(const Vec3 &A, const Vec3 &B)
        {
            return Vec3(A.v_[1]*B.v_[2]-A.v_[2]*B.v_[1],
                        -A.v_[0]*B.v_[2]+A.v_[2]*B.v_[0],
                        A.v_[0]*B.v_[1]-A.v_[1]*B.v_[0]);
        }
        
        //! Cross product.
        static inline Vec3 crossNormalised(const Vec3 &A, const Vec3 &B)
        {
            const float Ax=A.v_[0];
            const float Ay=A.v_[1];
            const float Az=A.v_[2];
            
            const float Bx=B.v_[0];
            const float By=B.v_[1];
            const float Bz=B.v_[2];
            
            const float cpx=Ay*Bz-Az*By;
            const float cpy=-Ax*Bz+Az*Bx;
            const float cpz=Ax*By-Ay*Bx;
            
            const float recipR=1.0f/sqrtf(cpx*cpx+cpy*cpy+cpz*cpz);
            
            return Vec3(cpx*recipR, cpy*recipR, cpz*recipR);
        }
        
        //! (A-C)^(B-C)
        static inline Vec3 cross(const Vec3 &A, const Vec3 &B, const Vec3 &C)
        {
            const float v0x=A.v_[0]-C.v_[0];
            const float v0y=A.v_[1]-C.v_[1];
            const float v0z=A.v_[2]-C.v_[2];
            
            const float v1x=B.v_[0]-C.v_[0];
            const float v1y=B.v_[1]-C.v_[1];
            const float v1z=B.v_[2]-C.v_[2];
            
            return Vec3(v0y*v1z-v0z*v1y,
                        -v0x*v1z+v0z*v1x,
                        v0x*v1y-v0y*v1x);
        }
        
        //! ((A-C)^(B-C)).normalised()
        static inline Vec3 crossNormalised(const Vec3 &A, const Vec3 &B, const Vec3 &C)
        {
            const float v0x=A.v_[0]-C.v_[0];
            const float v0y=A.v_[1]-C.v_[1];
            const float v0z=A.v_[2]-C.v_[2];
            
            const float v1x=B.v_[0]-C.v_[0];
            const float v1y=B.v_[1]-C.v_[1];
            const float v1z=B.v_[2]-C.v_[2];
            
            const float cpx=v0y*v1z-v0z*v1y;
            const float cpy=-v0x*v1z+v0z*v1x;
            const float cpz=v0x*v1y-v0y*v1x;
            
            const float recipR=1.0f/sqrtf(cpx*cpx+cpy*cpy+cpz*cpz);
            
            return Vec3(cpx*recipR, cpy*recipR, cpz*recipR);
        }
        
        //! ((A-C)^(B-C)).lengthSq()
        static inline float crossLengthSq(const Vec3 &A, const Vec3 &B, const Vec3 &C)
        {
            const float v0x=A.v_[0]-C.v_[0];
            const float v0y=A.v_[1]-C.v_[1];
            const float v0z=A.v_[2]-C.v_[2];
            
            const float v1x=B.v_[0]-C.v_[0];
            const float v1y=B.v_[1]-C.v_[1];
            const float v1z=B.v_[2]-C.v_[2];
            
            const float cpx=v0y*v1z-v0z*v1y;
            const float cpy=-v0x*v1z+v0z*v1x;
            const float cpz=v0x*v1y-v0y*v1x;
            
            return (cpx*cpx+cpy*cpy+cpz*cpz);
        }
        
        //! ((A-C)^(B-C)).length()
        static inline float crossLength(const Vec3 &A, const Vec3 &B, const Vec3 &C)
        {
            const float v0x=A.v_[0]-C.v_[0];
            const float v0y=A.v_[1]-C.v_[1];
            const float v0z=A.v_[2]-C.v_[2];
            
            const float v1x=B.v_[0]-C.v_[0];
            const float v1y=B.v_[1]-C.v_[1];
            const float v1z=B.v_[2]-C.v_[2];
            
            const float cpx=v0y*v1z-v0z*v1y;
            const float cpy=-v0x*v1z+v0z*v1x;
            const float cpz=v0x*v1y-v0y*v1x;
            
            return sqrtf(cpx*cpx+cpy*cpy+cpz*cpz);
        }
        
        //! ((A-0)^(B-0)).lengthSq()
        static inline float crossLengthSq(const Vec3 &A, const Vec3 &B)
        {
            const float v0x=A.v_[0];
            const float v0y=A.v_[1];
            const float v0z=A.v_[2];
            
            const float v1x=B.v_[0];
            const float v1y=B.v_[1];
            const float v1z=B.v_[2];
            
            const float cpx=v0y*v1z-v0z*v1y;
            const float cpy=-v0x*v1z+v0z*v1x;
            const float cpz=v0x*v1y-v0y*v1x;
            
            return (cpx*cpx+cpy*cpy+cpz*cpz);
        }
        //===
        
        inline Vec3 operator + (const Vec3 &lValue) const
        {
            return Vec3(this->v_[0]+lValue.v_[0], this->v_[1]+lValue.v_[1], this->v_[2]+lValue.v_[2]);
        }
        
        inline static Vec3 sum(const Vec3 &A, const Vec3 &B, const Vec3 &C)
        {
            return Vec3(A.v_[0]+B.v_[0]+C.v_[0],
                        A.v_[1]+B.v_[1]+C.v_[1],
                        A.v_[2]+B.v_[2]+C.v_[2]);
        }
        
        inline Vec3 operator + (const float lValue) const
        {
            return Vec3(this->v_[0]+lValue, this->v_[1]+lValue, this->v_[2]+lValue);
        }
        
        inline Vec3 &operator += (const float lValue)
        {
            this->v_[0]+=lValue;
            this->v_[1]+=lValue;
            this->v_[2]+=lValue;
            
            return *this;
        }
        
        inline Vec3 &operator += (const Vec3 &lValue)
        {
            this->v_[0]+=lValue.v_[0];
            this->v_[1]+=lValue.v_[1];
            this->v_[2]+=lValue.v_[2];
            
            return *this;
        }
        
        inline Vec3 &setScaled(const Vec3 &lValue, const float scale)
        {
            this->v_[0]=lValue.v_[0] * scale;
            this->v_[1]=lValue.v_[1] * scale;
            this->v_[2]=lValue.v_[2] * scale;
            
            return *this;
        }
        
        inline Vec3 &addScaled(const Vec3 &lValue, const float scale)
        {
            this->v_[0]+=lValue.v_[0] * scale;
            this->v_[1]+=lValue.v_[1] * scale;
            this->v_[2]+=lValue.v_[2] * scale;
            
            return *this;
        }
        
        inline Vec3 &addScaled(const Vec3 &lValue, const Vec3 &scale)
        {
            this->v_[0]+=lValue.v_[0] * scale.v_[0];
            this->v_[1]+=lValue.v_[1] * scale.v_[1];
            this->v_[2]+=lValue.v_[2] * scale.v_[2];
            
            return *this;
        }
        
        inline void setToSumScaleAndNormalise(const Vec3 &A, const float a,
                                              const Vec3 &B, const float b,
                                              const Vec3 &C, const float c)
        {
            const float dx=A.v_[0]*a + B.v_[0]*b + C.v_[0]*c;
            const float dy=A.v_[1]*a + B.v_[1]*b + C.v_[1]*c;
            const float dz=A.v_[2]*a + B.v_[2]*b + C.v_[2]*c;
            
            const float recipLength=1.0f / sqrtf(dx*dx + dy*dy + dz*dz);
            //const float recipLength=rsqrtf(dx*dx + dy*dy + dz*dz);
            
            this->v_[0]=dx*recipLength;
            this->v_[1]=dy*recipLength;
            this->v_[2]=dz*recipLength;
        }
        
        //! Calculates (A*a + B*b + C*c) normalised.
        inline static Vec3 sumScaleAndNormalise(const Vec3 &A, const float a,
                                                const Vec3 &B, const float b,
                                                const Vec3 &C, const float c)
        {
            const float dx=A.v_[0]*a + B.v_[0]*b + C.v_[0]*c;
            const float dy=A.v_[1]*a + B.v_[1]*b + C.v_[1]*c;
            const float dz=A.v_[2]*a + B.v_[2]*b + C.v_[2]*c;
            
            const float recipLength=1.0f / sqrtf(dx*dx + (dy*dy + dz*dz));
            //const float recipLength=rsqrtf(dx*dx + (dy*dy + dz*dz));
            
            return Vec3(dx*recipLength, dy*recipLength, dz*recipLength);
        }
        
        inline static Vec3 avrg(const Vec3 &A,
                                const Vec3 &B,
                                const Vec3 &C)
        {
            const float recipN=1.0f/3.0f;
            
            return Vec3((A.v_[0]+B.v_[0]+C.v_[0])*recipN,
                        (A.v_[1]+B.v_[1]+C.v_[1])*recipN,
                        (A.v_[2]+B.v_[2]+C.v_[2])*recipN);
        }
        
        inline static Vec3 avrg(const Vec3 &A,
                                const Vec3 &B)
        {
            const float recipN=0.5f;
            
            return Vec3((A.v_[0]+B.v_[0])*recipN,
                        (A.v_[1]+B.v_[1])*recipN,
                        (A.v_[2]+B.v_[2])*recipN);
        }
        
        //===
        
        inline Vec3 operator - (const Vec3 &lValue) const
        {
            return Vec3(this->v_[0]-lValue.v_[0], this->v_[1]-lValue.v_[1], this->v_[2]-lValue.v_[2]);
        }
        
        inline Vec3 operator - (const float lValue) const
        {
            return Vec3(this->v_[0]-lValue, this->v_[1]-lValue, this->v_[2]-lValue);
        }
        
        inline Vec3 &operator -= (const float lValue)
        {
            this->v_[0]-=lValue;
            this->v_[1]-=lValue;
            this->v_[2]-=lValue;
            
            return *this;
        }
        
        inline Vec3 &operator -= (const Vec3 &lValue)
        {
            this->v_[0]-=lValue.v_[0];
            this->v_[1]-=lValue.v_[1];
            this->v_[2]-=lValue.v_[2];
            
            return *this;
        }
        
        //===
        
        inline float length() const
        {
#ifdef STITCH_VEC3_USE_SSE
            return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(m128_,m128_,0x71)));
            return sqrtf(_mm_cvtss_f32(_mm_dp_ps(m128_,m128_,0x71)));
            
            //Please mister compiler... do your best.
            //return sqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
#else
            //Please mister compiler... do your best.
            return sqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
#endif
        }
        
        inline float lengthSq() const
        {
#ifdef STITCH_VEC3_USE_SSE
            return _mm_cvtss_f32(_mm_dp_ps(m128_,m128_,0x71));
            
            //Please mister compiler... do your best.
            //return v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2];
#else
            //Please mister compiler... do your best.
            return v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2];
#endif
        }
        
        inline bool isPointWithinSq(const Vec3 &B, const float &distSq) const
        {
            if ( ((v_[0]-B.v_[0])*(v_[0]-B.v_[0]) < distSq) &&
                ((v_[1]-B.v_[1])*(v_[1]-B.v_[1]) < distSq) &&
                ((v_[2]-B.v_[2])*(v_[2]-B.v_[2]) < distSq) )
            {
                return true;
            } else
            {
                return false;
            }
        }
        
        inline float calcDistToPoint(const Vec3 &B) const
        {
            const float dx=v_[0]-B.v_[0];
            const float dy=v_[1]-B.v_[1];
            const float dz=v_[2]-B.v_[2];
            
            return sqrtf(dx*dx+dy*dy+dz*dz);
        }
        
        inline float calcDistToPointSq(const Vec3 &B) const
        {
            const float dx=v_[0]-B.v_[0];
            const float dy=v_[1]-B.v_[1];
            const float dz=v_[2]-B.v_[2];
            
            return (dx*dx+dy*dy+dz*dz);
        }
        
        inline static float calcDistToPoint(const Vec3 &A, const Vec3 &B)
        {
            const float dx=A.v_[0]-B.v_[0];
            const float dy=A.v_[1]-B.v_[1];
            const float dz=A.v_[2]-B.v_[2];
            
            return sqrtf(dx*dx+dy*dy+dz*dz);
        }
        
        inline static float calcDistToPointSq(const Vec3 &A, const Vec3 &B)
        {
            const float dx=A.v_[0]-B.v_[0];
            const float dy=A.v_[1]-B.v_[1];
            const float dz=A.v_[2]-B.v_[2];
            
            return (dx*dx+dy*dy+dz*dz);
        }
        
        inline static float calcDistToLineSq(const Vec3 &point, Vec3 lA, const Vec3 &lB)
        {
            Vec3 normDirAB=lA;
            normDirAB-=lB;
            
            const float lengthAB=normDirAB.normalise_rt();
            
            const float t=(point-lB)*normDirAB;
            
            if (t<0.0f)
            {
                return Vec3::calcDistToPointSq(point, lB);
            } else
                if (t>lengthAB)
                {
                    return Vec3::calcDistToPointSq(point, lA);
                }
                else
                {
                    return Vec3::calcDistToPointSq(point, (lB + normDirAB * t));
                }
        }
        
        inline float calcDistToLineSq(Vec3 lA, const Vec3 &lB) const
        {
            Vec3 normDirAB=lA;
            normDirAB-=lB;
            
            const float lengthAB=normDirAB.normalise_rt();
            
            const float t=((*this)-lB)*normDirAB;
            
            if (t<0.0f)
            {
                return Vec3::calcDistToPointSq(*this, lB);
            } else
                if (t>lengthAB)
                {
                    return Vec3::calcDistToPointSq(*this, lA);
                }
                else
                {
                    return Vec3::calcDistToPointSq(*this, (lB + normDirAB * t));
                }
        }
        
        //! Component sum.
        inline float csum() const
        {
            return (v_[0]+v_[1]+v_[2]);
        }
        
        //! Component average.
        inline float cavrg() const
        {
            return (v_[0]+v_[1]+v_[2])/3.0f;
        }
        
        //! Normalise this vector and return what the length WAS.
        inline float normalise_rt()
        {
#ifdef STITCH_VEC3_USE_SSE
            
            const __m128 norm = _mm_sqrt_ps(_mm_dp_ps(m128_, m128_, 0x7F));
            m128_ = _mm_div_ps(m128_, norm);
            return _mm_cvtss_f32(norm);
#else
            const float vecLength=sqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
            const float recipVecLength=1.0f/vecLength;
            
            v_[0]*=recipVecLength;
            v_[1]*=recipVecLength;
            v_[2]*=recipVecLength;
            
            return vecLength;
#endif
        }
        
        inline void normalise()
        {
#ifdef STITCH_VEC3_USE_SSE
            m128_ = _mm_div_ps(m128_, _mm_sqrt_ps(_mm_dp_ps(m128_, m128_, 0x7F)));
#else
            const float recipVecLength=1.0f/sqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
            //const float recipVecLength=rsqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
            
            v_[0]*=recipVecLength;
            v_[1]*=recipVecLength;
            v_[2]*=recipVecLength;
#endif
        }
        
        //! Return a normalised vector.
        inline Vec3 normalised() const
        {
#ifdef STITCH_VEC3_USE_SSE
            return Vec3(_mm_div_ps(m128_, _mm_sqrt_ps(_mm_dp_ps(m128_, m128_, 0x7F))));
#else
            const float recipLength=1.0f/sqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
            //const float recipLength=rsqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
            return Vec3(v_[0]*recipLength, v_[1]*recipLength, v_[2]*recipLength);
#endif
        }
        
        
        inline void normalise_approx()
        {
#ifdef STITCH_VEC3_USE_SSE
            m128_ = _mm_mul_ps(m128_, _mm_rsqrt_ps(_mm_dp_ps(m128_, m128_, 0x77)));
#else
            const float recipVecLength=1.0f/sqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
            //const float recipVecLength=rsqrtf(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
            
            v_[0]*=recipVecLength;
            v_[1]*=recipVecLength;
            v_[2]*=recipVecLength;
#endif
        }
        
        //! Return a normalised vector.
        inline Vec3 normalised_approx() const
        {
#ifdef STITCH_VEC3_USE_SSE
            return Vec3(_mm_mul_ps(m128_, _mm_rsqrt_ps(_mm_dp_ps(m128_, m128_, 0x77))));
#else
            const float recipLength=1.0f/sqrtf(v_[0]*v_[0]+(v_[1]*v_[1]+v_[2]*v_[2]));
            //const float recipLength=rsqrtf(v_[0]*v_[0]+(v_[1]*v_[1]+v_[2]*v_[2]));
            return Vec3(v_[0]*recipLength, v_[1]*recipLength, v_[2]*recipLength);
#endif
        }
        
        //! Make area under this vector components one and return what the component sum was.
        inline float cnormalise()
        {
            const float cSum=csum();
            const float recipCSum=1.0f/cSum;
            
            v_[0]*=recipCSum;
            v_[1]*=recipCSum;
            v_[2]*=recipCSum;
            
            return recipCSum;
        }
        
        //! Return a component normalised vector.
        inline Vec3 cnormalised() const
        {
            const float recipCSum=1.0f/csum();
            return Vec3(v_[0]*recipCSum, v_[1]*recipCSum, v_[2]*recipCSum);
        }
        
        //===
        
        //! Raise each component to the power e.
        inline Vec3 toThePower(const float e) const
        {
            return Vec3(powf(v_[0],e), powf(v_[1],e), powf(v_[2],e));
        }
        
        //! Raise each component to the power e.
        inline Vec3 operator ^ (const float e) const
        {
            return Vec3(powf(v_[0],e), powf(v_[1],e), powf(v_[2],e));
        }
        
        //===
        
        //! Make all components of this vector positive.
        inline void positivise()
        {
            v_[0]=fabsf(v_[0]);
            v_[1]=fabsf(v_[1]);
            v_[2]=fabsf(v_[2]);
        }
        
        //! Return a vector with all positive components.
        inline Vec3 positivised()
        {
            return Vec3(fabsf(v_[0]), fabsf(v_[1]), fabsf(v_[2]));
        }
        
        //! Clamp all components between low and high.
        inline void clamp(const float low, const float high)
        {
            //v_[0]=max(min(v_[0],high),low);
            //v_[1]=max(min(v_[1],high),low);
            //v_[2]=max(min(v_[2],high),low);
            v_[0]=(v_[0]<low) ? low : ((v_[0] > high) ? high : v_[0]);
            v_[1]=(v_[1]<low) ? low : ((v_[1] > high) ? high : v_[1]);
            v_[2]=(v_[2]<low) ? low : ((v_[2] > high) ? high : v_[2]);
        }
        
        //! Generate a Russian Roulette mask using the vector components as probabilities.
        inline Vec3 rouletteMask(const float range) const
        {
            return Vec3(((GlobalRand::uniformSampler()*range)<v_[0]) ? 1.0f : 0.0f,
                        ((GlobalRand::uniformSampler()*range)<v_[1]) ? 1.0f : 0.0f,
                        ((GlobalRand::uniformSampler()*range)<v_[2]) ? 1.0f : 0.0f);
        }
        
        inline bool operator == (const Vec3 &lValue) const
        {
            return ((v_[0]==lValue.v_[0])&&(v_[1]==lValue.v_[1])&&(v_[2]==lValue.v_[2]));
        }
        
        inline bool operator != (const Vec3 &lValue) const
        {
            return !((*this)==lValue);
        }
        
        inline bool isZero() const
        {
            return ((v_[0]==0.0f)&&(v_[1]==0.0f)&&(v_[2]==0.0f));
        }
        
        inline bool isNotZero() const
        {
            return ((v_[0]!=0.0f)||(v_[1]!=0.0f)||(v_[2]!=0.0f));
        }
        
        //==
        //! Generate a random vector within the domain of [-1..1, -1..1, -1..1)
        static Vec3 rand();
        
        //! rand()'s PDF.
        static float rand_pdf(const Vec3 &v);
        
        //==
        //! Generate a random vector with radius <= 1.
        static Vec3 randBall();
        
        //! randBall()'s PDF.
        static float randBall_pdf(const Vec3 &v);
        
        //==
        //! Generate a random vector on XY disc with radius <= 1.
        static Vec3 randDisc();
        
        //! randDisc()'s PDF.
        static float randDisc_pdf(const Vec3 &v);
        
        //==
        //! Generate a random vector with radius == 1.
        static Vec3 randNorm();
        
        //! rand()'s PDF.
        static float randNorm_pdf(const Vec3 &v);
        
        //==
        /*! \brief Generate a vector sampled from a Gaussian probability distribution (i.e. lobe) around the mean normal (0,0,1).
         *  \param sd Standard deviation in radians.
         *  \return Gaussian distributed vector of length 1.
         */
        static Vec3 randGaussianLobe(const float sd);
        
        //! randGaussianLobe()'s PDF.
        static float randGaussianLobe_pdf(const float sd, const Vec3 &v);
        
        //==
        //! Generate a vector sampled from a Cosine probability distribution (i.e. lobe) around the normal (0,0,1).
        static Vec3 randCosineLobe(const float exp);
        
        //! randCosineLobe()'s PDF.
        static float randCosineLobe_pdf(const float exp, const Vec3 &v);
        
        //!Cosine lobe PDF normalised??? over projected solid angle.
        static float randCosineLobe_pdf_proj(const float exp, const Vec3 &v);
        
        //==
        //! Generate a set of approximately equidistant vectors using the Fibonacci spiral sphere.
        static void equidistantVectors_FibonacciSpiralSphere(const size_t minimumNumVectors, std::vector<stitch::Vec3> &vectors, std::vector<size_t> &binIndices);
        
        //! Get the number of vertices, edges and faces/bins that the sub-divided icosahedron will have.
        static void equidistantVectors_IcosahedronBased_GetDetails_minNumVertices(const size_t minNumVertices,
                                                                           size_t &actualNumVertices,
                                                                           size_t &actualNumEdges,
                                                                           size_t &actualNumBins);
        
        //! Get the number of vertices, edges and faces/bins that the sub-divided icosahedron will have.
        static void equidistantVectors_IcosahedronBased_GetDetails_minNumBins(const size_t minNumBins,
                                                                       size_t &actualNumVertices,
                                                                       size_t &actualNumEdges,
                                                                       size_t &actualNumBins);
        
        
        //! Generate a set of approximately equidistant vectors by subdividing an icosahedron. At least minimumNumVectors number of vectors are returns.
        static void equidistantVectors_IcosahedronBased(const size_t minimumNumVectors, std::vector<stitch::Vec3> &vectors, std::vector<size_t> &binIndices);
        
        //! Relax the equidistant vectors to remove any tension that results from unequal spacing of vectors.
        static void relaxEquidistantVectors(std::vector<stitch::Vec3> &vectors, size_t numIterations);
        
        //! Relax the equidistant vectors to remove any tension that results from unequal spacing of vectors. This method uses a kdtree search and is faster in certain cases.
        static void relaxEquidistantVectorsII(std::vector<stitch::Vec3> &vectors, uint32_t numIterations);
        
        
        inline Vec3 orthVecX0_() const
        {
            return Vec3(0.0f, v_[2], -v_[1]);
        }
        
        inline Vec3 orthVecY0_() const
        {
            return Vec3(v_[2], 0.0f, -v_[0]);
        }
        
        inline Vec3 orthVecZ0_() const
        {
            return Vec3(v_[1], -v_[0], 0.0f);
        }
        
        //! Return a/any orthogonal vector to this vector.
        inline Vec3 orthVec() const
        {
            const float fabsX=fabsf(v_[0]);
            const float fabsY=fabsf(v_[1]);
            const float fabsZ=fabsf(v_[2]);
            
            if ((fabsX<fabsY)&&(fabsX<fabsZ))
            {
                return orthVecX0_();
            } else
                if (fabsY<fabsZ)
                {
                    return orthVecY0_();
                } else
                {
                    return orthVecZ0_();
                }
        }
        
        //!Associate a unique random vector with every key that this method is called on.
        static Vec3 uniqueValue(uintptr_t key);
        
        
    public:
        
        union {//Ensure proper alignment for SSE.
//#ifdef STITCH_VEC3_USE_SSE
            //__m128 m128_;
//#endif
            float v_[3];
            //struct { float x_,y_,z_; };
        };
        
    private:
        static float abyss_;
        static std::map<uintptr_t, Vec3> uniqueValueMap_;
    };
}

#endif// STITCH_VEC3_H
