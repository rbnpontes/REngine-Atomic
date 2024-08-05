//
// Copyright (c) 2014-2015, THUNDERBEAST GAMES LLC All rights reserved
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

#include <EngineCore/Core/Context.h>
#include <EngineCore/IO/Log.h>
#include <EngineCore/IO/File.h>
#include <EngineCore/IO/FileSystem.h>
#include <EngineCore/Resource/JSONFile.h>
#include <EngineCore/Graphics/GraphicsDefs.h>

#include "AssetBuildConfig.h"

namespace Atomic
{
    AssetBuildConfig AssetBuildConfig::assetBuildConfig_;

    bool AssetBuildConfig::LoadAssetBuildTagArray(const JSONArray& tags)
    {
        if (tags.Empty())
            return false;

        for (auto it = tags.Begin(); it < tags.End(); ++it)
        {
            const JSONObject& resourceTag = (*it).GetObject();

            String tag = resourceTag["Tag"]->GetString();

            const JSONArray& resources = resourceTag["Resources"]->GetArray();

            StringVector value;
            GetArrayValue(resources, value);
            valueMap_[tag.CString()] = value;
        }

        return true;
    }

    bool AssetBuildConfig::LoadDesktopConfig(JSONValue root)
    {
        const JSONValue& assetBuildConfigRoot = root["AssetBuildConfig"];

        if (!assetBuildConfigRoot.IsArray())
            return false;

        const JSONArray& assetTags = assetBuildConfigRoot.GetArray();

        if (!assetTags.Empty())
            LoadAssetBuildTagArray(assetTags);

        return true;
    }

}
