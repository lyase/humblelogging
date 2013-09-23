#include "logger.h"

#include <string>
#include <list>
#include <iostream>

#include "appender.h"

namespace humble {
namespace logging {

Logger::Logger(const std::string &name, int level)
  : _name(name), _level(level), _appenders()
{ 

}

Logger::~Logger()
{
  _appenders.clear();
}

Logger& Logger::setName(const std::string &name)
{
  _name = name;
  return (*this);
}

const std::string& Logger::getName() const
{
  return _name;
}

Logger& Logger::setLogLevel(int level)
{
  _level = level;
  return (*this);
}

int Logger::getLogLevel() const
{
  return _level;
}

Logger& Logger::addAppender(Appender *appender)
{
  _appenders.push_back(appender);
  return (*this);
}

std::list<Appender*> Logger::getAppenders() const
{
  return _appenders;
}

Logger& Logger::log(int level, const std::string &message)
{
  if (_level >= level) {
    for (std::list<Appender*>::iterator i = _appenders.begin(), e = _appenders.end(); i != e; ++i) {
      (*i)->log(level, message);
    }
  }
  return (*this);
}

}}  // End of namespaces.
