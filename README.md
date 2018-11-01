## Signal handler for Java 8

Frustrated with signal handling in Java as I was working with a Java application managed by systemd, I decided to have a try at coding my own for Linux. I opted for more C over Java mostly for the ease of working with the system calls and limiting native entrypoints.

Only after I got an end to end test working did I stumble on `sun.misc.Signal`. How that is I may never know ,as I did a thorough digging through Oracle docs and StackOverflow. All the same it's been an enlightening exercise.

The short back story is the JVM doesn't behave gracefully when various signals used by tools like systemd are sent to it. This makes writing services managed by such tools frustrating, as signals like SIGHUP or SIGINT can often be handled gracefully and terminating an application safely can become a game of chance. Even more frustrating is when these services run in containers since local state--log data included--can be lost, making the hard-enough task of identifying what went wrong even harder.

This project hopefully helps prevent some of this frustration, or at least makes managing it easier.

### Signal handling behavior

Allowed signals are captured by the `com.markwat.system.signals.LinuxSigNo` class. The C code does the full validation of signal usage and throws errors/exceptions as appropriate. Signal handlers registered by the JVM aren't thrown away--they are retained and chained after the user-registered handler is added.

The current approach registers a single handler for all signals. All user handler callbacks are chained as a LIFO stack and upon handling the registered signal are executed sequentially, followed by executing the original JVM signal handler. This will be overridable as handling a signal should first conform to the established (literal) interface of having a single handler, with chaining being a secondary feature (IMO).

### Exception handling

If exceptions are encountered, they will be cleared and ignored by the C code. As threads handling a signal don't strictly have a JVM call stack from whence they came, ignoring the exceptions is the simplest solution. An async handler may be involved in the future, but the argument remains the same: still possible for that to generate exceptions, too. Rather than killing off the JVM, the implementation opts to allow the chain to continue without error.

### Notes and quirks

The handlers can't be removed once added. This implementation currently leans towards using signals for low system-level notification that lasts the duration of the JVM. While signals are useful tools, complex usage and interchanging of handlers may become problematic since the JVM depends on the OS to behave on the JVM's terms--not the developer's; so for now, treading carefully.

The C code needs lots of work :). JNI examples are scant and it's hard to find "good practices". Since global refs can get hairy, the initial offering uses them where strictly necessary. Improvements will be made to improve class references, method references, etc.