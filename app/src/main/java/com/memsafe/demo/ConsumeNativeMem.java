package com.memsafe.demo;

public class ConsumeNativeMem {

    public static int leak(){
        return leakSomeNativeMem();
    }
    public static native int leakSomeNativeMem();
}
