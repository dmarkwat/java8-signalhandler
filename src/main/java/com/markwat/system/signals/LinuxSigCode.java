package com.markwat.system.signals;

/**
 * Select siginfo_t.si_code values. Ones like SI_TIMER or SI_MESGQ are obscured and
 * will map to UNKNOWN as they're deemed irrelevant for the signals in the implementation.
 *
 * <a href="http://man7.org/linux/man-pages/man2/sigaction.2.html">sigaction(2)</a>
 */
public enum LinuxSigCode {
    /**
     * <a href="http://man7.org/linux/man-pages/man2/kill.2.html">kill(2)</a>.
     */
    USER,
    /**
     * Sent by the kernel.
     */
    KERNEL,
    /**
     * <a href="http://man7.org/linux/man-pages/man3/sigqueue.3.html">sigqueue(3)</a>.
     */
    QUEUE,
    /**
     * <a href="http://man7.org/linux/man-pages/man2/tkill.2.html">tkill(2)</a> or <a href="http://man7.org/linux/man-pages/man2/tgkill.2.html">tgkill(2)</a>.
     */
    TKILL,
    /**
     * All other codes not mentioned above. See <a href="http://man7.org/linux/man-pages/man2/sigaction.2.html">sigaction(2)</a>.
     */
    UNKNOWN
}
