// Log.cpp: implementation of the Log class.
//
//////////////////////////////////////////////////////////////////////

#include "stdhdrs.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const int Log::ToDebug   =  1;
const int Log::ToFile    =  2;
const int Log::ToConsole =  4;

const static int LINE_BUFFER_SIZE = 1024;

Log::Log(int mode, int level, LPSTR filename, bool append)
{
	m_lastLogTime = 0;
	m_filename = NULL;
	m_append = false;
    hlogfile = NULL;
    m_todebug = false;
    m_toconsole = false;
    m_tofile = false;
    SetFile(filename, append);
    SetMode(mode);
}

void Log::SetMode(int mode) {
    
	m_mode = mode;

    if (mode & ToDebug)
        m_todebug = true;
    else
        m_todebug = false;

    if (mode & ToFile)  {
		if (!m_tofile)
			OpenFile();
	} else {
		CloseFile();
        m_tofile = false;
    }
    
    if (mode & ToConsole) {
        if (!m_toconsole)
            AllocConsole();
        m_toconsole = true;
    } else {
        m_toconsole = false;
    }
}

int Log::GetMode() {
	return m_mode;
}

void Log::SetLevel(int level) {
    m_level = level;
}

int Log::GetLevel() {
	return m_level;
}

void Log::SetFile(LPSTR filename, bool append) 
{
	CloseFile();
	if (m_filename != NULL)
		free(m_filename);
	m_filename = strdup(filename);
	m_append = append;
	if (m_tofile)
		OpenFile();
}

void Log::OpenFile()
{
	// Is there a file-name?
	if (m_filename == NULL)
	{
        m_todebug = true;
        m_tofile = false;
        Print(0, "Error opening log file\n");
		return;
	}

    m_tofile  = true;
    
	// If there's an existing log and we're not appending then move it
	if (!m_append)
	{
		// Build the backup filename
		char *backupfilename = new char[strlen(m_filename)+5];
		if (backupfilename)
		{
			strcpy(backupfilename, m_filename);
			strcat(backupfilename, ".bak");
			// Attempt the move and replace any existing backup
			// Note that failure is silent - where would we log a message to? ;)
			DeleteFile(backupfilename);
			MoveFile(m_filename, backupfilename);
			delete [] backupfilename;
		}
	}

    // If filename is NULL or invalid we should throw an exception here
    hlogfile = CreateFile(
        m_filename,  GENERIC_WRITE, FILE_SHARE_READ, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL  );
    
    if (hlogfile == INVALID_HANDLE_VALUE) {
        // We should throw an exception here
        m_todebug = true;
        m_tofile = false;
        Print(0, "Error opening log file %s\n", m_filename);
    }
    if (m_append) {
        SetFilePointer( hlogfile, 0, NULL, FILE_END );
    } else {
        SetEndOfFile( hlogfile );
    }
}

// if a log file is open, close it now.
void Log::CloseFile() {
    if (hlogfile != NULL) {
        CloseHandle(hlogfile);
        hlogfile = NULL;
    }
}

inline void Log::ReallyPrintLine(LPSTR line) 
{
    if (m_todebug) OutputDebugString(line);
    if (m_toconsole) {
        DWORD byteswritten;
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), line, strlen(line), &byteswritten, NULL); 
    };
    if (m_tofile && (hlogfile != NULL)) {
        DWORD byteswritten;
        WriteFile(hlogfile, line, strlen(line), &byteswritten, NULL); 
    }
}

void Log::ReallyPrint(LPSTR format, va_list ap) 
{
	// Write current time to the log if necessary
	time_t current = time(0);
	if (current != m_lastLogTime) {
		m_lastLogTime = current;

		char time_str[32];
		strncpy(time_str, ctime(&m_lastLogTime), 24);
		strcpy(&time_str[24], "\r\n");
		ReallyPrintLine(time_str);
	}

	// Prepare the complete log message
	TCHAR line[LINE_BUFFER_SIZE];
	_vsnprintf(line, sizeof(line) - 2 * sizeof(TCHAR), format, ap);
	line[LINE_BUFFER_SIZE-2] = (TCHAR)'\0';
#if (!defined(_UNICODE) && !defined(_MBCS))
	int len = strlen(line);
	if (len > 0 && len <= sizeof(line) - 2 * sizeof(TCHAR) && line[len-1] == (TCHAR)'\n') {
		// Replace trailing '\n' with MS-DOS style end-of-line.
		line[len-1] = (TCHAR)'\r';
		line[len] =   (TCHAR)'\n';
		line[len+1] = (TCHAR)'\0';
	}
#endif
	ReallyPrintLine(line);
}

Log::~Log()
{
	if (m_filename != NULL)
		free(m_filename);
    CloseFile();
}
