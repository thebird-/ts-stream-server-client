#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include "log4cpp/Category.hh"
#pragma warning(pop)

#include "Report.h"

struct SessionManagerCallback
{
    virtual log4cpp::Category& GetLogger() = 0;
    virtual std::string const& GetSourceFolder() = 0;
    virtual Report& GetReporter() = 0;
    virtual void OnSessionFinished(intptr_t token) = 0;

protected:
    virtual ~SessionManagerCallback() = 0
    {
    }
};
