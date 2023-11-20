package com.memsafe.demo;

import java.util.ArrayList;
import java.util.List;

public class ConsumeJavaMem {
    private static final List<Object> mList = new ArrayList<>();
    private static boolean mConsuming = false;

    public static void startConsuming() {
        mConsuming = true;
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (mConsuming) {
                    Object obj = new Object();
                    mList.add(obj);

                    try {
                        Thread.sleep(1);
                    } catch (InterruptedException e) {
                        throw new RuntimeException(e);
                    }
                }
            }
        }).start();
    }

    public static void stopConsuming() {
        mConsuming = false;
    }

    public static int getConsumCount() {
        return mList.size();
    }
}
