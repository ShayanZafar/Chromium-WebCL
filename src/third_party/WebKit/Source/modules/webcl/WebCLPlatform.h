/*
* Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided the following conditions
* are met:
* 
* 1.  Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
* 
* 2.  Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
* 
* THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS CORPORATION AND ITS
* CONTRIBUTORS "AS IS", AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING
* BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG
* ELECTRONICS CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS, OR BUSINESS INTERRUPTION), HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
* NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WebCLPlatform_h
#define WebCLPlatform_h

#ifndef THIRD_PARTY_WEBKIT_MODULES_WEBCL // ScalableVision to avoid conflict between TraceEvent.h and trace_event.h
#define THIRD_PARTY_WEBKIT_MODULES_WEBCL
#endif

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
//#include <PlatformString.h>
#include <wtf/Vector.h>
#include <CL/opencl.h>

#include "WebCLGetInfo.h"
#include "WebCLDeviceList.h"

namespace WebCore {

class WebCL;

class WebCLPlatform : public RefCounted<WebCLPlatform> {
public:
	virtual ~WebCLPlatform();
	static PassRefPtr<WebCLPlatform> create(WebCL* context, cl_platform_id platform_id);
	WebCLGetInfo getInfo (int, ExceptionState&);
	PassRefPtr<WebCLDeviceList> getDevices(int, ExceptionState&);
	Vector<String> getSupportedExtensions(ExceptionState&);
	cl_platform_id getCLPlatform();

private:
	WebCLPlatform(WebCL* context, cl_platform_id platform_id);
	WebCL* m_context;
	cl_platform_id m_cl_platform_id;
};

} // namespace WebCore

#endif // WebCLPlatform_h
