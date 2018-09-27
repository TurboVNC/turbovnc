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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.

#if !defined(FILETRANSFERITEMINFO_H)
#define FILETRANSFERITEMINFO_H

#define rfbMAX_PATH 255


typedef struct tagFTITEMINFO {
  char Name[rfbMAX_PATH];
  char Size[16];
  unsigned int Data;
} FTITEMINFO;


typedef struct tagFTSIZEDATA {
  unsigned int size;
  unsigned int data;
} FTSIZEDATA;


class FileTransferItemInfo
{
  public:
    int GetIntSizeAt(int Number);
    static const char folderText[];
    int GetNumEntries();
    char *GetSizeAt(int Number);
    char *GetNameAt(int Number);
    unsigned int GetDataAt(int Number);
    bool IsFile(int Number);
    void Sort();
    void Free();
    void Add(char *Name, char *Size, unsigned int Data);
    FileTransferItemInfo();
    virtual ~FileTransferItemInfo();

  private:
    int ConvertCharToInt(char *pStr);
    FTITEMINFO *m_pEntries;
    int m_NumEntries;
};

#endif  // !defined(FILETRANSFERITEMINFO_H)
