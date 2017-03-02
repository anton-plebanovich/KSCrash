//
//  KSCrashSentry_User.c
//
//  Copyright (c) 2012 Karl Stenerud. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall remain in place
// in this source code.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "KSCrashSentry_User.h"
#include "KSCrashSentry_Context.h"
#include "KSCrashSentry_Private.h"
#include "KSMach.h"

//#define KSLogger_LocalLevel TRACE
#include "KSLogger.h"

#include <execinfo.h>
#include <stdlib.h>


/** Context to fill with crash information. */
static KSCrash_SentryContext* g_context;


bool kscrashsentry_installUserExceptionHandler(KSCrash_SentryContext* const context)
{
    KSLOG_DEBUG("Installing user exception handler.");
    g_context = context;
    return true;
}

void kscrashsentry_uninstallUserExceptionHandler(void)
{
    KSLOG_DEBUG("Uninstalling user exception handler.");
    g_context = NULL;
}

void kscrashsentry_reportUserException(const char* name,
                                       const char* reason,
                                       const char* language,
                                       const char* lineOfCode,
                                       const char* stackTrace,
                                       bool terminateProgram)
{
    if(g_context == NULL)
    {
        KSLOG_WARN("User-reported exception sentry is not installed. Exception has not been recorded.");
    }
    else
    {
        kscrashsentry_beginHandlingCrash(g_context);

        KSLOG_DEBUG("Filling out context.");
        g_context->crashType = KSCrashTypeUserReported;
        g_context->offendingThread = ksmach_thread_self();
        g_context->registersAreValid = false;
        g_context->crashReason = reason;
        g_context->stackTrace = 0;
        g_context->stackTraceLength = 0;
        g_context->userException.name = name;
        g_context->userException.language = language;
        g_context->userException.lineOfCode = lineOfCode;
        g_context->userException.customStackTrace = stackTrace;

        KSLOG_DEBUG("Calling main crash handler.");
        g_context->onCrash();

        if(terminateProgram)
        {
            kscrashsentry_uninstall(KSCrashTypeAll);
            abort();
        }
        else
        {
            kscrashsentry_clearContext(g_context);
        }
    }
}
