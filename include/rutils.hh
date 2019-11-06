#ifndef _RUTILS_H
#define _RUTILS_H

#include <string>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include <fstream>

/* R header files */
#include <R.h>
#include <Rdefines.h>
#include <Rembedded.h>
#include <Rinternals.h>
#include <Rversion.h>

// undef R marco to avoid conflict with protobuf
#undef Free
#undef length

#define MAX_SERVER_LOG_BUFFER_SIZE 2 * 1024 * 1024
#define MAX_MESSAGE_LINE_LENGTH 8 * 1024

enum class RServerLogLevel {
    DEBUGS = 1,
    LOGS,
    WARNINGS,
    ERRORS,
    UNKNOWN = 99,
};

enum class RServerWorkingMode {
    STANDALONG = 1,
    CONTAINER = 2,
    CONTAINERDEBUG = 3,
    UNKNOWN = 99,
};

class RServerErrorException : public std::exception {
   public:
    RServerErrorException() { this->msg = "R Server Runtime Error "; }
    RServerErrorException(std::string &msg) { this->msg = "R Server Runtime Error " + msg + " "; }

    virtual const char *what() const noexcept override { return this->msg.c_str(); }

   private:
    std::string msg;
};

class RServerWarningException : public std::exception {
   public:
    RServerWarningException() {
        this->msg = "R Server Runtime Warning ";
    };
    RServerWarningException(std::string &msg) {
        this->msg = "R Server Runtime Warning " + msg + " ";
    }

    virtual const char *what() const noexcept override { return this->msg.c_str(); }

   private:
    std::string msg;
};

class LogPrinter {
   public:
    virtual void print(std::string &logs) = 0;
    virtual ~LogPrinter() {};
};

class FileLogPrinter : public LogPrinter {
   public:
    FileLogPrinter(std::string filename) {
        this->file.open(filename, std::ios::out | std::ios::app);
    }

    virtual void print(std::string &logs) override;

    ~FileLogPrinter() { this->file.close(); }

   private:
    std::ofstream file;
};

class StdLogPrinter : public LogPrinter {
   public:
    virtual void print(std::string &logs) override;
};

class BufferLogPrinter : public LogPrinter {
   public:
    BufferLogPrinter(std::string *buffer) { this->bufferRef = buffer; }

    virtual void print(std::string &logs) override;

   private:
    std::string *bufferRef;
};

class RServerLog {
   public:
    explicit RServerLog(RServerWorkingMode mode, std::string filename);

    void log(RServerLogLevel lvl, const char *format, ...);
    void setLogLevel(RServerLogLevel level) { this->logLevel = level; }
    std::string getLogBuffer() { return this->localBuffer; }
    void resetLogBuffer() { this->localBuffer.clear(); }

    ~RServerLog() { delete printer; }

   private:
    RServerLogLevel logLevel;
    std::string localBuffer;
    RServerWorkingMode mode;
    LogPrinter *printer;

    std::string logLevelToString(RServerLogLevel level);
};

#endif  //_RUTILS_H
