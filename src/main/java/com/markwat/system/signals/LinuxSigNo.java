package com.markwat.system.signals;

/**
 * Enumeration of valid signals to handle on Linux.
 * <p>
 * Notably, special signals like SIGKILL won't be present here--they should and will always fail.
 * <p>
 * See <a href="http://man7.org/linux/man-pages/man7/signal.7.html">signal(7)</a> for more info.
 */
public enum LinuxSigNo {
    SIGHUP(1), SIGINT(2);

    // for easy lookup, allocating max + 1
    // constant time lookup instead of enum iteration or map
    private static final LinuxSigNo[] SIGNALS = new LinuxSigNo[32];

    static {
        SIGNALS[1] = SIGHUP;
        SIGNALS[2] = SIGINT;
    }

    private final int signo;

    LinuxSigNo(int signo) {
        this.signo = signo;
    }

    public int getSigno() {
        return signo;
    }

    /**
     * Get the signal by its signal number.
     *
     * @param signo the signal number
     * @return the corresponding LinuxSigNo
     */
    public static LinuxSigNo valueOf(int signo) {
        // todo add better exception
        if (signo >= SIGNALS.length)
            throw new RuntimeException("current implementation doesn't manage realtime signals");
        else if (SIGNALS[signo] == null)
            throw new RuntimeException(String.format("signal, %s, not implemented", signo));
        else
            return SIGNALS[signo];
    }
}
