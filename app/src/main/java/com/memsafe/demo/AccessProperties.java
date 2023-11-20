package com.memsafe.demo;

import android.util.Log;

import java.lang.reflect.Method;

public class AccessProperties {
    public static String TAG = "AccessProperties";
    //need close SELinux to run
    public static String get(String name){
        try {
            Class<?> c = Class.forName("android.os.SystemProperties");
            Method get = c.getMethod("get", String.class);
            return  (String) get.invoke(c, name);
        } catch (Exception e) {
            Log.e(TAG,e.toString());
            return "";
        }
    }

    public static boolean set(String name, String value){
        try {
            Class<?> c = Class.forName("android.os.SystemProperties");
            Method set = c.getMethod("set", String.class, String.class);
            set.invoke(c, name, value);

            return true;
        } catch (Exception e) {
            Log.e(TAG,e.toString());
            return false;
        }
    }
}
