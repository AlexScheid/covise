/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef BEZIERGRID_H
#define BEZIERGRID_H

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++                                                        (C)2000 RUS  ++
// ++ Description: "Hello, world!" in COVISE API                          ++
// ++                                                                     ++
// ++ Author:                                                             ++
// ++                                                                     ++
// ++                            Andreas Werner                           ++
// ++               Computer Center University of Stuttgart               ++
// ++                            Allmandring 30                           ++
// ++                           70550 Stuttgart                           ++
// ++                                                                     ++
// ++ Date:  10.01.2000  V2.0                                             ++
// ++**********************************************************************/

#include <api/coModule.h>
#include <math.h>
class BezierGrid : public covise::coModule
{

private:
    virtual int compute(const char *port);
    covise::coStringParam *m_message;
    covise::coOutputPort *pGridOut;
    covise::coInputPort *pGridIn;
    vector<float> casteljauAproximation(std::vector<vector<float> > points, float t);

public:
    BezierGrid(int argc, char *argv[]);

};
#endif













