/*
 * $Id: Colour.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Colour.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/06/15.
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

#ifndef STITCH_COLOUR_H
#define STITCH_COLOUR_H


namespace stitch {
	struct ColourMono;
}

#include "Vec3.h"
#include "Vec4.h"
#include "VecN.h"

#include <cmath>

//===	
namespace stitch {    
    
    //! Typedef for colour type Colour_t: ColourMono, Vec3 and VecN should be compatible with the use of Colour_t.
    typedef Vec3 Colour_t;
}

//===	
namespace stitch {

    /*! Monochrome colour class. Similar to a VecN with N=1.*/
    struct ColourMono
	{
	public:
        //! Nullary constructor.
		ColourMono() :
        x_(0.0f)//Always set to zero in default constructor!
		{}
		
        //! Scalar constructor.
		ColourMono(const float x) :
        x_(x)
		{}

        //! Vector constructor.
		ColourMono(const float x, const float y, const float z=0, const float w=0) :
        x_(x)
		{}
        
        //! Copy constructor.
		ColourMono(const ColourMono &lValue) :
        x_(lValue.x_)
		{}
		
        ~ColourMono() {}
		
        static size_t getNumComponents()
        {
            return 1;
        }
        //===
        
        //! Equals overload.
		inline ColourMono & operator = (const ColourMono &lValue)
		{
			x_=lValue.x_;
			return (*this);
		}
		        
		inline ColourMono &set(const ColourMono &lValue)
		{
			x_=lValue.x_;
			return (*this);
		}
		
        //===
        
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
            return x_;
        }
        inline float & y()
        {
            return x_;
        }
        
        inline float z() const
        {
            return x_;
        }
        inline float & z()
        {
            return x_;
        }

        //! Component access index operator.
        inline float operator [] (const size_t i) const
        {
//            switch (i) {
//                case 0:
                    return x_;
//                    break;                    
//                default:
//                    return 0.0f;
//                    break;
//            }
        }
        
        //! Component reference access index operator.
        inline float & operator [] (const size_t i)
        {
//            switch (i) {
//                case 0:
                    return x_;
//                    break;                    
//                default:
//                    return abyss_;
//                    break;
//            }
        }
        
		inline ColourMono &set(const float x)
		{
			x_=x;
			return (*this);
		}
        
        inline void setZeros()
        {
			x_=0.0f;
        }
        
        inline void setOnes()
        {
			x_=1.0f;
        }
        
        //! Create all zeroes object.
        static ColourMono allZeroes();

        //! Create all ones object.
        static ColourMono allOnes();
        
        inline ColourMono oneMinus() const
        {
			return ColourMono(1.0f-x_);
        }
		
        //===
        
		inline ColourMono operator * (const float lValue) const
		{
			return ColourMono(this->x_*lValue);
		}
        
		inline ColourMono &operator *= (const float lValue)
		{
			this->x_*=lValue;
            
            return *this;
		}
        
		inline ColourMono operator / (const float lValue) const
		{
            const float recipLValue=1.0f/lValue;
			return ColourMono(this->x_*recipLValue);
		}
        
		inline ColourMono &operator /= (const float lValue)
		{
            const float recipLValue=1.0f/lValue;
            
			this->x_*=recipLValue;
            
            return *this;
		}
        
        //===
        
        //! Dot product.
		inline float operator * (const ColourMono &lValue) const
		{
			return this->x_*lValue.x_;
		}
        
        //! Component wise multiply.
		inline ColourMono cmult(const ColourMono &lValue) const
		{
			return ColourMono(this->x_*lValue.x_);
		}
		
        //! Component wise multiply.
		inline ColourMono &operator *= (const ColourMono &lValue)
		{
			this->x_*=lValue.x_;
            
            return *this;
		}
        //===
        
		//! Cross product.
		inline ColourMono operator ^ (const ColourMono &lValue) const
		{
			return ColourMono(0.0f);
		}
		
        //===
        
		inline ColourMono operator + (const ColourMono &lValue) const
		{
			return ColourMono(this->x_+lValue.x_);
		}
        
		inline ColourMono operator + (const float lValue) const
		{
			return ColourMono(this->x_+lValue);
		}
        
		inline ColourMono &operator += (const float lValue)
		{
			this->x_+=lValue;
            
            return *this;
		}
        
		inline ColourMono &operator += (const ColourMono &lValue)
		{
			this->x_+=lValue.x_;
            
            return *this;
		}
        
		inline ColourMono &addScaled(const ColourMono &lValue, const float scale)
		{
			this->x_+=lValue.x_ * scale;
            
            return *this;
		}

		inline ColourMono &addScaled(const ColourMono &lValue, const ColourMono &scale)
		{
			this->x_+=lValue.x_ * scale.x_;
            
            return *this;
		}
        
        inline void setToSumScaleAndNormalise(const ColourMono &A, const float a,
                                              const ColourMono &B, const float b,
                                              const ColourMono &C, const float c)
        {
            const float dx=A.x_*a + B.x_*b + C.x_*c;
            const float recipLength=1.0f / sqrtf(dx*dx);
            
            this->x_=dx*recipLength;
        }
        
        inline static ColourMono SumScaleAndNormalise(const ColourMono &A, const float a,
                                                const ColourMono &B, const float b,
                                                const ColourMono &C, const float c)
        {
            const float dx=A.x_*a + B.x_*b + C.x_*c;
            const float recipLength=1.0f / sqrtf(dx*dx);
            
            return ColourMono(dx*recipLength);
        }

        //===
        
		inline ColourMono operator - (const ColourMono &lValue) const
		{
			return ColourMono(this->x_-lValue.x_);
		}
        
		inline ColourMono operator - (const float lValue) const
		{
			return ColourMono(this->x_-lValue);
		}
        
		inline ColourMono &operator -= (const float lValue)
		{
			this->x_-=lValue;
            
            return *this;
		}
        
		inline ColourMono &operator -= (const ColourMono &lValue)
		{
			this->x_-=lValue.x_;
            
            return *this;
		}
        
        //===
        
		inline float length() const
		{
			return fabsf(x_);
		}
        
		inline float lengthSq() const
		{
			return x_*x_;
		}
        
        //! Component sum.
		inline float csum() const
		{
			return x_;
		}
        
        //! Component average.
		inline float cavrg() const
		{
			return x_;
		}
        
        //! Normalise this vector and return what the length it WAS.
		inline float normalise()
		{
			const float vecLength=length();
			const float recipVecLength=1.0f/vecLength;
            
			x_*=recipVecLength;
            
            return vecLength;
		}
        
        //! Return a normalised vector.
		inline ColourMono normalised() const
		{
			const float recipLength=1.0f/length();
			return ColourMono(x_*recipLength);
		}
		
        //! Make area under this vector components one and return what the component sum was.
		inline float cnormalise()
		{
			const float cSum=csum();
			const float recipCSum=1.0f/cSum;
            
			x_*=recipCSum;
            
            return recipCSum;
		}
        
        //! Return a component normalised vector.
		inline ColourMono cnormalised() const
		{
			const float recipCSum=1.0f/csum();
			return ColourMono(x_*recipCSum);
		}
        
        //===
        
        //! Raise each component to the power e.
        inline ColourMono toThePower(const float e) const
        {
            return ColourMono(powf(x_,e));
        }
        
        //! Raise each component to the power e.
        inline ColourMono operator ^ (const float e) const
        {
            return ColourMono(powf(x_,e));
        }
        
        //===
        
        //! Make all components of this vector positive.
        inline void positivise()
        {
            x_=fabsf(x_);
        }

        //! Return a vector with all positive components.
        inline ColourMono positivised()
        {
            return ColourMono(fabsf(x_));
        }

        //! Clamp all components between low and high.
        inline void clamp(const float low, const float high)
        {
            //x_=stitch::max(stitch::min(x_,high),low);
            x_=(x_<low) ? low : ((x_ > high) ? high : x_);
        }
        
        //! Generate a Russian Roulette mask using the vector components as probabilities.
        inline ColourMono rouletteMask(const float range) const
        {
            return ColourMono(((GlobalRand::uniformSampler()*range)<x_) ? 1.0f : 0.0f);
        }
        
        inline bool operator == (const ColourMono &lValue) const
        {
            return (x_==lValue.x_);
        }
        
        inline bool operator != (const ColourMono &lValue) const
        {
            return !((*this)==lValue);
        }
        
        inline bool isZero() const
        {
            return (x_==0.0f);
        }

        inline bool isNotZero() const
        {
            return (x_!=0.0f);
        }
        
        
        static ColourMono rand();
        static ColourMono randBall();
        static ColourMono randNorm();

	private:
		float x_;
        static float abyss_;
	};
    
}
    


#endif// STITCH_COLOUR_H
