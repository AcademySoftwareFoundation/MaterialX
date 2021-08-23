//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TIMER_H
#define MATERIALX_TIMER_H

/// @file
/// Support for event timing

#include <MaterialXRender/Export.h>

#include <chrono>

namespace MaterialX
{

/// @class ScopedTimer
/// A class for scoped event timing
class MX_RENDER_API ScopedTimer
{
  public:
    using clock = std::chrono::high_resolution_clock;

    ScopedTimer(double* externalCounter = nullptr);
    ~ScopedTimer();

    /// Return the elapsed time in seconds since our start time.
    double elapsedTime();

    /// Set our start time to the current moment.
    void startTimer();

    /// Add the current elapsed time to our external counter, and reset
    /// our start time to the current moment.
    void endTimer();

  protected:
    double* _externalCounter;
    std::chrono::time_point<clock> _startTime;
};

} // namespace MaterialX

#endif
