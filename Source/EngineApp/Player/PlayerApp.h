//
// Copyright (c) 2014-2016 THUNDERBEAST GAMES LLC
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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../AppBase.h"

namespace Atomic
{
    /// Player launch mode
    enum PlayerLaunchMode
    {
        /// Standalone redistributable player
        PLAYER_LAUNCH_STANDALONE = 0,
        /// Editor binary play mode
        PLAYER_LAUNCH_ATOMICEDITOR,
        /// Launched from an external IDE such as Visual Studio
        PLAYER_LAUNCH_EXTERNALEDITOR
    };

    class PlayerApp : public AppBase
    {
        ATOMIC_OBJECT(PlayerApp, AppBase)

    public:
        /// Construct.
        PlayerApp(Context* context);
        virtual ~PlayerApp();

        /// Setup before engine initialization. 
        virtual void Setup();
        /// Setup after engine initialization. L
        virtual void Start();
        /// Cleanup after the main loop. 
        virtual void Stop();

        virtual void ProcessArguments();

        /// Get the player launch mode
        static PlayerLaunchMode GetLaunchMode() { return launchMode_; }

    protected:

        /// Set the player launch mode
        static void SetLaunchMode(PlayerLaunchMode launchMode) { launchMode_ = launchMode; }

        static PlayerLaunchMode launchMode_;

        bool executeJSMain_;

    private:

        bool playerMetrics_;

    };

}
