// This is an object and macros which provide general logging and debugging functions.
// It can log to a file, to a new console, and/or to debug - others maybe to follow.
// Every log object has a logging level (which can be changed).
// Only log requests with a high enough level attached get logged. So the
// level can be thought of as 'amount of detail'.
// We use Unicode-portable stuff here for compatibility with WinCE.
//
// Typical use:
//
//       Log vnclog;
//       vnclog.SetFile( _T("myapp.log") );
//       ...
//       vnclog.Print(2, _T("x = %d\n"), x);
//

#ifndef VNCLOGGING
#define VNCLOGGING

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

class Log  
{
public:
    // Logging mode flags:
    static const int ToDebug;
    static const int ToFile;
    static const int ToConsole;

    // Create a new log object.
    // Parameters as follows:
    //    mode     - specifies where output should go, using combination
    //               of flags above.
    //    level    - the default level
    //    filename - if flag Log::ToFile is specified in the type,
    //               a filename must be specified here.
    //    append   - if logging to a file, whether or not to append to any
    //               existing log.
	Log(int mode = ToDebug, int level = 1, LPSTR filename = NULL, bool append = false);

    inline void Print(int level, LPSTR format, ...) {
        if (level > m_level) return;
        va_list ap;
        va_start(ap, format);
        ReallyPrint(format, ap);
        va_end(ap);
    }
    
    // Change the log level
    void SetLevel(int level);
    int  GetLevel();

    // Change the logging mode
    void SetMode(int mode);
    int  GetMode();

    // Change or set the logging filename.  This enables ToFile mode if
    // not already enabled.
    void SetFile(LPSTR filename, bool append = false);

	virtual ~Log();

private:
	void ReallyPrintLine(LPSTR line);
    void ReallyPrint(LPSTR format, va_list ap);
	void OpenFile();
    void CloseFile();
    bool m_tofile, m_todebug, m_toconsole;
    int m_level;
    int m_mode;
    HANDLE hlogfile;
	LPSTR m_filename;
	bool m_append;

	time_t m_lastLogTime;
};

#endif // VNCLOGGING
