#include "factory.h"

#include <list>
#include <string>
#include <algorithm>

#include "logger.h"
#include "appender.h"
#include "loglevel.h"
#include "formatter.h"

namespace humble {
namespace logging {

Factory::Factory()
  : _level(LogLevel::All),
    _defaultFormatter(new SimpleFormatter())
{
}

Factory::~Factory()
{
  std::lock_guard<std::mutex> lock(_mutex);
  while (!_loggers.empty()) {
    Logger *l = _loggers.front();
    _loggers.pop_front();
    delete l;
  }
  while (!_appenders.empty()) {
    Appender *a = _appenders.front();
    _appenders.pop_front();
    delete a;
  }
  if (_defaultFormatter) {
    delete _defaultFormatter;
    _defaultFormatter = NULL;
  }
}

Factory& Factory::getInstance()
{
  static Factory _instance;
  return _instance;
}

Factory& Factory::registerAppender(Appender *appender)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _appenders.push_back(appender);
  configure();
  return (*this);
}

Logger& Factory::getLogger(const std::string &name)
{
  std::lock_guard<std::mutex> lock(_mutex);
  Logger *l = 0;
  for (std::list<Logger*>::iterator i = _loggers.begin(); i != _loggers.end(); ++i) {
    if ((*i)->getName().compare(name) == 0) {
      l = (*i);
      break;
    }
  }
  if (l == 0) {
    l = new Logger(name);
    l->setLogLevel(_level);
    _loggers.push_back(l);
    configure();
  }
  return (*l);
}

Factory& Factory::setDefaultLogLevel(int level)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _level = level;
  return *this;
}

Factory& Factory::setDefaultFormatter(Formatter *formatter)
{
  if (!formatter) {
    return *this;
  }
  std::lock_guard<std::mutex> lock(_mutex);
  if (_defaultFormatter) {
    delete _defaultFormatter;
  }
  _defaultFormatter = formatter;
  return *this;
}

Formatter* Factory::getDefaultFormatter() const
{
  return _defaultFormatter;
}

Factory& Factory::changeGlobalLogLevel(int level)
{
  std::lock_guard<std::mutex> lock(_mutex);
  for (std::list<Logger*>::iterator i = _loggers.begin(); i != _loggers.end(); ++i) {
    (*i)->setLogLevel(level);
  }
  return *this;
}

void Factory::configure()
{
  for (std::list<Logger*>::iterator i = _loggers.begin(); i != _loggers.end(); ++i) {
    for (std::list<Appender*>::iterator a = _appenders.begin(); a != _appenders.end(); ++a) {
      Appender *appender = (*a);
      if (!appender->getFormatter()) {
        appender->setFormatter(_defaultFormatter->copy());
      }
      if ((*i)->hasAppender(appender))
        continue;
      (*i)->addAppender(appender);
    }
  }
}

}}  // End of namespaces.
