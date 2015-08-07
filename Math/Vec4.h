/*
 * $Id: Vec4.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Vec4.h
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

#ifndef STITCH_VEC4_H
#define STITCH_VEC4_H

namespace stitch {
	struct Vec3;
	class Vec4;
	class VecN;
}

#include "GlobalRand.h"

#include <cmath>

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

//===	
namespace stitch {
        
    //! 4D vector class.
	class Vec4
	{
	public:
        //! Nullary constructor.
		Vec4() :
        x_(0.0f), y_(0.0f), z_(0.0f), w_(0.0f)
		{//Always set to zero in default constructor!
		}
				
        //! Scalar constructor.
		Vec4(const float lValue) :
        x_(lValue), y_(lValue), z_(lValue), w_(lValue)
		{}
        
        //! Vector constructor.
		Vec4(const float x, const float y, const float z, const float w=1.0f) :
        x_(x), y_(y), z_(z), w_(w)
		{}
		
        //! Copy constructor.
		Vec4(const Vec4 &lValue) :
        x_(lValue.x_), y_(lValue.y_), z_(lValue.z_), w_(lValue.w_)
		{}
		
        //! Conversion constructor.
		explicit Vec4(const Vec3 &lValue, float w);
        
        //! Conversion constructor.
		explicit Vec4(const Vec3 &lValue);

        //! Conversion constructor.
		explicit Vec4(const VecN &lValue);
		
        ~Vec4() {}		

        static size_t getNumComponents()
        {
            return 4;
        }
        //===
        
        //! Equals overload.
		inline Vec4 & operator = (const Vec4 &lValue)
		{
			x_=lValue.x_;
			y_=lValue.y_;
			z_=lValue.z_;
			w_=lValue.w_;
			return (*this);
		}
		
        //! Equals conversion overload.
		Vec4 & operator = (const Vec3 &lValue);
        
        //! Equals conversion overload.
		Vec4 & operator = (const VecN &lValue);
        
        inline float x() const
        {
            return x_;
        }
        inline float & x()
        {
            return x_;
        }
        
        inline float y() const
        {
            return y_;
        }
        inline float & y()
        {
            return y_;
        }
        
        inline float z() const
        {
            return z_;
        }
        inline float & z()
        {
            return z_;
        }
        
        inline float w() const
        {
            return w_;
        }
        inline float & w()
        {
            return w_;
        }
        
        //! Component access index operator.
        inline float operator [] (const size_t i) const
        {
            switch (i) {
                case 0ul:
                    return x_;
                    break;                    
                case 1ul:
                    return y_;
                    break;
                case 2ul:
                    return z_;
                    break;
                case 3ul:
                    return w_;
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
                case 0ul:
                    return x_;
                    break;                    
                case 1ul:
                    return y_;
                    break;
                case 2ul:
                    return z_;
                    break;
                case 3ul:
                    return w_;
                    break;
                default:
                    return abyss_;
                    break;
            }
        }

		inline Vec4 &set(const Vec4 &lValue)
		{
			x_=lValue.x_;
			y_=lValue.y_;
			z_=lValue.z_;
			w_=lValue.w_;
			return (*this);
		}
		
		inline Vec4 &set(const float x, const float y, const float z, const float w)
		{
			x_=x;
			y_=y;
			z_=z;
			w_=w;
			return (*this);
		}
        
        inline void setZeros()
        {
			x_=y_=z_=w_=0.0f;
        }

        inline void setOnes()
        {
			x_=y_=z_=w_=1.0f;
        }
        
        //! Create all zeroes object.
        static Vec4 allZeros();

        //! Create all ones object.
        static Vec4 allOnes();
        
        inline Vec4 oneMinus() const
        {
			return Vec4(1.0f-x_, 1.0f-y_, 1.0f-z_, 1.0f-w_);
        }
		
        //===
        
		inline Vec4 operator * (const float lValue) const
		{
			return Vec4(this->x_*lValue, this->y_*lValue, this->z_*lValue, this->w_*lValue);
		}
        
		inline Vec4 &operator *= (const float lValue)
		{
			this->x_*=lValue;
            this->y_*=lValue;
            this->z_*=lValue;
            this->w_*=lValue;
            
            return *this;
		}
		
		inline Vec4 operator / (const float lValue) const
		{
            const float recipLValue=1.0f/lValue;
			return Vec4(this->x_*recipLValue, this->y_*recipLValue, this->z_*recipLValue, this->w_*recipLValue);
		}
        
		inline Vec4 &operator /= (const float lValue)
		{
            const float recipLValue=1.0f/lValue;
            
			this->x_*=recipLValue;
            this->y_*=recipLValue;
            this->z_*=recipLValue;
            this->w_*=recipLValue;
            
            return *this;
		}
        
        //===
        
        //! Dot product.
		inline float operator * (const Vec4 &lValue) const
		{
			return this->x_*lValue.x_ + this->y_*lValue.y_ + this->z_*lValue.z_ + this->w_*lValue.w_;
		}
        
        //! Component wise multiply.
		inline Vec4 cmult(const Vec4 &lValue) const //component wise multiply.
		{
			return Vec4(this->x_*lValue.x_, this->y_*lValue.y_, this->z_*lValue.z_, this->w_*lValue.w_);
		}
		
        //! Component wise multiply.
		inline Vec4 &operator *= (const Vec4 &lValue)
		{
			this->x_*=lValue.x_;
            this->y_*=lValue.y_;
            this->z_*=lValue.z_;
            this->w_*=lValue.w_;
            
            return *this;
		}
        //===
        
		inline Vec4 operator + (const Vec4 &lValue) const
		{
			return Vec4(this->x_+lValue.x_, this->y_+lValue.y_, this->z_+lValue.z_, this->w_+lValue.w_);
		}
        
		inline Vec4 &operator += (const Vec4 &lValue)
		{
			this->x_+=lValue.x_;
            this->y_+=lValue.y_;
            this->z_+=lValue.z_;
            this->w_+=lValue.w_;
            
            return *this;
		}
		
		inline Vec4 operator + (const float lValue) const
		{
			return Vec4(this->x_+lValue, this->y_+lValue, this->z_+lValue, this->w_+lValue);
		}
        
		inline Vec4 &operator += (const float lValue)
		{
			this->x_+=lValue;
            this->y_+=lValue;
            this->z_+=lValue;
            this->w_+=lValue;
            
            return *this;
		}
        
        //===
        
		inline Vec4 operator - (const Vec4 &lValue) const
		{
			return Vec4(this->x_-lValue.x_, this->y_-lValue.y_, this->z_-lValue.z_, this->w_-lValue.w_);
		}
		
		inline Vec4 &operator -= (const Vec4 &lValue)
		{
			this->x_-=lValue.x_;
            this->y_-=lValue.y_;
            this->z_-=lValue.z_;
            this->w_-=lValue.w_;
            
            return *this;
		}
		
		inline Vec4 operator - (const float lValue) const
		{
			return Vec4(this->x_-lValue, this->y_-lValue, this->z_-lValue, this->w_-lValue);
		}
        
		inline Vec4 &operator -= (const float lValue)
		{
			this->x_-=lValue;
            this->y_-=lValue;
            this->z_-=lValue;
            this->w_-=lValue;
            
            return *this;
		}

        //===

		inline float length() const
		{
			return sqrt(x_*x_+y_*y_+z_*z_+w_*w_);
		}
		
		inline float lengthSq() const
		{
			return (x_*x_+y_*y_+z_*z_+w_*w_);
		}
        
        //! Component sum.
		inline float csum() const
		{
			return (x_+y_+z_+w_);
		}
		
		inline float cavrg() const
		{
			return (x_+y_+z_+w_)/4.0f;
		}
		
        //! Normalise this vector and return what the length it WAS.
		inline float normalise()
		{
			const float vecLength=length();
			const float recipVecLength=1.0f/vecLength;
            
			x_*=recipVecLength;
            y_*=recipVecLength;
            z_*=recipVecLength;
            w_*=recipVecLength;
            
            return vecLength;
		}
        
        //! Return a normalised vector.
		inline Vec4 normalised() const
		{
			const float recipLength=1.0f/length();
			return Vec4(x_*recipLength, y_*recipLength, z_*recipLength, w_*recipLength);
		}
        
        //! Make area under this vector components one and return what the component sum was.
		inline float cnormalise()
		{
			const float cSum=csum();
			const float recipCSum=1.0f/cSum;
            
			x_*=recipCSum;
            y_*=recipCSum;
            z_*=recipCSum;
            w_*=recipCSum;
            
            return cSum;
		}
        
        //! Return a component normalised vector.
		inline Vec4 cnormalised() const
		{
			const float recipCSum=1.0f/csum();
			return Vec4(x_*recipCSum, y_*recipCSum, z_*recipCSum, w_*recipCSum);
		}

        //! Raise each component to the power e.
        inline Vec4 toThePower(const float e) const
        {
            return Vec4(powf(x_,e), powf(y_,e), powf(z_,e), powf(w_,e));
        }
        
        //! Raise each component to the power e.
        inline Vec4 operator ^ (const float e) const
        {
            return Vec4(powf(x_,e), powf(y_,e), powf(z_,e), powf(w_,e));
        }
        
        
        //! Make all components of this vector positive.
        inline void positivise()
        {
            x_=fabsf(x_);
            y_=fabsf(y_);
            z_=fabsf(z_);
            w_=fabsf(w_);
        }

        //! Return a vector with all positive components.
        inline Vec4 positivised()
        {
            return Vec4(fabsf(x_), fabsf(y_), fabsf(z_), fabsf(w_));
        }

        //! Clamp all components between low and high.
        inline void clamp(const float low, const float high)
        {
            //x_=max(min(x_,high),low);
            //y_=max(min(y_,high),low);
            //z_=max(min(z_,high),low);
            //w_=max(min(w_,high),low);
            x_=(x_<low) ? low : ((x_ > high) ? high : x_);
            y_=(y_<low) ? low : ((y_ > high) ? high : y_);
            z_=(z_<low) ? low : ((z_ > high) ? high : z_);
            w_=(w_<low) ? low : ((w_ > high) ? high : w_);
        }
        
        //! Generate a Russian Roulette mask using the vector components as probabilities.
        inline Vec4 rouletteMask(const float range) const
        {
            return Vec4(((GlobalRand::uniformSampler()*range)<x_) ? 1.0f : 0.0f,
                        ((GlobalRand::uniformSampler()*range)<y_) ? 1.0f : 0.0f,
                        ((GlobalRand::uniformSampler()*range)<z_) ? 1.0f : 0.0f,
                        ((GlobalRand::uniformSampler()*range)<w_) ? 1.0f : 0.0f);
        }
        
        inline bool operator == (const Vec4 &lValue) const
        {
            return ((x_==lValue.x_)&&(y_==lValue.y_)&&(z_==lValue.z_)&&(w_==lValue.w_));
        }
        
        inline bool operator != (const Vec4 &lValue) const
        {
            return !((*this)==lValue);
        }
        
        inline bool isZero() const
        {
            return ((x_==0.0f)&&(y_==0.0f)&&(z_==0.0f)&&(w_==0.0f));
        }
        
        static Vec4 rand();
        static Vec4 randBall();
        static Vec4 randNorm();
        
	private:
        union {//Ensure proper alignment for SSE.
            __m128 m128_;
            float v_[4];
            struct { float x_,y_,z_,w_; };
        };
        static float abyss_;
	};

}

#endif// STITCH_VEC4_H
