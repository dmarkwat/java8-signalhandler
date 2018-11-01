package com.markwat.system.signals;

import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.lang.management.ManagementFactory;

class TestLinuxSignalHandler {

    @Test
    void myTest() throws IOException, InterruptedException {
        System.out.println("Come here, you!");

        SignalHandler h = new SignalHandler() {
            @Override
            public void handle(LinuxSigInfo info) {
                System.out.println("I got ya now, Sonny Jim!");
            }
        };
        LinuxSignalHandler.handle(LinuxSigNo.SIGHUP, h);

        String vmName = ManagementFactory.getRuntimeMXBean().getName();
        int p = vmName.indexOf("@");
        String pid = vmName.substring(0, p);

        Process proc = Runtime.getRuntime().exec(new String[]{"kill", "-SIGHUP", pid});
        proc.waitFor();
    }
}
