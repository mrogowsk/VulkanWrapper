//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Timer.h"

float Timer::s_clockTime = 0.0;

Timer::Timer()
{
    if (s_clockTime == 0.0)
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);

        s_clockTime = 1.0f / freq.QuadPart;
    }

    reset();
}

void Timer::reset()
{
    QueryPerformanceCounter(&startTime_);
    numFrames_ = 0;
    lastTick_ = 0.0f;
    timeSum_ = 0.0f;
}

float Timer::getTime()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);

    return s_clockTime * (time.QuadPart - startTime_.QuadPart);
}

void Timer::tick()
{
    float time = getTime();
    timeSum_ += time - lastTick_;
    lastTick_ = time;
    ++numFrames_;
}

float Timer::getFps()
{
    return static_cast<float>(numFrames_) / timeSum_;
}