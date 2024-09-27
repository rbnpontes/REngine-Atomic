#pragma once
#include "./Container/Index.h"
#include "./Core/Index.h"
#include "./Audio/Index.h"
#if defined(ENGINE_DATABASE_SQLITE) || defined(ENGINE_DATABASE_ODBC)
    #include "./Database/Index.h"
#endif
#include "./Engine/Index.h"
#include "./Resource/Index.h"
#include "./Graphics/Index.h"
#include "./RHI/Index.h"
#include "./Input/Index.h"
#include "./IO/Index.h"
#if defined(ENGINE_IPC)
    #include "./IPC/Index.h"
#endif
#include "./Math/Index.h"
#include "./Metrics/Index.h"
#if defined(ENGINE_NETWORK)
    #include "./Network/Index.h"
#endif
#include "./Scene/Index.h"
#include "./Physics/Index.h"
#include "./Navigation/Index.h"
#include "./Environment/Index.h"
#include "./Script/Index.h"
#include "./UI/Index.h"
//#include "./Web/Index.h"
#if defined(ENGINE_2D) || defined(ENGINE_2D_ONLY)
    #include "./2D/Index.h"
#endif
#if defined(ENGINE_IK)
    #include "./IK/IK.h"
#endif