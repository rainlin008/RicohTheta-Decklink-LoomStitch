/* -LICENSE-START-
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

// CapturePreviewDlg.h : header file
//

#pragma once
#include "resource.h"
#include "DeckLinkAPI_h.h"

// Custom messages
#define WM_REFRESH_INPUT_STREAM_DATA_MESSAGE	(WM_APP + 1)
#define WM_SELECT_VIDEO_MODE_MESSAGE			(WM_APP + 2)
#define WM_ADD_DEVICE_MESSAGE					(WM_APP + 3)
#define WM_REMOVE_DEVICE_MESSAGE				(WM_APP + 4)
#define WM_ERROR_RESTARTING_CAPTURE_MESSAGE		(WM_APP + 5)

typedef struct {
	// VITC timecodes and user bits for field 1 & 2
	CString	vitcF1Timecode;
	CString	vitcF1UserBits;
	CString	vitcF2Timecode;
	CString	vitcF2UserBits;

	// RP188 timecodes and user bits (VITC1, VITC2 and LTC)
	CString	rp188vitc1Timecode;
	CString	rp188vitc1UserBits;
	CString	rp188vitc2Timecode;
	CString	rp188vitc2UserBits;
	CString	rp188ltcTimecode;
	CString	rp188ltcUserBits;
} AncillaryDataStruct;


// Forward declarations
class DeckLinkDevice;
class DeckLinkDeviceDiscovery;
class PreviewWindow;
 

class CCapturePreviewDlg : public CDialog
{
public:
	CCapturePreviewDlg(CWnd* pParent = NULL);
	
	// Dialog Data
	enum { IDD = IDD_CAPTUREPREVIEW_DIALOG };

	// UI-related handlers
	afx_msg void			OnNewDeviceSelected();
	afx_msg void			OnStartStopBnClicked();
	afx_msg void			OnClose();

	// Custom message handlers
	afx_msg LRESULT			OnRefreshInputStreamData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT			OnSelectVideoMode(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT			OnAddDevice(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT			OnRemoveDevice(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT			OnErrorRestartingCapture(WPARAM wParam, LPARAM lParam);

	// DeckLinkDevice delegate methods
	void					ShowErrorMessage(TCHAR* msg, TCHAR* title);
	void					UpdateAncillaryData(AncillaryDataStruct& ancillaryData);
	 int device_count;

protected:
	// Internal helper methods
	void					EnableInterface(bool enabled);
	void					RefreshVideoModeList();
	void					StartCapture();
	void					StartCapture(DeckLinkDevice*  m_selectedDevice, PreviewWindow*	  m_previewWindow);
	void					StopCapture();
	void					StopCapture(DeckLinkDevice*  m_selectedDevice );
	void					AddDevice(IDeckLink* deckLink);
	void					RemoveDevice(IDeckLink* deckLink);

	// UI elements
	CComboBox				m_deviceListCombo;
	CButton					m_applyDetectedInputModeCheckbox;
	CComboBox				m_modeListCombo;
	CButton					m_startStopButton;
	CButton					m_startStopButton2;
	CButton					m_startStopButton3;
	CButton					m_startStopButton4;
	CStatic					m_invalidInputLabel;

	CStatic					m_vitcTcF1;
	CStatic					m_vitcUbF1;
	CStatic					m_vitcTcF2;
	CStatic					m_vitcUbF2;

	CStatic					m_rp188Vitc1Tc;
	CStatic					m_rp188Vitc1Ub;
	CStatic					m_rp188Vitc2Tc;
	CStatic					m_rp188Vitc2Ub;
	CStatic					m_rp188LtcTc;
	CStatic					m_rp188LtcUb;

	CStatic					m_previewBox;
	CStatic					m_previewBox2;
	CStatic					m_previewBox3;
	CStatic					m_previewBox4;
	PreviewWindow*			m_previewWindow;
	PreviewWindow*			m_previewWindow2;
	PreviewWindow*			m_previewWindow3;
	PreviewWindow*			m_previewWindow4;

	//
	AncillaryDataStruct			m_ancillaryData;
	CCriticalSection			m_critSec; // to synchronise access to the above structure
	DeckLinkDevice*				m_selectedDevice;
	DeckLinkDevice*				m_selectedDevice2;
	DeckLinkDevice*				m_selectedDevice3;
	DeckLinkDevice*				m_selectedDevice4;

	DeckLinkDeviceDiscovery*	m_deckLinkDiscovery;

	IDeckLink* m_pDeckLink;

	//
	virtual void			DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	HICON					m_hIcon;

	// Generated message map functions
	virtual BOOL			OnInitDialog();
	afx_msg void			OnPaint();
	afx_msg HCURSOR			OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStartStopButton2();
	afx_msg void OnBnClickedStartStopButton3();
	afx_msg void OnBnClickedStartStopButton4();
};
