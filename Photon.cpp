/*
 * $Id: Photon.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Photon.cpp
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

#include "Photon.h"

float stitch::Photon::quantumEnergy_=0.000003f; //in Joule!

//=======================================================================//
stitch::Vec3 stitch::Photon::rgb(float lambda)
{
    Vec3 colour;
    
    if (lambda<377.50000f) colour=stitch::Vec3(0.000000000f, 0.000000000f, 0.000000000f); else
    if (lambda<382.50000f) colour=stitch::Vec3(0.001296500f, -0.001086480f, 0.006948340f); else
    if (lambda<387.50000f) colour=stitch::Vec3(0.001741160f, -0.001507840f, 0.011200420f); else
    if (lambda<392.50000f) colour=stitch::Vec3(0.003436600f, -0.003046880f, 0.021458820f); else
    if (lambda<397.50000f) colour=stitch::Vec3(0.006274800f, -0.005484800f, 0.038645160f); else
    if (lambda<402.50000f) colour=stitch::Vec3(0.011876400f, -0.010284520f, 0.072483780f); else
    if (lambda<407.50000f) colour=stitch::Vec3(0.019323040f, -0.016775520f, 0.117648920f); else
    if (lambda<412.50000f) colour=stitch::Vec3(0.035728980f, -0.031281160f, 0.221395600f); else
    if (lambda<417.50000f) colour=stitch::Vec3(0.062989140f, -0.055636640f, 0.396329860f); else
    if (lambda<422.50000f) colour=stitch::Vec3(0.107544640f, -0.095899520f, 0.689055840f); else
    if (lambda<427.50000f) colour=stitch::Vec3(0.166848520f, -0.151262800f, 1.108782380f); else
    if (lambda<432.50000f) colour=stitch::Vec3(0.211425900f, -0.195753320f, 1.477997640f); else
    if (lambda<437.50000f) colour=stitch::Vec3(0.229612380f, -0.219348600f, 1.730348400f); else
    if (lambda<442.50000f) colour=stitch::Vec3(0.222376040f, -0.221745000f, 1.861358180f); else
    if (lambda<447.50000f) colour=stitch::Vec3(0.193573220f, -0.207317560f, 1.897483360f); else
    if (lambda<452.50000f) colour=stitch::Vec3(0.147633940f, -0.180837680f, 1.884050420f); else
    if (lambda<457.50000f) colour=stitch::Vec3(0.089503240f, -0.146281480f, 1.851441420f); else
    if (lambda<462.50000f) colour=stitch::Vec3(0.017975680f, -0.099844640f, 1.768272880f); else
    if (lambda<467.50000f) colour=stitch::Vec3(-0.061709420f, -0.041160760f, 1.614087260f); else
    if (lambda<472.50000f) colour=stitch::Vec3(-0.148609360f, 0.034898480f, 1.353293440f); else
    if (lambda<477.50000f) colour=stitch::Vec3(-0.232056480f, 0.116857320f, 1.086218660f); else
    if (lambda<482.50000f) colour=stitch::Vec3(-0.309220800f, 0.201929280f, 0.836300360f); else
    if (lambda<487.50000f) colour=stitch::Vec3(-0.379541140f, 0.287027120f, 0.620011000f); else
    if (lambda<492.50000f) colour=stitch::Vec3(-0.448015920f, 0.378545920f, 0.451063600f); else
    if (lambda<497.50000f) colour=stitch::Vec3(-0.526084320f, 0.485583640f, 0.321501020f); else
    if (lambda<502.50000f) colour=stitch::Vec3(-0.616318500f, 0.612514120f, 0.221884440f); else
    if (lambda<507.50000f) colour=stitch::Vec3(-0.724257400f, 0.770600400f, 0.141445340f); else
    if (lambda<512.50000f) colour=stitch::Vec3(-0.822049420f, 0.941195560f, 0.065122480f); else
    if (lambda<517.50000f) colour=stitch::Vec3(-0.896427200f, 1.117426200f, -0.004387940f); else
    if (lambda<522.50000f) colour=stitch::Vec3(-0.925389220f, 1.273862760f, -0.058663120f); else
    if (lambda<527.50000f) colour=stitch::Vec3(-0.892821860f, 1.384202560f, -0.095152940f); else
    if (lambda<532.50000f) colour=stitch::Vec3(-0.809894220f, 1.458464920f, -0.122040800f); else
    if (lambda<537.50000f) colour=stitch::Vec3(-0.689931840f, 1.498843640f, -0.142592080f); else
    if (lambda<542.50000f) colour=stitch::Vec3(-0.535614780f, 1.509092800f, -0.157012660f); else
    if (lambda<547.50000f) colour=stitch::Vec3(-0.348006760f, 1.490979000f, -0.165818080f); else
    if (lambda<552.50000f) colour=stitch::Vec3(-0.129401420f, 1.446930640f, -0.169687060f); else
    if (lambda<557.50000f) colour=stitch::Vec3(0.119474080f, 1.379909800f, -0.169502340f); else
    if (lambda<562.50000f) colour=stitch::Vec3(0.395116960f, 1.290592840f, -0.165803500f); else
    if (lambda<567.50000f) colour=stitch::Vec3(0.692848540f, 1.178460640f, -0.159061460f); else
    if (lambda<572.50000f) colour=stitch::Vec3(1.005314240f, 1.047412040f, -0.149615540f); else
    if (lambda<577.50000f) colour=stitch::Vec3(1.322309060f, 0.900814280f, -0.137996000f); else
    if (lambda<582.50000f) colour=stitch::Vec3(1.631342680f, 0.744112760f, -0.124736820f); else
    if (lambda<587.50000f) colour=stitch::Vec3(1.915964940f, 0.582977920f, -0.110635240f); else
    if (lambda<592.50000f) colour=stitch::Vec3(2.161878040f, 0.425487800f, -0.096203020f); else
    if (lambda<597.50000f) colour=stitch::Vec3(2.355926840f, 0.279520360f, -0.081950080f); else
    if (lambda<602.50000f) colour=stitch::Vec3(2.472091920f, 0.154305040f, -0.068820080f); else
    if (lambda<607.50000f) colour=stitch::Vec3(2.517092120f, 0.049946240f, -0.056857640f); else
    if (lambda<612.50000f) colour=stitch::Vec3(2.475964820f, -0.028079440f, -0.046550340f); else
    if (lambda<617.50000f) colour=stitch::Vec3(2.362953800f, -0.081797760f, -0.037618360f); else
    if (lambda<622.50000f) colour=stitch::Vec3(2.183261280f, -0.113320160f, -0.030007960f); else
    if (lambda<627.50000f) colour=stitch::Vec3(1.941732140f, -0.126056720f, -0.023600460f); else
    if (lambda<632.50000f) colour=stitch::Vec3(1.674607400f, -0.125474080f, -0.018342560f); else
    if (lambda<637.50000f) colour=stitch::Vec3(1.422682100f, -0.118117480f, -0.014138360f); else
    if (lambda<642.50000f) colour=stitch::Vec3(1.182598900f, -0.105804680f, -0.010796760f); else
    if (lambda<647.50000f) colour=stitch::Vec3(0.956884120f, -0.090424160f, -0.008132320f); else
    if (lambda<652.50000f) colour=stitch::Vec3(0.754321700f, -0.074036200f, -0.006065400f); else
    if (lambda<657.50000f) colour=stitch::Vec3(0.583354860f, -0.058882440f, -0.004486680f); else
    if (lambda<662.50000f) colour=stitch::Vec3(0.440659500f, -0.045385080f, -0.003275560f); else
    if (lambda<667.50000f) colour=stitch::Vec3(0.324241160f, -0.033797440f, -0.002359680f); else
    if (lambda<672.50000f) colour=stitch::Vec3(0.234066600f, -0.024676080f, -0.001668560f); else
    if (lambda<677.50000f) colour=stitch::Vec3(0.170459920f, -0.018117920f, -0.001196640f); else
    if (lambda<682.50000f) colour=stitch::Vec3(0.125543000f, -0.013466560f, -0.000865920f); else
    if (lambda<687.50000f) colour=stitch::Vec3(0.088333840f, -0.009562280f, -0.000598360f); else
    if (lambda<692.50000f) colour=stitch::Vec3(0.060964020f, -0.006617640f, -0.000410680f); else
    if (lambda<697.50000f) colour=stitch::Vec3(0.042444620f, -0.004620160f, -0.000284320f); else
    if (lambda<702.50000f) colour=stitch::Vec3(0.030644060f, -0.003357280f, -0.000202560f); else
    if (lambda<707.50000f) colour=stitch::Vec3(0.021793640f, -0.002410120f, -0.000141240f); else
    if (lambda<712.50000f) colour=stitch::Vec3(0.015569260f, -0.001681760f, -0.000105920f); else
    if (lambda<717.50000f) colour=stitch::Vec3(0.010982000f, -0.001159720f, -0.000078040f); else
    if (lambda<722.50000f) colour=stitch::Vec3(0.007861500f, -0.000934680f, -0.000042760f); else
    if (lambda<727.50000f) colour=stitch::Vec3(0.005405820f, -0.000625200f, -0.000031600f); else
    if (lambda<732.50000f) colour=stitch::Vec3(0.003768700f, -0.000418880f, -0.000024160f); else
    if (lambda<737.50000f) colour=stitch::Vec3(0.002626040f, -0.000218800f, -0.000026000f); else
    if (lambda<742.50000f) colour=stitch::Vec3(0.001961220f, -0.000303240f, -0.000001880f); else
    if (lambda<747.50000f) colour=stitch::Vec3(0.001313020f, -0.000109400f, -0.000013000f); else
    if (lambda<752.50000f) colour=stitch::Vec3(0.000818560f, -0.000103160f, -0.000003720f); else
    if (lambda<757.50000f) colour=stitch::Vec3(0.000494460f, -0.000006240f, -0.000009280f); else
    if (lambda<762.50000f) colour=stitch::Vec3(0.000494460f, -0.000006240f, -0.000009280f); else
    if (lambda<767.50000f) colour=stitch::Vec3(0.000324100f, -0.000096920f, 0.000005560f); else
    if (lambda<772.50000f) colour=stitch::Vec3(0.000324100f, -0.000096920f, 0.000005560f); else
    if (lambda<777.50000f) colour=stitch::Vec3(0.000324100f, -0.000096920f, 0.000005560f); else
    colour=stitch::Vec3(0.000000000f, 0.000000000f, 0.000000000f);

    colour.clamp(0.0f, 1.0f);
    
    return colour;
}

