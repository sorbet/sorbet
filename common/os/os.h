#ifndef SRUBY_OS_H
#define SRUBY_OS_H

std::string addr2line(const std::string program_name, void const *const *addr, int count);

std::string getProgramName();

/** The should trigger debugger breakpoint if the debugger is attached, if no debugger is attach, it should do nothing
 *  This allows to:
 *   - have "persistent" break points in development loop, that survive line changes.
 *   - test the same executable outside of debugger without rebuilding.
 * */
bool stopInDebugger();

#endif // SRUBY_OS_H
