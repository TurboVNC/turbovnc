//  Copyright (C) 2003 Dennis Syrovatsky. All Rights Reserved.
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
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

#include "vncviewer.h"
#include "VNCviewerApp32.h"
#include "FileTransfer.h"
#include "FileTransferItemInfo.h"

const char FileTransfer::uploadText[] = ">>>";
const char FileTransfer::downloadText[] = "<<<";
const char FileTransfer::noactionText[] = "<--->";

FileTransfer::FileTransfer(ClientConnection * pCC, VNCviewerApp * pApp)
{
	m_clientconn = pCC;
	m_pApp = pApp;
    m_bUploadStarted = FALSE;
    m_bDownloadStarted = FALSE;
	m_bTransferEnable = FALSE;
	m_bReportUploadCancel = FALSE;
	m_bServerBrowseRequest = FALSE;
	m_bFirstFileDownloadMsg = TRUE;
	m_hTreeItem = NULL;
	m_ClientPath[0] = '\0';
	m_ClientPathTmp[0] = '\0';
	m_ServerPath[0] = '\0';
	m_ServerPathTmp[0] = '\0';
	m_numOfFilesToDownload = -1;
	m_currentDownloadIndex = -1;
}

FileTransfer::~FileTransfer()
{
	m_FTClientItemInfo.Free();
	m_FTServerItemInfo.Free();
}

void
FileTransfer::CreateFileTransferDialog()
{
	m_hwndFileTransfer = CreateDialog(m_pApp->m_instance, 
									  MAKEINTRESOURCE(IDD_FILETRANSFER_DLG),
									  NULL, 
									  (DLGPROC) FileTransferDlgProc); 
#ifndef _WIN32_WCE
	VNCviewerApp32 *pApp = (VNCviewerApp32 *)(m_clientconn->m_pApp);
	pApp->AddModelessDialog(m_hwndFileTransfer);
#endif
	ShowWindow(m_hwndFileTransfer, SW_SHOW);
	UpdateWindow(m_hwndFileTransfer);
	SetWindowLong(m_hwndFileTransfer, GWL_USERDATA, (LONG) this);

	m_hwndFTProgress = GetDlgItem(m_hwndFileTransfer, IDC_FTPROGRESS);
	m_hwndFTClientList = GetDlgItem(m_hwndFileTransfer, IDC_FTCLIENTLIST);
	m_hwndFTServerList = GetDlgItem(m_hwndFileTransfer, IDC_FTSERVERLIST);
	m_hwndFTClientPath = GetDlgItem(m_hwndFileTransfer, IDC_CLIENTPATH);
	m_hwndFTServerPath = GetDlgItem(m_hwndFileTransfer, IDC_SERVERPATH);
	m_hwndFTStatus = GetDlgItem(m_hwndFileTransfer, IDC_FTSTATUS);

	ListView_SetExtendedListViewStyleEx(m_hwndFTClientList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyleEx(m_hwndFTServerList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	HANDLE hIcon = LoadImage(m_pApp->m_instance, MAKEINTRESOURCE(IDI_FILEUP), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTUP), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	SendMessage(GetDlgItem(m_hwndFileTransfer, IDC_SERVERUP), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	DestroyIcon((HICON) hIcon);
	hIcon = LoadImage(m_pApp->m_instance, MAKEINTRESOURCE(IDI_FILERELOAD), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTRELOAD), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	SendMessage(GetDlgItem(m_hwndFileTransfer, IDC_SERVERRELOAD), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	DestroyIcon((HICON) hIcon);

	RECT Rect;
	GetClientRect(m_hwndFTClientList, &Rect);
	int xwidth = (int) (0.7 * Rect.right);
	int xwidth_ = (int) (0.25 * Rect.right);

	FTInsertColumn(m_hwndFTClientList, "Name", 0, xwidth);
	FTInsertColumn(m_hwndFTClientList, "Size", 1, xwidth_);
	FTInsertColumn(m_hwndFTServerList, "Name", 0, xwidth);
	FTInsertColumn(m_hwndFTServerList, "Size", 1, xwidth_);

	ShowClientItems(m_ClientPathTmp);
	SendFileListRequestMessage(m_ServerPathTmp, 0);
}

LRESULT CALLBACK 
FileTransfer::FileTransferDlgProc(HWND hwnd, 
								  UINT uMsg, 
								  WPARAM wParam, 
								  LPARAM lParam)
{
	FileTransfer *_this = (FileTransfer *) GetWindowLong(hwnd, GWL_USERDATA);
	int i;
	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			SetForegroundWindow(hwnd);
			CentreWindow(hwnd);
			return TRUE;
		}
	break;
	case WM_HELP:	
		help.Popup(lParam);
		return 0;
	case WM_COMMAND:
		{
		switch (LOWORD(wParam))
		{
			case IDC_CLIENTPATH:
				switch (HIWORD (wParam))
				{
					case EN_SETFOCUS:
						SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
						EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
						return TRUE;
				}
			break;
			case IDC_SERVERPATH:
				switch (HIWORD (wParam))
				{
					case EN_SETFOCUS:
						SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
						EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
						return TRUE;
				}
			break;
			case IDC_EXIT:
			case IDCANCEL:
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				return TRUE;
			case IDC_CLIENTUP:
				SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
				EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
				SendMessage(_this->m_hwndFTProgress, PBM_SETPOS, 0, 0);
				SetWindowText(_this->m_hwndFTStatus, "");
				if (strcmp(_this->m_ClientPathTmp, "") == 0) return TRUE;
				for (i=(strlen(_this->m_ClientPathTmp)-2); i>=0; i--) {
					if (_this->m_ClientPathTmp[i] == '\\') {
						_this->m_ClientPathTmp[i] = '\0';
						break;
					}
					if (i == 0) _this->m_ClientPathTmp[0] = '\0';
				}
				_this->ShowClientItems(_this->m_ClientPathTmp);
				return TRUE;
			case IDC_SERVERUP:
				SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
				EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
				SendMessage(_this->m_hwndFTProgress, PBM_SETPOS, 0, 0);
				SetWindowText(_this->m_hwndFTStatus, "");
				if (strcmp(_this->m_ServerPathTmp, "") == 0) return TRUE;
				for (i=(strlen(_this->m_ServerPathTmp)-2); i>=0; i--) {
					if (_this->m_ServerPathTmp[i] == '\\') {
						_this->m_ServerPathTmp[i] = '\0';
						break;
					}
					if (i == 0) _this->m_ServerPathTmp[0] = '\0';
				}
				_this->SendFileListRequestMessage(_this->m_ServerPathTmp, 0);
				return TRUE;
			case IDC_CLIENTRELOAD:
				SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
				EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
				SendMessage(_this->m_hwndFTProgress, PBM_SETPOS, 0, 0);
				SetWindowText(_this->m_hwndFTStatus, "");
				_this->ShowClientItems(_this->m_ClientPath);
				return TRUE;
			case IDC_SERVERRELOAD:
				SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
				EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
				SendMessage(_this->m_hwndFTProgress, PBM_SETPOS, 0, 0);
				SetWindowText(_this->m_hwndFTStatus, "");
				_this->SendFileListRequestMessage(_this->m_ServerPathTmp, 0);
				return TRUE;
			case IDC_FTCOPY:
				// First, check if the action is supported by the server.
				if (_this->m_bFTCOPY == FALSE) {
					// Upload was requested.
					if ( !_this->m_clientconn->m_clientMsgCaps.IsEnabled(rfbFileUploadRequest) ||
						 !_this->m_clientconn->m_clientMsgCaps.IsEnabled(rfbFileUploadData) ) {
						MessageBox(hwnd, "Sorry but the server does not support uploading files.",
								   "Error", MB_OK | MB_ICONEXCLAMATION);
						char buf[MAX_PATH];
						sprintf(buf, "File upload not supported by server");
						SetWindowText(_this->m_hwndFTStatus, buf);
						return TRUE;
					}
				} else {
					// Download was requested.
					if ( !_this->m_clientconn->m_clientMsgCaps.IsEnabled(rfbFileDownloadRequest) ||
						 !_this->m_clientconn->m_serverMsgCaps.IsEnabled(rfbFileDownloadData) ) {
						MessageBox(hwnd, "Sorry but the server does not support downloading files.",
								   "Error", MB_OK | MB_ICONEXCLAMATION);
						char buf[MAX_PATH];
						sprintf(buf, "File download not supported by server");
						SetWindowText(_this->m_hwndFTStatus, buf);
						return TRUE;
					}
				}
				// Now, try to upload/download.
				SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
				EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
				if (_this->m_ClientPath[0] == '\0' || _this->m_ServerPath[0] == '\0') {
					SetWindowText(_this->m_hwndFTStatus, "Cannot transfer files: illegal directory.");
					return TRUE;
				}
				if (_this->m_bFTCOPY == FALSE) {
					_this->m_bTransferEnable = TRUE;
					_this->m_bReportUploadCancel = TRUE;
					EnableWindow(GetDlgItem(hwnd, IDC_FTCANCEL), TRUE);
					_this->FileTransferUpload();			
				} else {
					return _this->SendMultipleFileDownloadRequests();
				}
				return TRUE;
			case IDC_FTCANCEL:
				// Check if we allowed to interrupt the transfer.
				if ( _this->m_bUploadStarted &&
					 !_this->m_clientconn->m_clientMsgCaps.IsEnabled(rfbFileUploadFailed) ) {
					char buf[MAX_PATH];
					sprintf(buf, "Sorry, but interrupting upload is not supported by the server");
					SetWindowText(_this->m_hwndFTStatus, buf);
					return TRUE;
				}
				if ( _this->m_bDownloadStarted &&
					 !_this->m_clientconn->m_clientMsgCaps.IsEnabled(rfbFileDownloadCancel) ) {
					char buf[MAX_PATH];
					sprintf(buf, "Sorry, but interrupting download is not supported by the server");
					SetWindowText(_this->m_hwndFTStatus, buf);
					return TRUE;
				}
				// Now try to cancel the operation.
				SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
				EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
				_this->m_bTransferEnable = FALSE;
				EnableWindow(GetDlgItem(hwnd, IDC_FTCANCEL), FALSE);
				return TRUE;
			case IDC_CLIENTBROWSE_BUT:
				SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
				EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
				SendMessage(_this->m_hwndFTProgress, PBM_SETPOS, 0, 0);
				SetWindowText(_this->m_hwndFTStatus, "");
				_this->CreateFTBrowseDialog(FALSE);
				return TRUE;
			case IDC_SERVERBROWSE_BUT:
				SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), noactionText);
				EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), FALSE);
				SendMessage(_this->m_hwndFTProgress, PBM_SETPOS, 0, 0);
				SetWindowText(_this->m_hwndFTStatus, "");
				_this->CreateFTBrowseDialog(TRUE);
				return TRUE;
		}
		}
	break;

	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case IDC_FTCLIENTLIST:
			switch (((LPNMHDR) lParam)->code)
			{
				case NM_SETFOCUS:
					SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), uploadText);
					EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), TRUE);
					_this->m_bFTCOPY = FALSE;
					return TRUE;
				case LVN_GETDISPINFO:
					_this->OnGetDispClientInfo((NMLVDISPINFO *) lParam); 
					return TRUE;
				case LVN_ITEMACTIVATE:
					LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
					_this->ProcessListViewDBLCLK(_this->m_hwndFTClientList, _this->m_ClientPath, _this->m_ClientPathTmp, lpnmia->iItem);
					return TRUE;
			}
		break;
		case IDC_FTSERVERLIST:
			switch (((LPNMHDR) lParam)->code)
			{
				case NM_SETFOCUS:
					SetWindowText(GetDlgItem(hwnd, IDC_FTCOPY), downloadText);
					EnableWindow(GetDlgItem(hwnd, IDC_FTCOPY), TRUE);
					_this->m_bFTCOPY = TRUE;
					return TRUE;
				case LVN_GETDISPINFO: 
					_this->OnGetDispServerInfo((NMLVDISPINFO *) lParam); 
					return TRUE;
				case LVN_ITEMACTIVATE:
					LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
					_this->ProcessListViewDBLCLK(_this->m_hwndFTServerList, _this->m_ServerPath, _this->m_ServerPathTmp, lpnmia->iItem);
					return TRUE;
			}
		break;
		}
		break;
	case WM_CLOSE:
		_this->m_clientconn->m_fileTransferDialogShown = false;
		_this->m_FTClientItemInfo.Free();
		_this->m_FTServerItemInfo.Free();
#ifndef _WIN32_WCE
		{
			VNCviewerApp32 *pApp = (VNCviewerApp32 *)(_this->m_clientconn->m_pApp);
			pApp->RemoveModelessDialog(hwnd);
		}
#endif
		DestroyWindow(hwnd);
		return TRUE;
	}
	return 0;
}

// This method will be called, each time window message 'IDC_FTCOPY' is sent for 
// file download. In following cases IDC_FTCOPY will be sent for file download.
// 1. From UI, when download button is clicked
// 2. From 'FileTransferDownload' method after each file download is complete.
// This method will keep track of number of selected files for which download 
// request is not sent. With each call it will send download request for one file
// and will return. After the completion of file download 'FileTransferDownload' 
// method will do a PostMessage for 'IDC_FTCOPY', which will invoke this method 
// again and it will send another request for file download if download request for
// some of the selected files is still pending.
BOOL
FileTransfer:: SendMultipleFileDownloadRequests()
{
	// Download Request for all the selected files is sent.
	if(m_numOfFilesToDownload == 0) {

		m_numOfFilesToDownload = -1 ;
		m_currentDownloadIndex = -1;

		EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
		BlockingFileTransferDialog(TRUE);

		return TRUE;
	}

	// This is the first call for currently select file list.
	// Count of slected files is not calculated yet 
	if(m_numOfFilesToDownload == -1) {

		m_numOfFilesToDownload = ListView_GetSelectedCount(m_hwndFTServerList);
		if (m_numOfFilesToDownload <= 0) {

			SetWindowText(m_hwndFTStatus, "No file is selected, nothing to download.");

			m_numOfFilesToDownload  = -1;
			m_currentDownloadIndex = -1;

			EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
			BlockingFileTransferDialog(TRUE);

			return TRUE;
		}
		else { 
			// file transfer will start for all the selected files now. set m_bTransferEnable to true
			// Enable cancel button and disable rest of the UI components.
			m_bTransferEnable = TRUE;
			BlockingFileTransferDialog(FALSE);
			EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), TRUE);
		}
	}

	// Calculate the next selected index for which file download request has to be sent.
	int index = -1;
	index = ListView_GetNextItem(m_hwndFTServerList, m_currentDownloadIndex, LVNI_SELECTED);
	if (index < 0) {
		SetWindowText(m_hwndFTStatus, "No file is selected, nothing to download.");

		m_numOfFilesToDownload  = -1;
		m_currentDownloadIndex = -1;

		EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
		BlockingFileTransferDialog(TRUE);

		return TRUE;
	}

	// If Cancel button is clicked, dont send the file download request.
	if(m_bTransferEnable == FALSE) {
		m_numOfFilesToDownload  = -1;
		m_currentDownloadIndex = -1;

		EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
		BlockingFileTransferDialog(TRUE);

		return TRUE;
	}

	// Update member variables for next call.
	m_currentDownloadIndex = index;
	m_numOfFilesToDownload -= 1;

	return SendFileDownloadRequest();
}

BOOL
FileTransfer:: SendFileDownloadRequest()
{
	char path[rfbMAX_PATH + rfbMAX_PATH];
	if (!m_FTServerItemInfo.IsFile(m_currentDownloadIndex)) {
		SetWindowText(m_hwndFTStatus, "Cannot download: not a regular file.");
		// Send message to start download for next selected file.
		PostMessage(m_hwndFileTransfer, WM_COMMAND, IDC_FTCOPY, 0);
		return TRUE;
	}
	ListView_GetItemText(m_hwndFTServerList, m_currentDownloadIndex, 0, m_ServerFilename, rfbMAX_PATH);
	strcpy(m_ClientFilename, m_ServerFilename);
	char buffer[rfbMAX_PATH + rfbMAX_PATH + rfbMAX_PATH];
	sprintf(buffer, "Downloading: %s\\%s -> %s\\%s ...",
			m_ServerPath, m_ServerFilename,
			m_ClientPath, m_ClientFilename);
	SetWindowText(m_hwndFTStatus, buffer);
	m_sizeDownloadFile = m_FTServerItemInfo.GetIntSizeAt(m_currentDownloadIndex);
	rfbFileDownloadRequestMsg fdr;
	fdr.type = rfbFileDownloadRequest;
	fdr.compressedLevel = 0;
	fdr.position = Swap32IfLE(0);
	sprintf(path, "%s\\%s", m_ServerPath, m_ServerFilename);
	ConvertPath(path);
	int len = strlen(path);
	fdr.fNameSize = Swap16IfLE(len);
	m_clientconn->WriteExact((char *)&fdr, sz_rfbFileDownloadRequestMsg);
	m_clientconn->WriteExact(path, len);
	return TRUE;
}

void 
FileTransfer::OnGetDispClientInfo(NMLVDISPINFO *plvdi) 
{
  switch (plvdi->item.iSubItem)
    {
    case 0:
		plvdi->item.pszText = m_FTClientItemInfo.GetNameAt(plvdi->item.iItem);
      break;
    case 1:
		plvdi->item.pszText = m_FTClientItemInfo.GetSizeAt(plvdi->item.iItem);
      break;
    default:
      break;
    }
 } 

void 
FileTransfer::OnGetDispServerInfo(NMLVDISPINFO *plvdi) 
{
  switch (plvdi->item.iSubItem)
  {
    case 0:
		plvdi->item.pszText = m_FTServerItemInfo.GetNameAt(plvdi->item.iItem);
      break;
    case 1:
		plvdi->item.pszText = m_FTServerItemInfo.GetSizeAt(plvdi->item.iItem);
      break;
    default:
      break;
    }
 } 

void 
FileTransfer::FileTransferUpload()
{
	int numOfFilesToUpload = 0, currentUploadIndex = -1;
	DWORD sz_rfbFileSize;
	DWORD sz_rfbBlockSize= 8192;
	DWORD dwNumberOfBytesRead = 0;
	unsigned int mTime = 0;
	char path[rfbMAX_PATH + rfbMAX_PATH + 2];
	BOOL bResult;
	numOfFilesToUpload = ListView_GetSelectedCount(m_hwndFTClientList);
	if (numOfFilesToUpload < 0) {
		SetWindowText(m_hwndFTStatus, "No file selected, nothing to upload.");
		BlockingFileTransferDialog(TRUE);
		EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
		return;
	}

	for (int i = 0; i < numOfFilesToUpload; i++) {
		BlockingFileTransferDialog(FALSE);
		EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), TRUE);
		int index = ListView_GetNextItem(m_hwndFTClientList, currentUploadIndex, LVNI_SELECTED);
		if (index < 0) {
			SetWindowText(m_hwndFTStatus, "No file is selected, nothing to download.");
			return;
		}
		currentUploadIndex = index;
		ListView_GetItemText(m_hwndFTClientList, currentUploadIndex, 0, m_ClientFilename, rfbMAX_PATH);
		sprintf(path, "%s\\%s", m_ClientPath, m_ClientFilename);
		strcpy(m_UploadFilename, path);
		WIN32_FIND_DATA FindFileData;
		SetErrorMode(SEM_FAILCRITICALERRORS);
		HANDLE hFile = FindFirstFile(path, &FindFileData);
		SetErrorMode(0);
		if (hFile == INVALID_HANDLE_VALUE) {
			SetWindowText(m_hwndFTStatus, "Could not find selected file, can't upload");
			// Continue with upload of other files.
			continue;
		} else if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			SetWindowText(m_hwndFTStatus, "Cannot upload a directory");
			// Continue with upload of other files.
			continue;
		} else {
			sz_rfbFileSize = FindFileData.nFileSizeLow;
			mTime = FiletimeToTime70(FindFileData.ftLastWriteTime);
			strcpy(m_ServerFilename, FindFileData.cFileName);
		}
		FindClose(hFile);
		if ((sz_rfbFileSize != 0) && (sz_rfbFileSize <= sz_rfbBlockSize)) sz_rfbBlockSize = sz_rfbFileSize;
		m_hFiletoRead = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (m_hFiletoRead == INVALID_HANDLE_VALUE) {
			SetWindowText(m_hwndFTStatus, "Upload failed: could not open selected file");
			BlockingFileTransferDialog(TRUE);
			EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
			return;
		}
		char buffer[rfbMAX_PATH + rfbMAX_PATH + rfbMAX_PATH + rfbMAX_PATH + 20];
		sprintf(buffer, "Uploading: %s\\%s -> %s\\%s ...",
				m_ClientPath, m_ClientFilename, m_ServerPath, m_ServerFilename);
		SetWindowText(m_hwndFTStatus, buffer);
		sprintf(path, "%s\\%s", m_ServerPath, m_ClientFilename);
		ConvertPath(path);
		int pathLen = strlen(path);

		char *pAllFURMessage = new char[sz_rfbFileUploadRequestMsg + pathLen];
		rfbFileUploadRequestMsg *pFUR = (rfbFileUploadRequestMsg *) pAllFURMessage;
		char *pFollowMsg = &pAllFURMessage[sz_rfbFileUploadRequestMsg];
		pFUR->type = rfbFileUploadRequest;
		pFUR->compressedLevel = 0;
		pFUR->fNameSize = Swap16IfLE(pathLen);
		pFUR->position = Swap32IfLE(0);
		memcpy(pFollowMsg, path, pathLen);
		m_clientconn->WriteExact(pAllFURMessage, sz_rfbFileUploadRequestMsg + pathLen); 
		delete [] pAllFURMessage;

		if (sz_rfbFileSize == 0) {
			SendFileUploadDataMessage(mTime);
		} else {
			int amount = sz_rfbFileSize / (sz_rfbBlockSize * 10);

			InitProgressBar(0, 0, amount, 1);

			DWORD dwPortionRead = 0;
			char *pBuff = new char [sz_rfbBlockSize];
			m_bUploadStarted = TRUE;
			while(m_bUploadStarted) {
				ProcessDlgMessage(m_hwndFileTransfer);
				if (m_bTransferEnable == FALSE) {
					SetWindowText(m_hwndFTStatus, "File transfer canceled");
					EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
					BlockingFileTransferDialog(TRUE);
					char reason[] = "File transfer canceled by user";
					int reasonLen = strlen(reason);
					char *pFUFMessage = new char[sz_rfbFileUploadFailedMsg + reasonLen];
					rfbFileUploadFailedMsg *pFUF = (rfbFileUploadFailedMsg *) pFUFMessage;
					char *pReason = &pFUFMessage[sz_rfbFileUploadFailedMsg];
					pFUF->type = rfbFileUploadFailed;
					pFUF->reasonLen = Swap16IfLE(reasonLen);
					memcpy(pReason, reason, reasonLen);
					m_clientconn->WriteExact(pFUFMessage, sz_rfbFileUploadFailedMsg + reasonLen);
					delete [] pFUFMessage;
					break;
				}
				bResult = ReadFile(m_hFiletoRead, pBuff, sz_rfbBlockSize, &dwNumberOfBytesRead, NULL);
				if (bResult && dwNumberOfBytesRead == 0) {
					/* This is the end of the file. */
					SendFileUploadDataMessage(mTime);
					break;
				}
				SendFileUploadDataMessage((unsigned short)dwNumberOfBytesRead, pBuff);
				dwPortionRead += dwNumberOfBytesRead;
				if (dwPortionRead >= (10 * sz_rfbBlockSize)) {
					dwPortionRead = 0;
					SendMessage(m_hwndFTProgress, PBM_STEPIT, 0, 0);
				}
			}
			if (m_bTransferEnable == FALSE)
				break;
			m_bUploadStarted = FALSE;
			delete [] pBuff;
		}
		SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
		SetWindowText(m_hwndFTStatus, "");
		CloseHandle(m_hFiletoRead);
	}
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
	BlockingFileTransferDialog(TRUE);
	SendFileListRequestMessage(m_ServerPath, 0);
}

void
FileTransfer::CloseUndoneFileTransfers()
{
  if (m_bUploadStarted) {
    m_bUploadStarted = FALSE;
    CloseHandle(m_hFiletoRead);
  }
  if (m_bDownloadStarted) {
    m_bDownloadStarted = FALSE;
	m_bFirstFileDownloadMsg = TRUE;
	m_currentDownloadIndex = -1;
	m_numOfFilesToDownload = -1;
    CloseHandle(m_hFiletoWrite);
    DeleteFile(m_DownloadFilename);
  }
}

void 
FileTransfer::FileTransferDownload()
{
	rfbFileDownloadDataMsg fdd;
	m_clientconn->ReadExact((char *)&fdd, sz_rfbFileDownloadDataMsg);
	fdd.realSize = Swap16IfLE(fdd.realSize);
	fdd.compressedSize = Swap16IfLE(fdd.compressedSize);

	char path[rfbMAX_PATH + rfbMAX_PATH + 3];
	
	if (m_bFirstFileDownloadMsg) {
		m_dwDownloadBlockSize = fdd.compressedSize;
		sprintf(path, "%s\\%s", m_ClientPath, m_ServerFilename);
		strcpy(m_DownloadFilename, path);
		m_hFiletoWrite = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
									NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		int amount = m_sizeDownloadFile / ((m_dwDownloadBlockSize + 1) * 10);
		InitProgressBar(0, 0, amount, 1);
		m_bFirstFileDownloadMsg = FALSE;
		m_bDownloadStarted = TRUE;
	}
	if ((fdd.realSize == 0) && (fdd.compressedSize == 0)) {
		unsigned int mTime;
		m_clientconn->ReadExact((char *) &mTime, sizeof(unsigned int));
		if (m_hFiletoWrite == INVALID_HANDLE_VALUE) {
			CancelDownload("Could not create file");
			MessageBox(m_hwndFileTransfer, "Download failed: could not create local file",
					   "Download Failed", MB_ICONEXCLAMATION | MB_OK);
			SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
			// Send message to start download for next selected file.
			PostMessage(m_hwndFileTransfer, WM_COMMAND, IDC_FTCOPY, 0);
			return;
		}
		FILETIME Filetime;
		Time70ToFiletime(mTime, &Filetime);
		SetFileTime(m_hFiletoWrite, &Filetime, &Filetime, &Filetime);
		SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
		SetWindowText(m_hwndFTStatus, "");
		CloseHandle(m_hFiletoWrite);
		ShowClientItems(m_ClientPath);
		m_bFirstFileDownloadMsg = TRUE;
		m_bDownloadStarted = FALSE;
		// Send message to start download for next selected file.
		PostMessage(m_hwndFileTransfer, WM_COMMAND, IDC_FTCOPY, 0);
		return;
	}
	char * pBuff = new char [fdd.compressedSize];
	DWORD dwNumberOfBytesWritten;
	m_clientconn->ReadExact(pBuff, fdd.compressedSize);
	ProcessDlgMessage(m_hwndFileTransfer);
	if (!m_bTransferEnable) {
		CancelDownload("Download cancelled by user");
		delete [] pBuff;
		return;
	}
	if (m_hFiletoWrite == INVALID_HANDLE_VALUE) {
		CancelDownload("Could not create file");
		MessageBox(m_hwndFileTransfer, "Download failed: could not create local file",
				   "Download Failed", MB_ICONEXCLAMATION | MB_OK);
		delete [] pBuff;
		return;
	}
	WriteFile(m_hFiletoWrite, pBuff, fdd.compressedSize, &dwNumberOfBytesWritten, NULL);
	m_dwDownloadRead += dwNumberOfBytesWritten;
	if (m_dwDownloadRead >= (10 * m_dwDownloadBlockSize)) {
		m_dwDownloadRead = 0;
		SendMessage(m_hwndFTProgress, PBM_STEPIT, 0, 0); 
	}
	delete [] pBuff;
}

void
FileTransfer::CancelDownload(char *reason)
{
	SendFileDownloadCancelMessage(strlen(reason), reason);
	SetWindowText(m_hwndFTStatus, reason);
	CloseUndoneFileTransfers();
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
	BlockingFileTransferDialog(TRUE);
	m_bDownloadStarted = FALSE;
}

void 
FileTransfer::ShowClientItems(char *path)
{
	if (strlen(path) == 0) {
		//Show Logical Drives
		ListView_DeleteAllItems(m_hwndFTClientList);
		m_FTClientItemInfo.Free();
		int LengthDrivesString = 0;
		char DrivesString[256];
		char DriveName[rfbMAX_PATH] = "?:";
		LengthDrivesString = GetLogicalDriveStrings(256, DrivesString);
		if ((LengthDrivesString == 0) || (LengthDrivesString > 256)) {
			BlockingFileTransferDialog(TRUE);
			strcpy(m_ClientPathTmp, m_ClientPath);
			return;
		} else {
			strcpy(m_ClientPath, m_ClientPathTmp);
			SetWindowText(m_hwndFTClientPath, m_ClientPath);
			for (int i=0; i<256; i++) {
				DriveName[0] = DrivesString[i];
				char txt[16];
				strcpy(txt, m_FTClientItemInfo.folderText);
				m_FTClientItemInfo.Add(DriveName, txt, 0);
				DriveName[0] = '\0';
				i+=3;
				if ((DrivesString[i] == '\0') && (DrivesString[i+1] == '\0')) break;
			}
			m_FTClientItemInfo.Sort();
			ShowListViewItems(m_hwndFTClientList, &m_FTClientItemInfo);
		}
	} else {
		//Show Files
		HANDLE m_handle;
		int n = 0;
		WIN32_FIND_DATA m_FindFileData;
		strcat(path, "\\*");
		SetErrorMode(SEM_FAILCRITICALERRORS);
		m_handle = FindFirstFile(path, &m_FindFileData);
		DWORD LastError = GetLastError();
		SetErrorMode(0);
		if (m_handle == INVALID_HANDLE_VALUE) {
			if (LastError != ERROR_SUCCESS && LastError != ERROR_FILE_NOT_FOUND) {
				strcpy(m_ClientPathTmp, m_ClientPath);
				FindClose(m_handle);
				BlockingFileTransferDialog(TRUE);
				return;
			}
			path[strlen(path) - 2] = '\0';
			strcpy(m_ClientPath, m_ClientPathTmp);
			SetWindowText(m_hwndFTClientPath, m_ClientPath);
			FindClose(m_handle);
			ListView_DeleteAllItems(m_hwndFTClientList);
			BlockingFileTransferDialog(TRUE);
			return;
		}
		ListView_DeleteAllItems(m_hwndFTClientList);
		m_FTClientItemInfo.Free();
		char buffer[16];
		while(1) {
			if ((strcmp(m_FindFileData.cFileName, ".") != 0) &&
		       (strcmp(m_FindFileData.cFileName, "..") != 0)) {
				if (!(m_FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					sprintf(buffer, "%d", m_FindFileData.nFileSizeLow);
					LARGE_INTEGER li;
					li.LowPart = m_FindFileData.ftLastWriteTime.dwLowDateTime;
					li.HighPart = m_FindFileData.ftLastWriteTime.dwHighDateTime;
					li.QuadPart = (li.QuadPart - 116444736000000000) / 10000000;
					m_FTClientItemInfo.Add(m_FindFileData.cFileName, buffer, li.LowPart);
				} else {
					strcpy(buffer, m_FTClientItemInfo.folderText);
					m_FTClientItemInfo.Add(m_FindFileData.cFileName, buffer, 0);
				}
			}
			if (!FindNextFile(m_handle, &m_FindFileData)) break;
		}
		FindClose(m_handle);
		m_FTClientItemInfo.Sort();
		ShowListViewItems(m_hwndFTClientList, &m_FTClientItemInfo);
		path[strlen(path) - 2] = '\0';
		strcpy(m_ClientPath, m_ClientPathTmp);
		SetWindowText(m_hwndFTClientPath, m_ClientPath);
	}
	BlockingFileTransferDialog(TRUE);
}

BOOL CALLBACK 
FileTransfer::FTBrowseDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FileTransfer *_this = (FileTransfer *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			_this = (FileTransfer *) lParam;
			CentreWindow(hwnd);
			_this->m_hwndFTBrowse = hwnd;
			if (_this->m_bServerBrowseRequest) {
				_this->SendFileListRequestMessage("", 0x10);
				return TRUE;
			} else {
				TVITEM TVItem;
				TVINSERTSTRUCT tvins; 
				char DrivesString[256];
				char drive[] = "?:";
				int LengthDriveString = GetLogicalDriveStrings(256, DrivesString);
				TVItem.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_HANDLE;
				for (int i=0; i<LengthDriveString; i++) {
					drive[0] = DrivesString[i];
					TVItem.pszText = drive;
					TVItem.cChildren = 1;
					tvins.item = TVItem;
					tvins.hParent = TreeView_InsertItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), &tvins);
					tvins.item = TVItem;
					TreeView_InsertItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), &tvins);
					tvins.hParent = NULL;
					i += 3;
				}
			}
			return TRUE;
		}
	break;
	case WM_COMMAND:
		{
		switch (LOWORD(wParam))
		{
			case IDC_FTBROWSECANCEL:
				EndDialog(hwnd, TRUE);
				_this->m_bServerBrowseRequest = FALSE;
				return TRUE;
			case IDC_FTBROWSEOK:
				char path[rfbMAX_PATH];
				if (GetWindowText(GetDlgItem(hwnd, IDC_FTBROWSEEDIT), path, rfbMAX_PATH) == 0) {
					EndDialog(hwnd, TRUE);
					_this->m_bServerBrowseRequest = FALSE;
					return TRUE;
				}
				if (_this->m_bServerBrowseRequest) {
					strcpy(_this->m_ServerPathTmp, path);
					EndDialog(hwnd,TRUE);
					_this->m_bServerBrowseRequest = FALSE;
					_this->SendFileListRequestMessage(_this->m_ServerPathTmp, 0);
					return TRUE;
				} else {
					strcpy(_this->m_ClientPathTmp, path);
					EndDialog(hwnd,TRUE);
					_this->ShowClientItems(_this->m_ClientPathTmp);
				}
				return TRUE;
		}
		}
	break;
	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case IDC_FTBROWSETREE:
			switch (((LPNMHDR) lParam)->code)
			{
			case TVN_SELCHANGED:
				{
					NMTREEVIEW *m_lParam = (NMTREEVIEW *) lParam;
					char path[rfbMAX_PATH];
					_this->GetTVPath(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem, path);
					SetWindowText(GetDlgItem(hwnd, IDC_FTBROWSEEDIT), path);
					return TRUE;
				}
				break;
			case TVN_ITEMEXPANDING:
				{
				NMTREEVIEW *m_lParam = (NMTREEVIEW *) lParam;
				char Path[rfbMAX_PATH];
				if (m_lParam -> action == 2) {
					if (_this->m_bServerBrowseRequest) {
						_this->m_hTreeItem = m_lParam->itemNew.hItem;
						_this->GetTVPath(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem, Path);
						_this->SendFileListRequestMessage(Path, 0x10);
						return TRUE;
					} else {
						_this->ShowTreeViewItems(hwnd, m_lParam);
					}
				}
				return TRUE;
				}
			}
			break;
		}
	break;
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, FALSE);
		_this->m_bServerBrowseRequest = FALSE;
		return TRUE;
	}
	return 0;
}

void 
FileTransfer::CreateFTBrowseDialog(BOOL status)
{
	m_bServerBrowseRequest = status;
	DialogBoxParam(m_pApp->m_instance, MAKEINTRESOURCE(IDD_FTBROWSE_DLG), m_hwndFileTransfer, (DLGPROC) FTBrowseDlgProc, (LONG) this);
}

void 
FileTransfer::GetTVPath(HWND hwnd, HTREEITEM hTItem, char *path)
{
	char szText[rfbMAX_PATH];
	TVITEM _tvi;
	path[0] = '\0';
	do {
		_tvi.mask = TVIF_TEXT | TVIF_HANDLE;
		_tvi.hItem = hTItem;
		_tvi.pszText = szText;
		_tvi.cchTextMax = rfbMAX_PATH;
		TreeView_GetItem(hwnd, &_tvi);
		strcat(path, "\\");
		strcat(path, _tvi.pszText);
		hTItem = TreeView_GetParent(hwnd, hTItem);
	}
	while(hTItem != NULL);
	char path_tmp[rfbMAX_PATH], path_out[rfbMAX_PATH];
	path_tmp[0] = '\0';
	path_out[0] = '\0';
	int len = strlen(path);
	int ii = 0;
	for (int i = (len-1); i>=0; i--) {
		if (path[i] == '\\') {
			StrInvert(path_tmp);
			strcat(path_out, path_tmp);
			strcat(path_out, "\\");
			path_tmp[0] = '\0';
			ii = 0;
		} else {
			path_tmp[ii] = path[i];
			path_tmp[ii+1] = '\0';
			ii++;
		}
	}
	if (path_out[strlen(path_out)-1] == '\\') path_out[strlen(path_out)-1] = '\0';
	strcpy(path, path_out);
}

void
FileTransfer::StrInvert(char str[rfbMAX_PATH])
{
	int len = strlen(str), i;
	char str_out[rfbMAX_PATH];
	str_out[0] = '\0';
	for (i = (len-1); i>=0; i--) str_out[len-i-1] = str[i];
	str_out[len] = '\0';
	strcpy(str, str_out);
}

void 
FileTransfer::ShowTreeViewItems(HWND hwnd, LPNMTREEVIEW m_lParam)
{
	HANDLE m_handle;
	WIN32_FIND_DATA m_FindFileData;
	TVITEM tvi;
	TVINSERTSTRUCT tvins;
	char path[rfbMAX_PATH];
	GetTVPath(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem, path);
	strcat(path, "\\*");
	while (TreeView_GetChild(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem) != NULL) {
		TreeView_DeleteItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), TreeView_GetChild(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem));
	}
	SetErrorMode(SEM_FAILCRITICALERRORS);
	m_handle = FindFirstFile(path, &m_FindFileData);
	SetErrorMode(0);
	if (m_handle == INVALID_HANDLE_VALUE) return;
	while(1) {
		if ((strcmp(m_FindFileData.cFileName, ".") != 0) && 
			(strcmp(m_FindFileData.cFileName, "..") != 0)) {
			if (m_FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {	
				tvi.mask = TVIF_TEXT;
				tvi.pszText = m_FindFileData.cFileName;
				tvins.hParent = m_lParam->itemNew.hItem;
				tvins.item = tvi;
				tvins.hParent = TreeView_InsertItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), &tvins);
				TreeView_InsertItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), &tvins);
			}
		}
		if (!FindNextFile(m_handle, &m_FindFileData)) break;
	}
	FindClose(m_handle);
}

void 
FileTransfer::ProcessDlgMessage(HWND hwnd)
{
	MSG msg;
	while(PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void 
FileTransfer::BlockingFileTransferDialog(BOOL status)
{
	EnableWindow(m_hwndFTClientList, status);
	EnableWindow(m_hwndFTServerList, status);
	EnableWindow(m_hwndFTClientPath, status);
	EnableWindow(m_hwndFTServerPath, status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_UPLOAD), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_DOWNLOAD), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTUP), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_SERVERUP), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTRELOAD), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_SERVERRELOAD), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTBROWSE_BUT), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_SERVERBROWSE_BUT), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_EXIT), status);
}

void 
FileTransfer::ShowServerItems()
{
	rfbFileListDataMsg fld;
	m_clientconn->ReadExact((char *) &fld, sz_rfbFileListDataMsg);
	if ((fld.flags & 0x80) && !m_bServerBrowseRequest) {
		BlockingFileTransferDialog(TRUE);
		return;
	}
	fld.numFiles = Swap16IfLE(fld.numFiles);
	fld.dataSize = Swap16IfLE(fld.dataSize);
	fld.compressedSize = Swap16IfLE(fld.compressedSize);
	FTSIZEDATA *pftSD = new FTSIZEDATA[fld.numFiles];
	char *pFilenames = new char[fld.dataSize];
	m_clientconn->ReadExact((char *)pftSD, fld.numFiles * 8);
	m_clientconn->ReadExact(pFilenames, fld.dataSize);
	if (!m_bServerBrowseRequest) {
		if (fld.numFiles == 0) {
			BlockingFileTransferDialog(TRUE);
			strcpy(m_ServerPath, m_ServerPathTmp);
			SetWindowText(m_hwndFTServerPath, m_ServerPath);
			ListView_DeleteAllItems(m_hwndFTServerList); 
			delete [] pftSD;
			delete [] pFilenames;
			return;
		} else {
			m_FTServerItemInfo.Free();
			ListView_DeleteAllItems(m_hwndFTServerList); 
			strcpy(m_ServerPath, m_ServerPathTmp);
			SetWindowText(m_hwndFTServerPath, m_ServerPath);
			CreateServerItemInfoList(&m_FTServerItemInfo, pftSD, fld.numFiles, pFilenames, fld.dataSize);
			m_FTServerItemInfo.Sort();
			ShowListViewItems(m_hwndFTServerList, &m_FTServerItemInfo);
		}
	} else {
		while (TreeView_GetChild(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), m_hTreeItem) != NULL) {
			TreeView_DeleteItem(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), TreeView_GetChild(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), m_hTreeItem));
		}
		TVITEM TVItem;
		TVINSERTSTRUCT tvins; 
		TVItem.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_HANDLE;
		TVItem.cChildren = 1;
		int pos = 0;
		for (int i = 0; i < fld.numFiles; i++) {
			if (pftSD[i].size == -1) {
				TVItem.pszText = pFilenames + pos;
				TVItem.cChildren = 1;
				tvins.item = TVItem;
				tvins.hParent = m_hTreeItem;
				tvins.hParent = TreeView_InsertItem(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), &tvins);
				tvins.item = TVItem;
				TreeView_InsertItem(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), &tvins);
				tvins.hParent = m_hTreeItem;;
			}
			pos += strlen(pFilenames + pos) + 1;
		}
	}
	delete [] pftSD;
	delete [] pFilenames;
	BlockingFileTransferDialog(TRUE);
}

void 
FileTransfer::SendFileListRequestMessage(char *filename, unsigned char flags)
{
	char _filename[rfbMAX_PATH];
	strcpy(_filename, filename);
	int len = strlen(_filename);
	if (_filename[len-1] == '\\') _filename[len-1] = '\0';
	ConvertPath(_filename);
	len = strlen(_filename);
	rfbFileListRequestMsg flr;
	flr.type = rfbFileListRequest;
	flr.dirNameSize = Swap16IfLE(len);
	flr.flags = flags;
	m_clientconn->WriteExact((char *)&flr, sz_rfbFileListRequestMsg);
	m_clientconn->WriteExact(_filename, len);
}

void 
FileTransfer::ProcessListViewDBLCLK(HWND hwnd, char *Path, char *PathTmp, int iItem)
{
	SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
	SetWindowText(m_hwndFTStatus, "");
	strcpy(PathTmp, Path);
	char buffer[rfbMAX_PATH];
	char buffer_tmp[16];
	ListView_GetItemText(hwnd, iItem, 0, buffer, rfbMAX_PATH);
	ListView_GetItemText(hwnd, iItem, 1, buffer_tmp, 16);
	if (strcmp(buffer_tmp, m_FTClientItemInfo.folderText) == 0) {
			BlockingFileTransferDialog(FALSE);
			if (strlen(PathTmp) >= 2) strcat(PathTmp, "\\");
			strcat(PathTmp, buffer);
			if (hwnd == m_hwndFTClientList) ShowClientItems(PathTmp);
			if (hwnd == m_hwndFTServerList) SendFileListRequestMessage(PathTmp, 0);
	}
}

void
FileTransfer::ConvertPath(char *path)
{
	int len = strlen(path);
	if (len >= rfbMAX_PATH) return;
	if (strcmp(path, "") == 0) {strcpy(path, "/"); return;}
	for (int i = (len - 1); i >= 0; i--) {
		if (path[i] == '\\') path[i] = '/';
		path[i+1] = path[i];
	}
	path[len + 1] = '\0';
	path[0] = '/';
	return;
}

void 
FileTransfer::ShowListViewItems(HWND hwnd, FileTransferItemInfo *ftii)
{
	LVITEM LVItem;
	LVItem.mask = LVIF_TEXT | LVIF_STATE; 
	LVItem.state = 0; 
	LVItem.stateMask = 0; 
	for (int i=0; i<ftii->GetNumEntries(); i++) {
		LVItem.iItem = i;
		LVItem.iSubItem = 0;
		LVItem.pszText = LPSTR_TEXTCALLBACK;
		ListView_InsertItem(hwnd, &LVItem);
	}
}

void
FileTransfer::FTInsertColumn(HWND hwnd, char *iText, int iOrder, int xWidth)
{
  LVCOLUMN lvc; 
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
  lvc.fmt = LVCFMT_RIGHT;
  lvc.iSubItem = iOrder;
  lvc.pszText = iText;	
  lvc.cchTextMax = 10;
  lvc.cx = xWidth;
  lvc.iOrder = iOrder;
  ListView_InsertColumn(hwnd, iOrder, &lvc);
}

void 
FileTransfer::InitProgressBar(int nPosition, int nMinRange, int nMaxRange, int nStep)
{
  SendMessage(m_hwndFTProgress, PBM_SETPOS, (WPARAM) nPosition, (LPARAM) 0);
  SendMessage(m_hwndFTProgress, PBM_SETRANGE, (WPARAM) 0, MAKELPARAM(nMinRange, nMaxRange)); 
  SendMessage(m_hwndFTProgress, PBM_SETSTEP, (WPARAM) nStep, 0); 
}

void 
FileTransfer::CreateServerItemInfoList(FileTransferItemInfo *pftii, 
                  										 FTSIZEDATA *ftsd, int ftsdNum,
											                 char *pfnames, int fnamesSize)
{
  int pos = 0;
  for (int i = 0; i < ftsdNum; i++) {
    char buf[16];
    ftsd[i].size = Swap32IfLE(ftsd[i].size);
    ftsd[i].data = Swap32IfLE(ftsd[i].data);
    if (ftsd[i].size == -1) {
      strcpy(buf, FileTransferItemInfo::folderText);
    } else {
      sprintf(buf, "%d", ftsd[i].size);
    }
    pftii->Add(pfnames + pos, buf, ftsd[i].data);
    pos += strlen(pfnames + pos) + 1;
  }
}

void 
FileTransfer::SendFileUploadDataMessage(unsigned int mTime)
{
	rfbFileUploadDataMsg msg;
	msg.type = rfbFileUploadData;
	msg.compressedLevel = 0;
	msg.realSize = Swap16IfLE(0);
	msg.compressedSize = Swap16IfLE(0);

	CARD32 time32 = Swap32IfLE((CARD32)mTime);

	char data[sz_rfbFileUploadDataMsg + sizeof(CARD32)];
	memcpy(data, &msg, sz_rfbFileUploadDataMsg);
	memcpy(&data[sz_rfbFileUploadDataMsg], &time32, sizeof(CARD32));

	m_clientconn->WriteExact(data, sz_rfbFileUploadDataMsg + sizeof(CARD32));
}

void 
FileTransfer::SendFileUploadDataMessage(unsigned short size, char *pFile)
{
	int msgLen = sz_rfbFileUploadDataMsg + size;
	char *pAllFUDMessage = new char[msgLen];
	rfbFileUploadDataMsg *pFUD = (rfbFileUploadDataMsg *) pAllFUDMessage;
	char *pFollow = &pAllFUDMessage[sz_rfbFileUploadDataMsg];
	pFUD->type = rfbFileUploadData;
	pFUD->compressedLevel = 0;
	pFUD->realSize = Swap16IfLE(size);
	pFUD->compressedSize = Swap16IfLE(size);
	memcpy(pFollow, pFile, size);
	m_clientconn->WriteExact(pAllFUDMessage, msgLen);
	delete [] pAllFUDMessage;
}

void 
FileTransfer::SendFileDownloadCancelMessage(unsigned short reasonLen, char *reason)
{
  int msgLen = sz_rfbFileDownloadCancelMsg + reasonLen;
  char *pAllFDCMessage = new char[msgLen];
  rfbFileDownloadCancelMsg *pFDC = (rfbFileDownloadCancelMsg *) pAllFDCMessage;
  char *pFollow = &pAllFDCMessage[sz_rfbFileDownloadCancelMsg];
  pFDC->type = rfbFileDownloadCancel;
  pFDC->reasonLen = Swap16IfLE(reasonLen);
  memcpy(pFollow, reason, reasonLen);
  m_clientconn->WriteExact(pAllFDCMessage, msgLen);
  delete [] pAllFDCMessage;
}

void
FileTransfer::ReadUploadCancel()
{
  if (m_bUploadStarted) {
    m_bUploadStarted = FALSE;
    CloseHandle(m_hFiletoRead);
  }
	// Stop file transfer
	CloseUndoneFileTransfers();

	// Read the message
	rfbFileUploadCancelMsg msg;
	m_clientconn->ReadExact((char *)&msg, sz_rfbFileUploadCancelMsg);
	int len = Swap16IfLE(msg.reasonLen);
	char *reason = new char[len + 1];
	m_clientconn->ReadExact(reason, len);
	reason[len] = '\0';

	// Report error (only once per upload)
	if (m_bReportUploadCancel) {
		char *errmsg = new char[128 + len];
		sprintf(errmsg, "Upload failed: %s", reason);
		MessageBox(m_hwndFileTransfer, errmsg, "Upload Failed", MB_ICONEXCLAMATION | MB_OK);
		SetWindowText(m_hwndFTStatus, errmsg);
		vnclog.Print(1, _T("Upload failed: %s\n"), reason);
		m_bReportUploadCancel = FALSE;
		delete[] errmsg;
	}
	delete[] reason;

	// Enable dialog
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
	BlockingFileTransferDialog(TRUE);
}

void
FileTransfer::ReadDownloadFailed()
{
	// We'll report the error only if we're actually downloading
	BOOL downloadActive = m_bDownloadStarted;

	// Stop file transfer
	CloseUndoneFileTransfers();

	// Read the message
	rfbFileDownloadFailedMsg msg;
	m_clientconn->ReadExact((char *)&msg, sz_rfbFileDownloadFailedMsg);
	int len = Swap16IfLE(msg.reasonLen);
	char *reason = new char[len + 1];
	m_clientconn->ReadExact(reason, len);
	reason[len] = '\0';

	// Report error
	if (downloadActive) {
		char *errmsg = new char[128 + len];
		sprintf(errmsg, "Download failed: %s", reason);
		MessageBox(m_hwndFileTransfer, errmsg, "Download Failed", MB_ICONEXCLAMATION | MB_OK);
		SetWindowText(m_hwndFTStatus, errmsg);
		vnclog.Print(1, _T("Download failed: %s\n"), reason);
		delete[] errmsg;
	}
	delete[] reason;

	// Enable dialog
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), FALSE);
	BlockingFileTransferDialog(TRUE);
}

unsigned int FileTransfer::FiletimeToTime70(FILETIME ftime)
{
	LARGE_INTEGER uli;
	uli.LowPart = ftime.dwLowDateTime;
	uli.HighPart = ftime.dwHighDateTime;
	uli.QuadPart = (uli.QuadPart - 116444736000000000) / 10000000;
	return uli.LowPart;
}

void FileTransfer::Time70ToFiletime(unsigned int time70, FILETIME *pftime)
{
    LONGLONG ll = Int32x32To64(time70, 10000000) + 116444736000000000;
    pftime->dwLowDateTime = (DWORD) ll;
    pftime->dwHighDateTime = (DWORD)(ll >> 32);
}
