/* http_defs.cc
   Mathieu Stefani, 01 September 2015
   
   Implementation of http definitions
*/

#include <iostream>
#include <iomanip>

#include <pistache/http_defs.h>
#include <pistache/common.h>
#include <pistache/date.h>

namespace Pistache {
namespace Http {

namespace {
    using time_point = FullDate::time_point;    
    
    bool parse_RFC_1123(const std::string& s, time_point &tp)
    {
        std::istringstream in{s};
        in >> date::parse("%a, %d %b %Y %T %Z", tp);
        return !in.fail();
    }

    bool parse_RFC_850(const std::string& s, time_point &tp)
    {
        std::istringstream in{s};
        in >> date::parse("%a, %d-%b-%y %T %Z", tp);
        return !in.fail();
    }

    bool parse_asctime(const std::string& s, time_point &tp)
    {
        std::istringstream in{s};
        in >> date::parse("%a %b %d %T %Y", tp);
        return !in.fail();
    }
    
} // anonymous namespace

CacheDirective::CacheDirective(Directive directive)
{
    init(directive, std::chrono::seconds(0));
}

CacheDirective::CacheDirective(Directive directive, std::chrono::seconds delta)
{
    init(directive, delta);
}

std::chrono::seconds
CacheDirective::delta() const
{
    switch (directive_) {
        case MaxAge:
            return std::chrono::seconds(data.maxAge);
        case SMaxAge:
            return std::chrono::seconds(data.sMaxAge);
        case MaxStale:
            return std::chrono::seconds(data.maxStale);
        case MinFresh:
            return std::chrono::seconds(data.minFresh);
    }

    throw std::domain_error("Invalid operation on cache directive");    
}

void
CacheDirective::init(Directive directive, std::chrono::seconds delta)
{
    directive_ = directive;
    switch (directive) {
        case MaxAge:
            data.maxAge = delta.count();
            break;
        case SMaxAge:
            data.sMaxAge = delta.count();
            break;
        case MaxStale:
            data.maxStale = delta.count();
            break;
        case MinFresh:
            data.minFresh = delta.count();
            break;
    }
}

FullDate
FullDate::fromString(const std::string& str) {
    
    FullDate::time_point tp;
    if(parse_RFC_1123(str, tp))
        return FullDate(tp);
    else if(parse_RFC_850(str, tp))
        return FullDate(tp);
    else if(parse_asctime(str, tp))
        return FullDate(tp);
    
    throw std::runtime_error("Invalid Date format");
}

void
FullDate::write(std::ostream& os, Type type) const
{
    switch (type) {
    case Type::RFC1123:
        date::to_stream(os, "%a, %d %b %Y %T %Z", date_);
        break;
    case Type::RFC850:
        date::to_stream(os, "%a, %d-%b-%y %T %Z", date_);
        break;
    case Type::AscTime:
        date::to_stream(os, "%a %b %d %T %Y", date_);
        break;
    default:
        std::runtime_error("Invalid use of FullDate::write");
    }
}

const char *versionString(Version version) {
    switch (version) {
        case Version::Http10:
            return "HTTP/1.0";
        case Version::Http11:
            return "HTTP/1.1";
    }

    unreachable();
}

const char* methodString(Method method)
{
    switch (method) {
#define METHOD(name, str) \
    case Method::name: \
        return str;
    HTTP_METHODS
#undef METHOD
    }

    unreachable();
}

const char* codeString(Code code)
{
    switch (code) {
#define CODE(_, name, str) \
    case Code::name: \
         return str;
    STATUS_CODES
#undef CODE
    }

    return "";
}

std::ostream& operator<<(std::ostream& os, Version version) {
    os << versionString(version);
    return os;
}

std::ostream& operator<<(std::ostream& os, Method method) {
    os << methodString(method);
    return os;
}

std::ostream& operator<<(std::ostream& os, Code code) {
    os << codeString(code);
    return os;
}

HttpError::HttpError(Code code, std::string reason)
    : code_(static_cast<int>(code))
    , reason_(std::move(reason))
{ }

HttpError::HttpError(int code, std::string reason)
    : code_(code)
    , reason_(std::move(reason))
{ }


} // namespace Http
} // namespace Pistache
