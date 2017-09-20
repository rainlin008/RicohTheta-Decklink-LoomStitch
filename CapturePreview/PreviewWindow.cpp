﻿/* -LICENSE-START-
** Copyright (c) 2013 Blackmagic Design
**
** Permission is hereby granted, free of charge, to any person or organization
** obtaining a copy of the software and accompanying documentation covered by
** this license (the "Software") to use, reproduce, display, distribute,
** execute, and transmit the Software, and to prepare derivative works of the
** Software, and to permit third-parties to whom the Software is furnished to
** do so, all subject to the following:
**
** The copyright notices in the Software and this entire statement, including
** the above license grant, this restriction and the following disclaimer,
** must be included in all copies of the Software, in whole or in part, and
** all derivative works of the Software, unless such copies or derivative
** works are solely in the form of machine-executable object code generated by
** a source language processor.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
** SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
** FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
** -LICENSE-END-
*/

#include "loom_shell_util.h"
#include "stdafx.h"
#include <gl/gl.h>
#include "PreviewWindow.h"
#include "loom_shell.h"


extern  vx_status createOpenCLContext(const char * platform, const char * device, cl_context * opencl_context);
extern  vx_status createBuffer(cl_context opencl_context, vx_uint32 size, cl_mem * mem);
extern  int64_t GetClockCounter();
extern  int64_t GetClockCounter();
extern  int64_t GetClockFrequency();
extern   cl_command_queue GetCmdqCached(cl_mem mem);
extern  vx_status loadBuffer(cl_mem mem, const char * fileName);
extern  vx_status saveBuffer(cl_mem mem, const char * fileName);
extern vx_status saveBufferToImage(cl_mem mem, const char * fileName, vx_df_image buffer_format, vx_uint32 buffer_width, vx_uint32 buffer_height, vx_uint32 stride_in_bytes);
extern   vx_status setGlobalAttribute(vx_uint32 offset, float value);


static int UYVY422ToRGB888(unsigned char *src_buffer, int w, int h, unsigned char *des_buffer)
{
	unsigned char *yuv, *rgb;


	yuv = src_buffer;
	rgb = des_buffer;

	if (yuv == NULL || rgb == NULL)
	{
		printf("error: input data null!\n");
		return -1;
	}

	int size = w * h;
	int u, v, y1, y2;
	int r, g, b;
	for (int i = 0; i < size; i += 2)
	{
		u = (int)yuv[2 * i];
		y1 = (int)yuv[2 * i + 1];
		v = (int)yuv[2 * i + 2];
		y2 = (int)yuv[2 * i + 3];

		u -= 128;
		v -= 128;

		r = y1 + u + ((104 * u) >> 8);
		r = (r > 255) ? 255 : r;
		r = (r <   0) ? 0 : r;
		g = y1 - (89 * v >> 8) - ((183 * u) >> 8);
		g = (g > 255) ? 255 : g;
		g = (g <   0) ? 0 : g;
		b = y1 + v + ((199 * v) >> 8);
		b = (b > 255) ? 255 : b;
		b = (b <   0) ? 0 : b;
		rgb[3 * i] = (unsigned char)r;
		rgb[3 * i + 1] = (unsigned char)g;
		rgb[3 * i + 2] = (unsigned char)b;

		r = y2 + u + ((104 * u) >> 8);
		r = (r > 255) ? 255 : r;
		r = (r <   0) ? 0 : r;
		g = y2 - (89 * v >> 8) - ((183 * u) >> 8);
		g = (g > 255) ? 255 : g;
		g = (g <   0) ? 0 : g;
		b = y2 + v + ((199 * v) >> 8);
		b = (b > 255) ? 255 : b;
		b = (b <   0) ? 0 : b;
		rgb[3 * i + 3] = (unsigned char)r;
		rgb[3 * i + 4] = (unsigned char)g;
		rgb[3 * i + 5] = (unsigned char)b;

	}
	return 0;


}

PreviewWindow::PreviewWindow(CCapturePreviewDlg* ui)
	: m_deckLinkScreenPreviewHelper(NULL), m_refCount(1), m_previewBox(NULL), m_previewBoxDC(NULL), m_openGLctx(NULL), m_uiDelegate(ui)
{
	//m_uiDelegate->m_selectedDevice->
	//// initialize default counts
	//decl_ls_disabled = false;
	//decl_vx_disabled = false;
	//decl_cl_disabled = false;
	//decl_buf_disabled = false;
	//num_context_ = DEFAULT_LS_CONTEXT_COUNT;
	//num_openvx_context_ = DEFAULT_VX_CONTEXT_COUNT;
	//num_opencl_context_ = DEFAULT_CL_CONTEXT_COUNT;
	//num_opencl_buf_ = DEFAULT_CL_BUFFER_COUNT;
	//// set log callback
	////lsGlobalSetLogCallback(stitch_log_callback);
	//// name of contexts
	//strcpy(name_ls, "ls");
	//strcpy(name_vx, "vx");
	//strcpy(name_cl, "cl");
	//strcpy(name_buf, "buf");
	//// create array for contexts, cmd_queues, and buffers
	//context_ = new ls_context[num_context_]();
	//openvx_context_ = new vx_context[num_openvx_context_]();
	//opencl_context_ = new cl_context[num_opencl_context_]();
	//openvx_context_allocated_ = new bool[num_openvx_context_]();
	//opencl_context_allocated_ = new bool[num_opencl_context_]();
	//opencl_buf_mem_ = new cl_mem[num_opencl_buf_]();
	//// misc
	//memset(attr_buf_, 0, sizeof(attr_buf_));






}

PreviewWindow::~PreviewWindow()
{
	if (m_deckLinkScreenPreviewHelper != NULL)
	{
		m_deckLinkScreenPreviewHelper->Release();
		m_deckLinkScreenPreviewHelper = NULL;
	}

	if (m_openGLctx != NULL)
	{
		wglDeleteContext(m_openGLctx);
		m_openGLctx = NULL;
	}

	if (m_previewBoxDC != NULL)
	{
		m_previewBox->ReleaseDC(m_previewBoxDC);
		m_previewBoxDC = NULL;
	}
	//int contextIndex = 0;
	//vx_status vx_status = lsReleaseContext(&context_[contextIndex]);
	//if (!vx_status)
	//	//return Error("ERROR: lsReleaseContext(%s[%d]) failed (%d)", name_ls, contextIndex, vx_status);
	//Message("..lsReleaseContext: released context %s[%d]\n", name_ls, contextIndex);
	//if (context_) delete[] context_;
	//if (opencl_buf_mem_) delete[] opencl_buf_mem_;
	//if (openvx_context_) delete[] openvx_context_;
	//if (opencl_context_) delete[] opencl_context_;
	//if (openvx_context_allocated_) delete[] openvx_context_allocated_;
	//if (opencl_context_allocated_) delete[] opencl_context_allocated_;






}

bool		PreviewWindow::init(CStatic *previewBox)
{
	m_previewBox = previewBox;

	// Create the DeckLink screen preview helper
	if (CoCreateInstance(CLSID_CDeckLinkGLScreenPreviewHelper, NULL, CLSCTX_ALL, IID_IDeckLinkGLScreenPreviewHelper, (void**)&m_deckLinkScreenPreviewHelper) != S_OK)
		return false;

	// Initialise OpenGL
	return initOpenGL();
}


//bool		PreviewWindow::initStitching()
//{ 
// 
//
//	//unsigned char des_buffer[1920 * 1080 * 3];
// 
// 
//
//	// initialize
//	decl_ls_disabled = false;
//	decl_vx_disabled = false;
//	decl_cl_disabled = false;
//	decl_buf_disabled = false;
//	num_context_ = DEFAULT_LS_CONTEXT_COUNT;
//	num_openvx_context_ = DEFAULT_VX_CONTEXT_COUNT;
//	num_opencl_context_ = DEFAULT_CL_CONTEXT_COUNT;
//	num_opencl_buf_ = DEFAULT_CL_BUFFER_COUNT;
//	// set log callback
//	//lsGlobalSetLogCallback(stitch_log_callback);
//	// name of contexts
//	strcpy(name_ls, "ls");
//	strcpy(name_vx, "vx");
//	strcpy(name_cl, "cl");
//	strcpy(name_buf, "buf");
//	// create array for contexts, cmd_queues, and buffers
//	context_ = new ls_context[num_context_]();
//	openvx_context_ = new vx_context[num_openvx_context_]();
//	opencl_context_ = new cl_context[num_opencl_context_]();
//	openvx_context_allocated_ = new bool[num_openvx_context_]();
//	opencl_context_allocated_ = new bool[num_opencl_context_]();
//	opencl_buf_mem_ = new cl_mem[num_opencl_buf_]();
//	// misc
//	memset(attr_buf_, 0, sizeof(attr_buf_));
//
//
//	//LIVE_STITCH_API_ENTRY vx_status VX_API_CALL lsGlobalSetAttributes(vx_uint32 attr_offset, vx_uint32 attr_count, const vx_float32 * attr_ptr)
//	//1 setGlobalAttribute(0,2); // process the command
//
//	vx_uint32 attr_offset = 0;
//	float value = 2.000f;
//	if (setGlobalAttribute(attr_offset, value) != VX_SUCCESS)
//		return -1;
//
//	//2 cl_context clc;	//name_cl = "clc" // process the command
//	strcpy(name_cl, "clc");
//	num_opencl_context_ = 1;
//	Message("..cl_context %s[%d] created\n", name_cl, num_opencl_context_);
//
//	//3 createOpenCLContext("Advanced Micro Devices, Inc.", "0", &clc);
//	char platform[64], device[64];
//	vx_uint32 clIndex = 0;
//	strcpy(platform, "Advanced Micro Devices, Inc.");
//
//
//	strcpy(device, "0");
//	cl_int cl_status = createOpenCLContext(platform, device, &opencl_context_[clIndex]);
//	if (cl_status)
//		return cl_status;
//	opencl_context_allocated_[clIndex] = true;
//
//
//
//
//	//4 cl_mem buf[2];// process the command
//	num_opencl_buf_ = 2;
//	strcpy(name_buf, "buf");
//	Message("..cl_mem %s[%d] created\n", name_buf, num_opencl_buf_);
//
//	//5 createBuffer(clc, 6220800, &buf[0]);
//	clIndex = 0;
//	vx_uint32 bufIndex = 0, bufSize = 6220800;
//	strcpy(name_cl, "clc");
//	clIndex = 0;
//	if (clIndex >= num_opencl_context_) return Error("ERROR: OpenCL context out-of-range: expects: 0..%d", num_opencl_context_ - 1);
//	if (!opencl_context_[clIndex]) return Error("ERROR: OpenCL context %s[%d] doesn't exist", name_cl, clIndex);
//	if (bufIndex >= num_opencl_buf_) return Error("ERROR: OpenCL buffer out-of-range: expects: 0..%d", num_opencl_buf_ - 1);
//	if (opencl_buf_mem_[bufIndex]) return Error("ERROR: OpenCL buffer %s[%d] already exists", name_buf, bufIndex);
//	cl_status = createBuffer(opencl_context_[clIndex], bufSize, &opencl_buf_mem_[bufIndex]);
//	if (cl_status)
//		return cl_status;
//	//6 createBuffer(clc, 22118400, &buf[1]);
//	clIndex = 0;
//	bufSize = 22118400;
//	bufIndex = 1;
//	if (clIndex >= num_opencl_context_) return Error("ERROR: OpenCL context out-of-range: expects: 0..%d", num_opencl_context_ - 1);
//	if (!opencl_context_[clIndex]) return Error("ERROR: OpenCL context %s[%d] doesn't exist", name_cl, clIndex);
//	if (bufIndex >= num_opencl_buf_) return Error("ERROR: OpenCL buffer out-of-range: expects: 0..%d", num_opencl_buf_ - 1);
//	if (opencl_buf_mem_[bufIndex]) return Error("ERROR: OpenCL buffer %s[%d] already exists", name_buf, bufIndex);
//	cl_status = createBuffer(opencl_context_[clIndex], bufSize, &opencl_buf_mem_[bufIndex]);
//	if (cl_status)
//		return cl_status;
//
//	// 7 ls_context lsc;
//	strcpy(name_ls, "lsc");
//	num_context_ = 1;
//	//8 lsc = lsCreateContext();
//	// process the command
//	vx_uint32 contextIndex = 0;
//	num_context_ = 1;
//	if (contextIndex >= num_context_) return Error("ERROR: context out-of-range: expects: 0..%d", num_context_ - 1);
//	if (context_[contextIndex]) return Error("ERROR: context %s[%d] already exists", name_ls, contextIndex);
//	context_[contextIndex] = lsCreateContext();
//	if (!context_[contextIndex]) return Error("ERROR: lsCreateContext() failed");
//	Message("..lsCreateContext: created context %s[%d]\n", name_ls, contextIndex);
//
//	//9 lsSetOpenCLContext(lsc, clc);
//	contextIndex = 0;
//	clIndex = 0;
//	//ParseContextWithErrorCheck(s, contextIndex, invalidSyntax));
//	if (contextIndex >= num_context_)
//		return false;
//
//	if (!context_[contextIndex])
//		return false;
//	//process the command
//	vx_status vx_status = lsSetOpenCLContext(context_[contextIndex], opencl_context_[clIndex]);
//	if (vx_status) return Error("ERROR: lsSetOpenCLContext(%s[%d],%s[%d]) failed (%d)", name_ls, contextIndex, name_cl, clIndex, vx_status);
//	Message("..lsSetOpenCLContext: set OpenCL context %s[%d] for %s[%d]\n", name_cl, clIndex, name_ls, contextIndex);
//
//	//10 lsSetCameraConfig(lsc, 1, 2, RGB2, 1920, 1080);
//	vx_uint32  camera_rows = 0, camera_cols = 0, buffer_width = 0, buffer_height = 0;
//	vx_df_image buffer_format = VX_DF_IMAGE_RGB;
//	//ParseContextWithErrorCheck
//	if (contextIndex >= num_context_) {
//
//		return false;
//	}
//	if (!context_[contextIndex]) {
//
//		return false;
//	}
//	contextIndex = 0;
//	camera_rows = 1;
//	camera_cols = 2;
//	buffer_format = 0x32424752;
//	buffer_width = 1920;
//	buffer_height = 1080;
//	//process the command
//	vx_status = lsSetCameraConfig(context_[contextIndex], camera_rows, camera_cols, buffer_format, buffer_width, buffer_height);
//	if (vx_status) return Error("ERROR: lsSetCameraConfig(%s[%d],*) failed (%d)", name_ls, contextIndex, vx_status);
//	Message("..lsSetCameraConfig: successful for %s[%d]\n", name_ls, contextIndex);
//
//	//11 lsSetOutputConfig(lsc, RGB2, 3840, 1920);// process the command
//	contextIndex = 0;
//	//ParseContextWithErrorCheck(s, contextIndex, invalidSyntax));
//	if (contextIndex >= num_context_)
//		return false;
//	if (!context_[contextIndex])
//		return false;
//	buffer_format = VX_DF_IMAGE_RGB;
//	buffer_width = 3840;
//	buffer_height = 1920;
//	vx_status = lsSetOutputConfig(context_[contextIndex],
//		buffer_format, buffer_width, buffer_height);
//	if (vx_status) return Error("ERROR: lsSetOutputConfig(%s[%d],*) failed (%d)", name_ls, contextIndex, vx_status);
//	Message("..lsSetOutputConfig: successful for %s[%d]\n", name_ls, contextIndex);
//
//	//12 lsSetCameraBufferStride(lsc, 2880);
//	vx_uint32 buffer_stride_in_bytes = 2880;
//	contextIndex = 0;
//	vx_status = lsSetCameraBufferStride(context_[contextIndex], buffer_stride_in_bytes);
//	if (vx_status) return Error("ERROR: lsSetCameraBufferStride(%s[%d],*) failed (%d)", name_ls, contextIndex, vx_status);
//	Message("..lsSetCameraBufferStride: successful for %s[%d]\n", name_ls, contextIndex);
//
//
//	//13 lsSetOutputBufferStride(lsc, 11520);
//	// process the command
//	contextIndex = 0;
//	buffer_stride_in_bytes = 11520;
//	vx_status = lsSetOutputBufferStride(context_[contextIndex], buffer_stride_in_bytes);
//	if (vx_status) return Error("ERROR: lsSetOutputBufferStride(%s[%d],*) failed (%d)", name_ls, contextIndex, vx_status);
//	Message("..lsSetOutputBufferStride: successful for %s[%d]\n", name_ls, contextIndex);
//
//	//14 rig_params rig_par = { 0,0,0,0 };	// process the command
//
//	char parName[64];
//	rig_params rig_par = { 0 };
//	rig_par.yaw = 0.0f;
//	rig_par.pitch = 0.0f;
//	rig_par.roll = 0.0f;
//	rig_par.d = 0.0f;
//	strcpy(parName, "rig_par");
//	if (rigParList.find(parName) != rigParList.end()) return Error("ERROR: rig_params %s already exists", parName);
//
//	rigParList[parName] = rig_par;
//	Message("..rig_params %s declared\n", parName);
//
//	//15 lsSetRigParams(lsc, &rig_par);
//	contextIndex = 0;
//	vx_status = lsSetRigParams(context_[contextIndex], &rig_par);
//	if (vx_status) return Error("ERROR: lsSetRigParams(%s[%d],*) failed (%d)", name_ls, contextIndex, vx_status);
//	Message("..lsSetRigParams: successful for %s[%d]\n", name_ls, contextIndex);
//
//	//16 camera_params cam_par0 = { { 0,0,90,0,0,0 },{ 180,842.2,421.1,0,-60,ptgui_lens_fisheye_circ,-0.190745733,0.189054235,0.998701352 } };
//	char   lensType[64];
//	camera_params camera_par = { 0 };
//	strcpy(parName, "cam_par0");
//	camera_par.focal.yaw = 0.0f;
//	camera_par.focal.pitch = 0.0f;
//	camera_par.focal.roll = 90.0f;
//	camera_par.focal.tx = 0.0f;
//	camera_par.focal.ty = 0.0f;
//	camera_par.focal.tz = 0.0f;
//
//	camera_par.lens.hfov = 180.0f;
//	camera_par.lens.haw = 842.2;
//	camera_par.lens.r_crop = 421.1;
//	camera_par.lens.du0 = 0.0f;
//	camera_par.lens.dv0 = -60.0f;
//	camera_par.lens.lens_type = ptgui_lens_fisheye_circ;
//
//	//strcpy(lensType, "ptgui_lens_fisheye_circ");
//
//	camera_par.lens.k1 = -0.190745733;
//	camera_par.lens.k2 = 0.189054235;
//	camera_par.lens.k3 = 0.998701352;
//	if (camParList.find(parName) != camParList.end()) return Error("ERROR: camera_params %s already exists", parName);
//	// process the command
//	camParList[parName] = camera_par;
//
//
//
//	//17 lsSetCameraParams(lsc, 0, &cam_par0);
//	bool isCamera = true;
//	vx_uint32  index = 0;
//	memcpy(&camera_par, &camParList[parName], sizeof(camera_par));
//	vx_status = lsSetCameraParams(context_[contextIndex], index, &camera_par);
//	if (vx_status) return Error("ERROR: lsSetCameraParams(%s[%d],%d,*) failed (%d)", name_ls, contextIndex, index, vx_status);
//	Message("..lsSetCameraParams: successful for %s[%d] and camera#%d\n", name_ls, contextIndex, index);
//
//	//18 camera_params cam_par1 = { { 180,0,-90,0,0,0 },{ 180,842.2,421.1,0,-60,ptgui_lens_fisheye_circ,-0.190745733,0.189054235,0.998701352 } };
//	strcpy(parName, "cam_par1");
//	camera_par.focal.yaw = 180.0f;
//	camera_par.focal.pitch = 0.0f;
//	camera_par.focal.roll = -90.0f;
//	camera_par.focal.tx = 0.0f;
//	camera_par.focal.ty = 0.0f;
//	camera_par.focal.tz = 0.0f;
//
//	camera_par.lens.hfov = 180.0f;
//	camera_par.lens.haw = 842.2;
//	camera_par.lens.r_crop = 421.1;
//	camera_par.lens.du0 = 0.0f;
//	camera_par.lens.dv0 = -60.0f;
//
//
//	camera_par.lens.lens_type = ptgui_lens_fisheye_circ;
//
//	camera_par.lens.k1 = -0.190745733;
//	camera_par.lens.k2 = 0.189054235;
//	camera_par.lens.k3 = 0.998701352;
//	if (camParList.find(parName) != camParList.end()) return Error("ERROR: camera_params %s already exists", parName);
//	camParList[parName] = camera_par;
//	Message("..camera_params %s declared\n", parName);
//	//19 lsSetCameraParams(lsc, 1, &cam_par1);
//	isCamera = true;
//	index = 1;
//
//	memcpy(&camera_par, &camParList[parName], sizeof(camera_par));
//	vx_status = lsSetCameraParams(context_[contextIndex], index, &camera_par);
//	if (vx_status) return Error("ERROR: lsSetCameraParams(%s[%d],%d,*) failed (%d)", name_ls, contextIndex, index, vx_status);
//	Message("..lsSetCameraParams: successful for %s[%d] and camera#%d\n", name_ls, contextIndex, index);
//
//	//20 lsInitialize(lsc);
//	contextIndex = 0;
//	double clk2msec = 1000.0 / GetClockFrequency();
//	int64_t clk = GetClockCounter();
//	vx_status = lsInitialize(context_[contextIndex]);
//	double msec = clk2msec * (GetClockCounter() - clk);
//	if (vx_status) return Error("ERROR: lsInitialize(%s[%d]) failed (%d)", name_ls, contextIndex, vx_status);
//	Message("..lsInitialize: successful for %s[%d] (%7.3lf ms)\n", name_ls, contextIndex, msec);
//
//	
//
//}


//int PreviewWindow::Error(const char * format, ...)
//{
//	va_list args;
//	va_start(args, format);
//	vprintf(format, args);
//	va_end(args);
//	if (format[strlen(format) - 1] != '\n') printf("\n");
//	fflush(stdout);
//	return -1;
//}
//void PreviewWindow::Message(const char * format, ...)
//{
//	va_list args;
//	va_start(args, format);
//	vprintf(format, args);
//	va_end(args);
//	fflush(stdout);
//}


bool		PreviewWindow::initOpenGL()
{
	PIXELFORMATDESCRIPTOR	pixelFormatDesc;
	int						pixelFormat;
	bool					result = false;

	//
	// Here, we create an OpenGL context attached to the screen preview box
	// so we can use it later on when we need to draw preview frames.

	// Get the preview box drawing context
	m_previewBoxDC = m_previewBox->GetDC();
	if (m_previewBoxDC == NULL)
		return false;

	// Ensure the preview box DC uses ARGB pixel format
	ZeroMemory(&pixelFormatDesc, sizeof(pixelFormatDesc));
	pixelFormatDesc.nSize = sizeof(pixelFormatDesc);
	pixelFormatDesc.nVersion = 1;
	pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDesc.cColorBits = 32;
	pixelFormatDesc.cDepthBits = 16;
	pixelFormatDesc.cAlphaBits = 8;
	pixelFormatDesc.iLayerType = PFD_MAIN_PLANE;
	pixelFormat = ChoosePixelFormat(m_previewBoxDC->m_hDC, &pixelFormatDesc);
	if (SetPixelFormat(m_previewBoxDC->m_hDC, pixelFormat, &pixelFormatDesc) == false)
		return false;

	// Create OpenGL rendering context
	m_openGLctx = wglCreateContext(m_previewBoxDC->m_hDC);
	if (m_openGLctx == NULL)
		return false;

	// Make the new OpenGL context the current rendering context so
	// we can initialise the DeckLink preview helper
	if (wglMakeCurrent(m_previewBoxDC->m_hDC, m_openGLctx) == FALSE)
		return false;

	if (m_deckLinkScreenPreviewHelper->InitializeGL() == S_OK)
		result = true;

	// Reset the OpenGL rendering context
	wglMakeCurrent(NULL, NULL);

	return result;
}

HRESULT			PreviewWindow::DrawFrame(IDeckLinkVideoFrame* theFrame)
{


	//	m_selectedDevice
	/*
	//Kevin Begin
	void * inputBytes = nullptr;
	long rowbytes = theFrame->GetRowBytes();
	long Height = theFrame->GetHeight();
	long Width = theFrame->GetWidth();
	BMDPixelFormat format = theFrame->GetPixelFormat();
	long textureSize = theFrame->GetRowBytes() * theFrame->GetHeight();
	HRESULT 	r = theFrame->GetBytes(&inputBytes);

	UYVY422ToRGB888((unsigned char*)inputBytes, Width, Height, des_buffer);
	textureSize = 3 * Width * Height;
	FILE *fp = NULL;
	int write_length = 0;
	fp = fopen("temp.raw", "wb");
	if (fp == NULL)
	{
	return 0;
	}
	write_length = fwrite(des_buffer, sizeof(char), 1920 * 1080 * 3, fp);//printf("write_length = %d\n", write_length);
	fclose(fp);

	//begin 20161128
	//21 loadBuffer(buf[0], "temp.raw");
	int bufIndex = 0;
	if (bufIndex >= num_opencl_buf_) return Error("ERROR: OpenCL buffer out-of-range: expects: 0..%d", num_opencl_buf_ - 1);
	if (!opencl_buf_mem_[bufIndex]) return Error("ERROR: OpenCL buffer %s[%d] doesn't exist", name_buf, bufIndex);
	vx_status cl_status = loadBuffer(opencl_buf_mem_[bufIndex], "temp.raw");
	if (cl_status) return cl_status;


	//22 lsSetCameraBuffer(lsc, &buf[0]);

	vx_uint32 contextIndex = 0;
	vx_status vx_status = lsSetCameraBuffer(context_[contextIndex], &opencl_buf_mem_[bufIndex]);
	if (vx_status) return Error("ERROR: lsSetCameraBuffer(%s[%d],%s[%d]) failed (%d)", name_ls, contextIndex, name_buf, bufIndex, vx_status);
	Message("..lsSetCameraBuffer: set OpenCL buffer %s[%d] for %s[%d]\n", name_buf, bufIndex, name_ls, contextIndex);

	//23 lsSetOutputBuffer(lsc, &buf[1]);
	contextIndex = 0, bufIndex = 1;
	vx_status = lsSetOutputBuffer(context_[contextIndex], &opencl_buf_mem_[bufIndex]);
	if (vx_status) return Error("ERROR: lsSetOutputBuffer(%s[%d],%s[%d]) failed (%d)", name_ls, contextIndex, name_buf, bufIndex, vx_status);
	Message("..lsSetOutputBuffer: set OpenCL buffer %s[%d] for %s[%d]\n", name_buf, bufIndex, name_ls, contextIndex);

	//24 lsScheduleFrame(lsc);// process the command

	contextIndex = 0;
	const char * invalidSyntax = "ERROR: invalid syntax: expects: lsScheduleFrame(ls[#])";
	vx_status = lsScheduleFrame(context_[contextIndex]);
	if (vx_status) return Error("ERROR: lsScheduleFrame(%s[%d]) failed (%d)", name_ls, contextIndex, vx_status);
	Message("..lsScheduleFrame: successful for %s[%d]\n", name_ls, contextIndex);

	//25 lsWaitForCompletion(lsc);	// process the command
	contextIndex = 0;
	vx_status = lsWaitForCompletion(context_[contextIndex]);
	if (vx_status) return Error("ERROR: lsWaitForCompletion(%s[%d]) failed (%d)", name_ls, contextIndex, vx_status);
	Message("..lsWaitForCompletion: successful for %s[%d]\n", name_ls, contextIndex);

	//26 saveBuffer(buf[1], "result.raw");
	//bufIndex = 1;
	//cl_status = saveBuffer(opencl_buf_mem_[bufIndex], "result.raw");
	//cl_status = saveBuffer(opencl_buf_mem_[bufIndex], "E:\\Loom\\loom20161124\\loom\\utils\\loom_shell\\temp_result2.raw");
	//if (cl_status) return cl_status;
	//saveBufferToImage(buf[1], "nihao2.bmp", RGB2, 3840, 1920, 11520);
	bufIndex = 1;
	char fileName[256];
	strcpy(fileName, "result_bmp.bmp");
	//vx_df_image buffer_format = VX_DF_IMAGE_RGB;
	//vx_uint32 buffer_width = 3840, buffer_height = 1920, stride_in_bytes = 11520;
	cl_status = saveBufferToImage(opencl_buf_mem_[bufIndex], fileName, VX_DF_IMAGE_RGB, 3840, 1920, 11520);
	if (cl_status)
	return cl_status;






	//fp = fopen("result.raw", "wb");
	//	if (fp == NULL)
	//{
	//	return 0;
	//}
	//memcpy(theFrame, fp, 3840*1920*3);
	//	fclose(fp);
	//fp = NULL;

	//	HRESULT   CreateVideoFrame(long width, long height, long rowBytes, BMDPixelFormat pixelFormat, BMDFrameFlags flags, IDeckLinkMutableVideoFrame **outFrame);
	IDeckLinkMutableVideoFrame *outputFrame;
	//r = CreateVideoFrame(3840, 1920, 11520, bmdFormat8BitYUV,0, &outputFrame);


	////////////////////////////////////////

	*/
	// Make sure we are initialised
	if ((m_deckLinkScreenPreviewHelper == NULL) || (m_previewBoxDC == NULL) || (m_openGLctx == NULL))
		return E_FAIL;

	// First, pass the frame to the DeckLink screen preview helper
	m_deckLinkScreenPreviewHelper->SetFrame(theFrame);

	// Then set the OpenGL rendering context to the one we created before
	wglMakeCurrent(m_previewBoxDC->m_hDC, m_openGLctx);

	// and let the helper take care of the drawing
	m_deckLinkScreenPreviewHelper->PaintGL();

	// Last, reset the OpenGL rendering context
	wglMakeCurrent(NULL, NULL);

	return S_OK;
}
