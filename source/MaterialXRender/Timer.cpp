//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Timer.h>

MATERIALX_NAMESPACE_BEGIN

ScopedTimer::ScopedTimer(double* externalCounter) :
    _externalCounter(externalCounter)
{
    startTimer();
}

ScopedTimer::~ScopedTimer()
{
    endTimer();
}

double ScopedTimer::elapsedTime()
{
    std::chrono::time_point<clock> endTime = clock::now();
    std::chrono::duration<double> elapsedTime = endTime - _startTime;
    return elapsedTime.count();
}

void ScopedTimer::startTimer()
{
    _active = true;
    _startTime = clock::now();
}

void ScopedTimer::endTimer()
{
    if (_active && _externalCounter)
    {
        *_externalCounter += elapsedTime();
    }
    _active = false;
}

MATERIALX_NAMESPACE_END
