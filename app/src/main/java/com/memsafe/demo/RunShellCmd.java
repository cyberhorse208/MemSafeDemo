package com.memsafe.demo;

import android.util.Log;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class RunShellCmd {
    public static String TAG = "RunShellCmd";
    public static String mStdout = null;
    public static String mStderr = null;
    public static String runCmd(String cmd)
    {
        try
        {
            //如果不是调用android自带命令，如pm之类，或者是在应用的lib目录(getApplicationInfo().nativeLibraryDir)下的executable，都会因为selinux原因不能执行
            //Runtime.getRuntime().exec("/system/bin/linker64 " + cmd);
            //执行命令

            Process process = Runtime.getRuntime().exec(cmd);
            BufferedReader stdout = new BufferedReader(new InputStreamReader(process.getInputStream()));
            BufferedReader stderr = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            String line = "";
            mStdout = "";
            String ret = "";
            while ((line = stdout.readLine()) != null) {
                mStdout += line+"\n";
                ret += line+"\n";
            }
            mStderr = null;
            while ((line = stderr.readLine()) != null) {
                mStderr += line+"\n";
                ret += line+"\n";
            }
            return ret;
        }
        catch (Exception e)
        {
            Log.e(TAG,e.toString());
            return e.toString();
        }

    }

}
