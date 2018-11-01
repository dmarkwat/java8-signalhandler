package com.markwat.system.signals;

/**
 * A representative subset of the siginfo_t type
 */
public class LinuxSigInfo {

    private final LinuxSigCode code;
    private final LinuxSigNo signo;

    public LinuxSigInfo(LinuxSigCode code, LinuxSigNo signo) {
        this.code = code;
        this.signo = signo;
    }

    public LinuxSigCode getCode() {
        return code;
    }

    public LinuxSigNo getSigno() {
        return signo;
    }
}
