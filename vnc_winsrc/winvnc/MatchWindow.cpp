//  Copyright (C) 2001 HorizonLive.com, Inc. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/


// MatchWindow.cpp: implementation of the CMatchWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdhdrs.h"
#include "MatchWindow.h"
#include "vncProperties.h"

#define MW_WIDTH 5
#define MW_MARGRIN MW_WIDTH/2+1


TCHAR szMatchWindowClass[]="MATCHWINDOW";

int iCornerNumber;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMatchWindow::CMatchWindow(vncServer* pServer,int left,int top,int right,int bottom)
{
	m_bSized=FALSE;
	m_pServer=pServer;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)CMatchWindow::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= NULL;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szMatchWindowClass;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	m_hWnd=CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,			//dwExStyle
		szMatchWindowClass,		//pointer to registered class name
  		"ScreenShared",					// pointer to window name
  		WS_POPUP  ,				// window style
		12,		               // horizontal position of window
		13,				        // vertical position of window
		300,					 // window width
		200,					// window height
		NULL,				 // handle to parent or owner window
		NULL,						// handle to menu, or child-window identifier
		NULL,				// handle to application instance
		NULL				// pointer to window-creation data
	);
  	
	SetWindowLong(m_hWnd,GWL_USERDATA,(LONG)this);
	ModifyPosition(left,top,right,bottom);

	CanModify(FALSE);
	
}

CMatchWindow::~CMatchWindow()
{
	DestroyWindow(m_hWnd);

}

LRESULT CALLBACK CMatchWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	PAINTSTRUCT ps;
	HDC hdc;
	HBRUSH hBrush;
	CMatchWindow* pMatchWnd=(CMatchWindow*)GetWindowLong(hWnd,GWL_USERDATA);

	switch (message)
	{
	case WM_MOUSEMOVE:
	
		if (!pMatchWnd->m_bSized)
			break;
				
		if (GetCapture()==hWnd)
		{
			// Resize Window
			POINTS ptsMousePoint=MAKEPOINTS(lParam);
			POINT ptMousePoint;
			ptMousePoint.x=ptsMousePoint.x;
			ptMousePoint.y=ptsMousePoint.y;
			ClientToScreen(hWnd,&ptMousePoint);
			RECT wRect;
			GetWindowRect(hWnd,&wRect);
			
			switch (iCornerNumber)
			{
			case 0:	// NW
				wRect.left=ptMousePoint.x;
				wRect.top=ptMousePoint.y;
				break;
			case 1:	// N
				wRect.top=ptMousePoint.y;
				break;
			case 2:	// NE
				wRect.right=ptMousePoint.x;
				wRect.top=ptMousePoint.y;
				break;
			case 3:	// E
				wRect.right=ptMousePoint.x;
				break;
			case 4:	// SE
				wRect.right=ptMousePoint.x;
				wRect.bottom=ptMousePoint.y;
				break;
			case 5:	// S
				wRect.bottom=ptMousePoint.y;
				break;
			case 6:	// SW
				wRect.left=ptMousePoint.x;
				wRect.bottom=ptMousePoint.y;
				break;
			case 7:	// W
				wRect.left=ptMousePoint.x;
				break;
			}
			SetWindowPos(hWnd,NULL,wRect.left,wRect.top,wRect.right-wRect.left,wRect.bottom-wRect.top,0);
            pMatchWnd->ChangeRegion();
			pMatchWnd->ArrangeHits();
			UpdateWindow(hWnd);
		}
		else
		{
			POINT ptHitTest;
			ptHitTest.x=LOWORD(lParam);  
			ptHitTest.y=HIWORD(lParam);
			int i=pMatchWnd->HitTest(ptHitTest);
		
			if (i>-1)
			{
				HCURSOR hCursor;
				div_t div_result;
				div_result = div( i, 4 );
				
				switch (div_result.rem)
				{
				case 0:
					hCursor=LoadCursor(NULL,IDC_SIZENWSE);
					break;
				case 1:
					hCursor=LoadCursor(NULL,IDC_SIZENS);
					break;
				case 2:
					hCursor=LoadCursor(NULL,IDC_SIZENESW);
					break;
				default:
					hCursor=LoadCursor(NULL,IDC_SIZEWE);
				}
				SetCursor(hCursor);
			}
			else 
			{
				HCURSOR hCursor;
				hCursor=LoadCursor(NULL,IDC_ARROW);
				SetCursor(hCursor);
			}
		}
		break;
	
	case WM_LBUTTONDOWN:
	
		if (pMatchWnd->m_bSized) 
		{
			POINT ptHitTest;
			ptHitTest.x=LOWORD(lParam);  
			ptHitTest.y=HIWORD(lParam);
			int i=pMatchWnd->HitTest(ptHitTest);
		
			if (i > -1)
			{
				RECT wRect;
				GetWindowRect(hWnd,&wRect);
				SetCapture(hWnd);
				iCornerNumber=i;

				RECT screenrect = GetScreenRect();

				switch (iCornerNumber)
				{
				case 0:	// NW
					wRect.left = screenrect.left;
					wRect.top = screenrect.top;
					wRect.bottom -= MW_WIDTH*3;
					wRect.right -= MW_WIDTH*3;
					break;
				case 1:	// N
					wRect.left = screenrect.left;
					wRect.top = screenrect.top;
					wRect.bottom -= MW_WIDTH*3;
					wRect.right = screenrect.right;
					break;
				case 2:	// NE
					wRect.left += MW_WIDTH*3;
					wRect.top = screenrect.top;
					wRect.bottom -= MW_WIDTH*3;
					wRect.right = screenrect.right;
					break;
				case 3:	// E
					wRect.left += MW_WIDTH*3;
					wRect.top = screenrect.top;
					wRect.bottom = screenrect.bottom;
					wRect.right = screenrect.right;
					break;
				case 4:	// SE
					wRect.left += MW_WIDTH*3;
					wRect.top += MW_WIDTH*3;
					wRect.bottom = screenrect.bottom;
					wRect.right = screenrect.right;
					break;
				case 5:	// S
					wRect.left = screenrect.left;
					wRect.top += MW_WIDTH*3;
					wRect.bottom = screenrect.bottom;
					wRect.right = screenrect.right;
					break;
				case 6:	// SW
					wRect.left = screenrect.left;
					wRect.top += MW_WIDTH*3;
					wRect.bottom = screenrect.bottom;
					wRect.right -= MW_WIDTH*3;
					break;
				case 7:	// W
					wRect.left = screenrect.left;
					wRect.top = screenrect.top;
					wRect.bottom = screenrect.bottom;
					wRect.right -= MW_WIDTH*3;
					break;
				}
				ClipCursor(&wRect);
			}
			else
			{
				PostMessage(hWnd,WM_NCLBUTTONDOWN, HTCAPTION, lParam);
			} 
		}
		break;
	
	case WM_LBUTTONUP: 
		
		if (GetCapture()==hWnd)
		{
			ReleaseCapture();
			ClipCursor(NULL);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		RECT windowRect;
		GetWindowRect(hWnd,&windowRect);
		hBrush=CreateHatchBrush(HS_FDIAGONAL,RGB(0,0,128));
		OffsetRect(&windowRect,-windowRect.left,-windowRect.top);
		FillRect(hdc,&windowRect,hBrush );
		
		if (pMatchWnd->m_bSized) 
		{
			RECT blackRect;
			HBRUSH hbBlack=CreateSolidBrush(0);
			
			for (int i=0;i<8;i++)
			{
				SetRect(&blackRect, pMatchWnd->m_TrackerHits[i].x,
                    pMatchWnd->m_TrackerHits[i].y,
					pMatchWnd->m_TrackerHits[i].x+MW_WIDTH,
					pMatchWnd->m_TrackerHits[i].y+MW_WIDTH);
				FillRect(hdc, &blackRect,hbBlack);
			}
			DeleteObject(hbBlack);
		}
		DeleteObject(hBrush);
		EndPaint(hWnd, &ps);
		break;

	case WM_CAPTURECHANGED:

		if (pMatchWnd->m_bSized && GetCapture()!=hWnd)
		{
			int left,right,top,bottom;
			pMatchWnd->GetPosition(left,top,right,bottom);

			if (pMatchWnd->m_pServer!=NULL)
				pMatchWnd->m_pServer->SetMatchSizeFields(left,top,right,bottom);
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static long GetRectWidth(const RECT &rect)
{
    return rect.right-rect.left;
}

static long GetRectHeight(const RECT &rect)
{
    return rect.bottom-rect.top;
}

void CMatchWindow::Show()
{
	ShowWindow(m_hWnd,SW_SHOWNOACTIVATE);
}

void CMatchWindow::Hide()
{
	ShowWindow(m_hWnd, SW_HIDE);
}

void CMatchWindow::ArrangeHits()
{
	RECT rect;
    GetWindowRect(m_hWnd,&rect);
    m_TrackerHits[0].x=0;
    m_TrackerHits[0].y=0;

    m_TrackerHits[1].x=(GetRectWidth(rect)-MW_WIDTH)/2;
    m_TrackerHits[1].y=0;

    m_TrackerHits[2].x=GetRectWidth(rect)-MW_WIDTH;
    m_TrackerHits[2].y=0;

    m_TrackerHits[3].x=GetRectWidth(rect)-MW_WIDTH;
    m_TrackerHits[3].y=(GetRectHeight(rect)-MW_WIDTH)/2;

    m_TrackerHits[4].x=GetRectWidth(rect)-MW_WIDTH;
    m_TrackerHits[4].y=GetRectHeight(rect)-MW_WIDTH;

    m_TrackerHits[5].x=(GetRectWidth(rect)-MW_WIDTH)/2;
    m_TrackerHits[5].y=GetRectHeight(rect)-MW_WIDTH;

    m_TrackerHits[6].x=0;
    m_TrackerHits[6].y=GetRectHeight(rect)-MW_WIDTH;

    m_TrackerHits[7].x=0;
    m_TrackerHits[7].y=(GetRectHeight(rect)-MW_WIDTH)/2;

}

void CMatchWindow::CanModify(BOOL bModify)
{	
	m_bSized=bModify;
	HRGN windowRgn=CreateRectRgn(0,0,1,1);
	GetWindowRgn(m_hWnd, windowRgn);
	InvalidateRgn(m_hWnd,windowRgn,false);
	DeleteObject(windowRgn);
}

int CMatchWindow::HitTest(POINT& ptHitTest)
{
    for (int i=0;i<8;i++)
    {   
        RECT Rect;
        SetRect(&Rect, m_TrackerHits[i].x,
			m_TrackerHits[i].y,
			m_TrackerHits[i].x+MW_WIDTH,
			m_TrackerHits[i].y+MW_WIDTH);
        InflateRect(&Rect,MW_MARGRIN,MW_MARGRIN);
        
		if (PtInRect(&Rect,ptHitTest))
			return i;
	}
    return -1;
}

void CMatchWindow::ChangeRegion()
{
	RECT rect;
    HRGN wndHiRgn,wndLoRgn,wndRgn=NULL;
	
    GetWindowRect(m_hWnd,&rect);
    OffsetRect(&rect,-rect.left,-rect.top);

    wndRgn=CreateRectRgn(0, 0, 1,1);
    wndHiRgn=CreateRectRgnIndirect(&rect);
    InflateRect(&rect,-MW_WIDTH,-MW_WIDTH);
    wndLoRgn=CreateRectRgnIndirect(&rect);
    CombineRgn(wndRgn,wndHiRgn,wndLoRgn,RGN_DIFF);
    SetWindowRgn(m_hWnd,wndRgn, TRUE);
    DeleteObject(wndHiRgn);
    DeleteObject(wndLoRgn);
}

BOOL CMatchWindow::ModifyPosition(int left, int top, int right, int bottom)
{
	if (right-left<MW_WIDTH) 
		return FALSE;

	if (bottom-top<MW_WIDTH) 
		return FALSE;
	
	SetWindowPos(m_hWnd,NULL,left-MW_WIDTH,top-MW_WIDTH,right-left+2*MW_WIDTH,bottom-top+2*MW_WIDTH,SWP_NOACTIVATE);
	ChangeRegion();
	ArrangeHits();
	UpdateWindow(m_hWnd);
	return TRUE;
}

void CMatchWindow::GetPosition(int &left,int &top, int &right,int &bottom)
{
	WINDOWPLACEMENT wpl;
	wpl.length=sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_hWnd,&wpl);
	left=wpl.rcNormalPosition.left+MW_WIDTH;
	top=wpl.rcNormalPosition.top+MW_WIDTH;
	right=wpl.rcNormalPosition.right-MW_WIDTH;
	bottom=wpl.rcNormalPosition.bottom-MW_WIDTH;
}
