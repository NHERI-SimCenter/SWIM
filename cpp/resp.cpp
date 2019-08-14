/* *****************************************************************************
Copyright (c) 2018-2019, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

#include "resp.h"
#include <QVector>

Resp::Resp()
{
    steps = 0;
    size = 0;
    data = NULL;
}

Resp::Resp(int inSize, int inSteps)
{
    size = inSize;
    steps = inSteps;

    data = new QVector<double> *[size];
    for (int j=0; j<size; j++) {
        data[j] = new QVector<double>(steps,0.);
    }
}

Resp::~Resp()
{
    if (data != NULL) {
        for (int j=0; j < size; j++) {
            delete data[j];
        }
        delete [] data;
    }
}

void Resp::reStep(int newSteps)
{
    steps = newSteps;
    for (int j=0; j<steps; j++) {
        data[j]->resize(newSteps);
    }
    zero();
}

void Resp::reSize(int newSize)
{
    if (data != NULL) {
        for (int j=0; j < size; j++) {
            delete data[j];
        }
        delete [] data;
    }

    size = newSize;

    data = new QVector<double> *[size];
    for (int j=0; j<size; j++) {
        data[j] = new QVector<double>(steps,0.);
    }
}

void Resp::reSize(int newSize, int newSteps)
{
    if (data != NULL) {
        for (int j=0; j < size; j++) {
            delete data[j];
        }
        delete [] data;
    }

    size = newSize;
    steps = newSteps;

    data = new QVector<double> *[size];
    for (int j=0; j<size; j++) {
        data[j] = new QVector<double>(steps,0.);
    }
}

void Resp::zero()
{
    for (int j=0; j<size; j++) {
        for (int k=0; k<steps; k++) {
            (*data[j])[k] = 0.;
        }
    }
}
