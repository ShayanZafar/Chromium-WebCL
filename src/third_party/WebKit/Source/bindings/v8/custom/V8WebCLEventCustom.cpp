/*
 * Copyright (C) 2012 Intel Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE
 * INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(WEBCL)


#include "V8WebCLCustom.h"
#include "V8WebCLEvent.h"

namespace WebCore {

void V8WebCLEvent::getInfoMethodCustom(const v8::Arguments& args)
{
  
    if (args.Length() != 1)
        { throwNotEnoughArgumentsError(args.GetIsolate()); return; }

    ExceptionState es(args.GetIsolate());
    WebCLEvent* event = V8WebCLEvent::toNative(args.Holder()); 
    int event_index = toInt32(args[0]);
    WebCLGetInfo info = event->getInfo(event_index, es);

    v8SetReturnValue(args, toV8Object(info,args.Holder(), args.GetIsolate()));
}

void V8WebCLEvent::getProfilingInfoMethodCustom(const v8::Arguments& args)
{
  
    if (args.Length() != 1)
        { throwNotEnoughArgumentsError(args.GetIsolate()); return; }

    ExceptionState es(args.GetIsolate());
    WebCLEvent* event = V8WebCLEvent::toNative(args.Holder()); 
    int profiling_index = toInt32(args[0]);
    WebCLGetInfo info = event->getProfilingInfo(profiling_index, es);

    v8SetReturnValue(args, toV8Object(info,args.Holder(), args.GetIsolate()));
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
