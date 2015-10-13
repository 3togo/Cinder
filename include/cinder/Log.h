/*
 Copyright (c) 2014, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Filesystem.h"
#include "cinder/CurrentFunction.h"
#include "cinder/CinderAssert.h"
#include "cinder/System.h"

#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <mutex>

#if defined( CINDER_MSW ) && ( _MSC_VER < 1800 )
	#define CINDER_NO_VARIADIC_TEMPLATES
	#include <boost/preprocessor/repetition.hpp>
	#include <boost/preprocessor/control/if.hpp>
#endif

#if defined( CINDER_ANDROID )
 	#include <sstream>
#endif 

#define CINDER_LOG_STREAM( level, stream ) ::cinder::log::Entry( level, ::cinder::log::Location( CINDER_CURRENT_FUNCTION, __FILE__, __LINE__ ) ) << stream

// CI_MAX_LOG_LEVEL is designed so that if you set it to 0, nothing logs, 1 only fatal, 2 fatal + error, etc...

#if ! defined( CI_MAX_LOG_LEVEL )
	#if ! defined( NDEBUG )
		#define CI_MAX_LOG_LEVEL 5	// debug mode default is LEVEL_VERBOSE
	#else
		#define CI_MAX_LOG_LEVEL 4	// release mode default is LEVEL_INFO
	#endif
#endif

#if( CI_MAX_LOG_LEVEL >= 5 )
	#define CI_LOG_V( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_VERBOSE, stream )
#else
	#define CI_LOG_V( stream )	((void)0)
#endif

#if( CI_MAX_LOG_LEVEL >= 4 )
	#define CI_LOG_I( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_INFO, stream )
#else
	#define CI_LOG_I( stream )	((void)0)
#endif

// Use the same level as INFO for now
#if defined( CINDER_ANDROID )
	#if( CI_MAX_LOG_LEVEL >= 4 )
		#define CI_LOG_D( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_DEBUG, stream )
	#else
		#define CI_LOG_D( stream )	((void)0)
	#endif
#endif

#if( CI_MAX_LOG_LEVEL >= 3 )
	#define CI_LOG_W( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_WARNING, stream )
#else
	#define CI_LOG_W( stream )	((void)0)
#endif

#if( CI_MAX_LOG_LEVEL >= 2 )
	#define CI_LOG_E( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_ERROR, stream )
#else
	#define CI_LOG_E( stream )	((void)0)
#endif

#if( CI_MAX_LOG_LEVEL >= 1 )
	#define CI_LOG_F( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_FATAL, stream )
#else
	#define CI_LOG_F( stream )	((void)0)
#endif

namespace cinder { namespace log {

typedef enum {
	LEVEL_VERBOSE,
	LEVEL_DEBUG,
	LEVEL_INFO,
	LEVEL_WARNING,
	LEVEL_ERROR,
	LEVEL_FATAL
} Level;

typedef enum {
	OUTPUT_DEFAULT,
	OUTPUT_CONSOLE,
	OUTPUT_FILE
} Output;

struct Location {
	Location() {}

	Location( const std::string &functionName, const std::string &fileName, const size_t &lineNumber )
		: mFunctionName( functionName ), mFileName( fileName ), mLineNumber( lineNumber )
	{}

	const std::string&	getFileName() const				{ return mFileName; }
	const std::string&	getFunctionName() const			{ return mFunctionName; }
	size_t				getLineNumber() const			{ return mLineNumber; }

private:
	std::string mFunctionName, mFileName;
	size_t		mLineNumber;
};

struct Metadata {

	std::string toString() const;

	Level		mLevel;
	Location	mLocation;
};

extern std::ostream& operator<<( std::ostream &os, const Location &rhs );
extern std::ostream& operator<<( std::ostream &lhs, const Level &rhs );

class Logger {
  public:
	virtual ~Logger()	{}

	virtual void write( const Metadata &meta, const std::string &text ) = 0;

	void 	setTimestampEnabled( bool enable = true )	{ mTimeStampEnabled = enable; }
	bool 	isTimestampEnabled() const					{ return mTimeStampEnabled; }
	Output 	getOutput() const 							{ return mOutput; }

  protected:
	Logger( Output output = OUTPUT_DEFAULT ) : mTimeStampEnabled( false ), mOutput( output ) {}

	void writeDefault( std::ostream &stream, const Metadata &meta, const std::string &text );

  private:
	bool 	mTimeStampEnabled;
	Output 	mOutput;
};

class LoggerConsole : public Logger {
  public:
	void write( const Metadata &meta, const std::string &text ) override;
};

class LoggerFile : public Logger {
  public:
	// Standard loggerFile, will write to a single log file.  File appending is configurable.
	// ! If \a filePath is empty, uses the default ('cinder.log' next to app binary)
	LoggerFile( const fs::path &filePath = fs::path(), bool appendToExisting = true );
	// daily rotating logger, will write to a formatted log file, updated at the first log request
	// after midnight.
	// ! If \a folder or \a formatStr are empty, ignores request
	LoggerFile( const fs::path &folder, const std::string &formatStr, bool appendToExisting = true );
	virtual ~LoggerFile();

	void write( const Metadata &meta, const std::string &text ) override;

	const fs::path&		getFilePath() const		{ return mFilePath; }

  protected:
	fs::path	getDefaultLogFilePath() const;
	void		ensureDirectoryExists();

	fs::path		mFilePath;
	fs::path		mFolderPath;
	std::string		mDailyFormatStr;
	int				mYearDay;
	bool			mAppend;
	bool			mRotating;
	std::ofstream	mStream;
};

//! Logger that doesn't actually print anything, but triggers a breakpoint if a log event happens past a specified threshold
class LoggerBreakpoint : public Logger {
  public:
	LoggerBreakpoint( Level triggerLevel = LEVEL_ERROR )
		: mTriggerLevel( triggerLevel )
	{}

	void write( const Metadata &meta, const std::string &text ) override;

	void	setTriggerLevel( Level triggerLevel )	{ mTriggerLevel = triggerLevel; }
	Level	getTriggerLevel() const					{ return mTriggerLevel; }
  private:
	Level	mTriggerLevel;
};

//! Provides 'system' logging support. Uses syslog on platforms that have it, on MSW uses Windows Event Logging. \note Does nothing on WinRT.
class LoggerSystem : public Logger {
public:
	LoggerSystem();
	virtual ~LoggerSystem();
	
	void write( const Metadata &meta, const std::string &text ) override;
	void setLoggingLevel( Level minLevel ) { mMinLevel = minLevel; }
	
protected:
	Level mMinLevel;
#if defined( CINDER_COCOA )
	class ImplSysLog;
	std::unique_ptr<ImplSysLog> mImpl;
#elif defined( CINDER_MSW )
	class ImplEventLog;
	std::unique_ptr<ImplEventLog> mImpl;
#elif defined( CINDER_ANDROID )	
	class ImplLogCat;
	std::unique_ptr<ImplLogCat> mImpl;
#elif defined( CINDER_LINUX )
	class ImplConsole;
	std::unique_ptr<ImplConsole> mImpl;
#endif
};

//! \brief Logger that can log to multiple other Loggers.
//!
//! This is primarily used by LogManager as it's base Logger, when multiple log outputs are enabled (ex. console and file)
class LoggerMulti : public Logger {
  public:
	void add( Logger *logger )							{ mLoggers.push_back( std::unique_ptr<Logger>( logger ) ); }
	void add( std::unique_ptr<Logger> &&logger )		{ mLoggers.emplace_back( move( logger ) ); }

	template <typename LoggerT>
	LoggerT* findType();

	void remove( Logger *logger );

	const std::vector<std::unique_ptr<Logger> >& getLoggers() const	{ return mLoggers; }

	void write( const Metadata &meta, const std::string &text ) override;

  private:
	std::vector<std::unique_ptr<Logger> >	mLoggers;
};

class LogManager {
public:
	// Returns a pointer to the shared instance. To enable logging during shutdown, this instance is leaked at shutdown.
	static LogManager* instance()	{ return sInstance; }
	//! Destroys the shared instance. Useful to remove false positives with leak detectors like valgrind.
	static void destroyInstance()	{ delete sInstance; }
	//! Restores LogManager to its default state.
	void restoreToDefault();

	//! Resets the current Logger stack so only \a logger exists.
	void resetLogger( Logger *logger );
	//! Adds \a logger to the current stack of loggers.
	void addLogger( Logger *logger );
	//! Remove \a logger to the current stack of loggers.
	void removeLogger( Logger *logger );
	//! Returns a pointer to the current base Logger instance.
	Logger* getLogger()	{ return mLogger.get(); }
	//! Returns a logger of a specifc type, or nullptr is that type of Logger is currently not in use.
	template<typename LoggerT>
	LoggerT* getLogger();
	//! Returns a vector of all current loggers
	std::vector<Logger *> getAllLoggers();
	//! Returns the mutex used for thread safe loggers. Also used when adding or resetting new loggers.
	std::mutex& getMutex() const			{ return mMutex; }

	void enableConsoleLogging();
	void disableConsoleLogging();
	void setConsoleLoggingEnabled( bool enable )		{ enable ? enableConsoleLogging() : disableConsoleLogging(); }
	bool isConsoleLoggingEnabled() const				{ return mConsoleLoggingEnabled; }

	void enableFileLogging( const fs::path &filePath = fs::path(), bool appendToExisting = true );
	void enableFileLoggingRotating( const fs::path &folder, const std::string& formatStr, bool appendToExisting = true);
	void disableFileLogging();
	void setFileLoggingEnabled( bool enable, const fs::path &filePath = fs::path(), bool appendToExisting = true );
	void setFileLoggingRotatingEnabled( bool enable, const fs::path &folder, const std::string &formatStr, bool appendToExisting = true );
	bool isFileLoggingEnabled() const					{ return mFileLoggingEnabled; }

	void enableSystemLogging();
	void disableSystemLogging();
	void setSystemLoggingEnabled( bool enable = true )		{ enable ? enableSystemLogging() : disableSystemLogging(); }
	bool isSystemLoggingEnabled() const					{ return mSystemLoggingEnabled; }
	void setSystemLoggingLevel( Level level );
	Level getSystemLoggingLevel() const					{ return mSystemLoggingLevel; }

	//! Enables a breakpoint to be triggered when a log message happens at `LEVEL_ERROR` or higher
	void enableBreakOnError()							{ enableBreakOnLevel( LEVEL_ERROR ); }
	//! Enables a breakpoint to be triggered when a log message happens at \a trigerLevel or higher.
	void enableBreakOnLevel( Level trigerLevel );
	//! Disables any breakpoints set for logging.
	void disableBreakOnLog();

protected:
	LogManager();

	bool initFileLogging();

	std::unique_ptr<Logger>	mLogger;
	LoggerMulti*			mLoggerMulti;
	mutable std::mutex		mMutex;
	bool					mConsoleLoggingEnabled, mFileLoggingEnabled, mSystemLoggingEnabled, mBreakOnLogEnabled;
	Level					mSystemLoggingLevel;

	static LogManager *sInstance;
};

LogManager* manager();

struct Entry {
	// TODO: move &&location
	Entry( Level level, const Location &location )
		: mHasContent( false )
	{
		mMetaData.mLevel = level;
		mMetaData.mLocation = location;
	}

	~Entry()
	{
		if( mHasContent )
			writeToLog();
	}

	template <typename T>
	Entry& operator<<( const T &rhs )
	{
		mHasContent = true;
		mStream << rhs;
		return *this;
	}

	void writeToLog()
	{
		manager()->getLogger()->write( mMetaData, mStream.str() );
	}

	const Metadata&	getMetaData() const	{ return mMetaData; }

private:

	Metadata			mMetaData;
	bool				mHasContent;
	std::stringstream	mStream;
};


template<class LoggerT>
class ThreadSafeT : public LoggerT {
  public:
	template <typename... Args>
	ThreadSafeT( Args &&... args )
	: LoggerT( std::forward<Args>( args )... )
	{}

	void write( const Metadata &meta, const std::string &text ) override
	{
		std::lock_guard<std::mutex> lock( manager()->getMutex() );
		LoggerT::write( meta, text );
	}
};

typedef ThreadSafeT<LoggerConsole>		LoggerConsoleThreadSafe;
typedef ThreadSafeT<LoggerFile>			LoggerFileThreadSafe;

// ----------------------------------------------------------------------------------
// Template method implementations

template <typename LoggerT>
LoggerT* LoggerMulti::findType()
{
	for( const auto &logger : mLoggers ) {
		auto result = dynamic_cast<LoggerT *>( logger.get() );
		if( result )
			return result;
	}

	return nullptr;
}

template<typename LoggerT>
LoggerT* LogManager::getLogger()
{
	auto loggerMulti = dynamic_cast<LoggerMulti *>( mLogger.get() );
	if( loggerMulti ) {
		return loggerMulti->findType<LoggerT>();
	}
	else {
		return dynamic_cast<LoggerT *>( mLogger.get() );
	}
}

} } // namespace cinder::log

// ----------------------------------------------------------------------------------
// Logging macros

#define CINDER_LOG_STREAM( level, stream ) ::cinder::log::Entry( level, ::cinder::log::Location( CINDER_CURRENT_FUNCTION, __FILE__, __LINE__ ) ) << stream

// CI_MIN_LOG_LEVEL is designed so that if you set it to 7 : nothing logs, 6 : only fatal, 5 : fatal + error, ..., 1 : everything

#if ! defined( CI_MIN_LOG_LEVEL )
	#if ! defined( NDEBUG )
		#define CI_MIN_LOG_LEVEL 0	// debug mode default is LEVEL_VERBOSE
	#else
		#define CI_MIN_LOG_LEVEL 2	// release mode default is LEVEL_INFO
	#endif
#endif

#if( CI_MIN_LOG_LEVEL <= 0 )
	#define CI_LOG_V( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_VERBOSE, stream )
#else
	#define CI_LOG_V( stream )	((void)0)
#endif

#if( CI_MIN_LOG_LEVEL <= 1 )
#define CI_LOG_D( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_DEBUG, stream )
#else
#define CI_LOG_D( stream )	((void)0)
#endif

#if( CI_MIN_LOG_LEVEL <= 2 )
	#define CI_LOG_I( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_INFO, stream )
#else
	#define CI_LOG_I( stream )	((void)0)
#endif

#if( CI_MIN_LOG_LEVEL <= 3 )
	#define CI_LOG_W( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_WARNING, stream )
#else
	#define CI_LOG_W( stream )	((void)0)
#endif

#if( CI_MIN_LOG_LEVEL <= 4 )
	#define CI_LOG_E( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_ERROR, stream )
#else
	#define CI_LOG_E( stream )	((void)0)
#endif

#if( CI_MIN_LOG_LEVEL <= 5 )
	#define CI_LOG_F( stream )	CINDER_LOG_STREAM( ::cinder::log::LEVEL_FATAL, stream )
#else
	#define CI_LOG_F( stream )	((void)0)
#endif

//! Debug macro to simplify logging an exception, which also prints the exception type
#define CI_LOG_EXCEPTION( str, exc )	\
{										\
	CI_LOG_E( str << ", exception type: " << cinder::System::demangleTypeName( typeid( exc ).name() ) << ", what: " << exc.what() );	\
}

