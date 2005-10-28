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

#if !defined(FILETRANSFER)
#define FILETRANSFER

#include "windows.h"
#include "commctrl.h"
#include "ClientConnection.h"
#include "FileTransferItemInfo.h"

class ClientConnection;

class FileTransfer  
{
private:
	static const char uploadText[];
	static const char downloadText[];
	static const char noactionText[];

public:
	FileTransfer(ClientConnection * pCC, VNCviewerApp * pApp);
	~FileTransfer();

	void FTInsertColumn(HWND hwnd, char *iText, int iOrder, int xWidth);
	void CreateFileTransferDialog();
	void ShowListViewItems(HWND hwnd, FileTransferItemInfo *ftii);
	void ConvertPath(char *path);
	void ProcessListViewDBLCLK(HWND hwnd, char *Path, char *PathTmp, int iItem);
	void SendFileListRequestMessage(char *filename, unsigned char flags);
	void ShowServerItems();
	void ShowClientItems(char *path);
	void BlockingFileTransferDialog(BOOL status);
	void ProcessDlgMessage(HWND hwnd);
	void ShowTreeViewItems(HWND hwnd, LPNMTREEVIEW m_lParam);
	void CreateFTBrowseDialog(BOOL status);
	void StrInvert(char *str);
	void GetTVPath(HWND hwnd, HTREEITEM hTItem, char *path);
	char m_ServerPath[rfbMAX_PATH];
	char m_ClientPath[rfbMAX_PATH];
	char m_ServerPathTmp[rfbMAX_PATH];
	char m_ClientPathTmp[rfbMAX_PATH];
	char m_ServerFilename[rfbMAX_PATH];
	char m_ClientFilename[rfbMAX_PATH];
	char m_UploadFilename[rfbMAX_PATH];
	char m_DownloadFilename[rfbMAX_PATH];
	void OnGetDispClientInfo(NMLVDISPINFO *plvdi); 
	void OnGetDispServerInfo(NMLVDISPINFO *plvdi); 
	static LRESULT CALLBACK FileTransferDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK FTBrowseDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void FileTransferDownload();
	void FileTransferUpload();
	void CloseUndoneFileTransfers();

	void ReadUploadCancel();
	void ReadDownloadFailed();

	ClientConnection * m_clientconn;
	VNCviewerApp * m_pApp; 
	
private:
	DWORD m_dwDownloadRead;
	DWORD m_dwDownloadBlockSize;
	int m_sizeDownloadFile;
	void Time70ToFiletime(unsigned int time70, FILETIME *pftime);
	unsigned int FiletimeToTime70(FILETIME ftime);
	void SendFileUploadDataMessage(unsigned short size, char *pFile);
	void SendFileUploadDataMessage(unsigned int mTime);
	void CancelDownload(char *reason);
	void SendFileDownloadCancelMessage(unsigned short reasonLen, char *reason);
	void CreateServerItemInfoList(FileTransferItemInfo *pftii, FTSIZEDATA *ftsd, int ftsdNum, char *pfnames, int fnamesSize);
	void InitProgressBar(int nPosition, int nMinRange, int nMaxRange, int nStep);
	HWND m_hwndFileTransfer;
	HWND m_hwndFTClientList;
	HWND m_hwndFTServerList;
	HWND m_hwndFTClientPath;
	HWND m_hwndFTServerPath;
	HWND m_hwndFTProgress;
	HWND m_hwndFTStatus;
	HWND m_hwndFTBrowse;
	
	BOOL m_bFTCOPY;
    BOOL m_bUploadStarted;
    BOOL m_bDownloadStarted;
	BOOL m_bTransferEnable;
	BOOL m_bReportUploadCancel;
	BOOL m_bServerBrowseRequest;
	BOOL m_bFirstFileDownloadMsg;

	HANDLE m_hFiletoWrite;
    HANDLE m_hFiletoRead;
	HTREEITEM m_hTreeItem;
	HINSTANCE m_FTInstance;

	FileTransferItemInfo m_FTClientItemInfo;
	FileTransferItemInfo m_FTServerItemInfo;
};

#endif // !defined(FILETRANSFER)
