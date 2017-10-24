#ifndef SRUBY_OS_H
#define SRUBY_OS_H

std::string addr2line(const std::string program_name, void const *const *addr, int count);

std::string getProgramName();

#endif // SRUBY_OS_H
