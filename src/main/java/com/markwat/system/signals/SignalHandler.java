package com.markwat.system.signals;

public interface SignalHandler {

    /**
     * Handler interface for signals.
     *
     * Avoid throwing exceptions in here--they are ignored by the implementation.
     *
     * @param info a subset of the "siginfo_t" C type
     */
    void handle(LinuxSigInfo info);
}
