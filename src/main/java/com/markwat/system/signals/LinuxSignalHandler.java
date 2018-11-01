package com.markwat.system.signals;

public class LinuxSignalHandler {

    static {
        NarSystem.loadLibrary();
    }

    /**
     * Tells the OS to handle a particular signal when it comes through. These handlers are handled LIFO. For underlying
     * OS semantics, look at <a href="http://man7.org/linux/man-pages/man2/sigaction.2.html">sigaction(2)</a>.
     * <p>
     * The signal handler always chains regardless of whether an exception is thrown. The reason for this is the
     * JVM handles various signals for specific things in specific ways and we don't want to totally disrupt that.
     *
     * @param sigNo   the "signo" to attach a handler for
     * @param handler the function to invoke when the signal comes through
     */
    public static native void handle(LinuxSigNo sigNo, SignalHandler handler);
}
