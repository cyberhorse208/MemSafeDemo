package com.memsafe.demo;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;

public class CopyELF {
    String TAG="CopyELF";
    Context ct;
    String appFileDirectory,executableFilePath;
    AssetManager assetManager;
    List resList;
    String cpuType;
    String[] assetsFiles={
            //todo 要拷贝的文件列表放这里
    };

    CopyELF(Context c){
        ct=c;
        appFileDirectory = ct.getFilesDir().getPath();
        executableFilePath = appFileDirectory + "/executable";

        assetManager = ct.getAssets();
        try {
            resList = Arrays.asList(ct.getAssets().list("/"));
            Log.d(TAG,"get assets list:"+resList.toString());
        } catch (IOException e){
            Log.e(TAG, "Error list assets folder:", e);
        }
    }
    boolean resFileExist(String filename){
        File f=new File(executableFilePath+"/"+filename);
        if (f.exists()) {
            Log.i(TAG, executableFilePath+"/"+filename + " exist!");
            return true;
        }
        Log.i(TAG, executableFilePath+"/"+filename + " not exist!");
        return false;
    }
    void copyFile(InputStream in, OutputStream out){
        try {
            byte[] buf = new byte[1024];
            int len;
            while ((len = in.read(buf)) > 0) {
                out.write(buf, 0, len);
            }
        } catch (IOException e){
            Log.e(TAG, "Failed to read/write asset file: ", e);
        }
    };
    private void copyAssets(String filename) {
        InputStream in = null;
        OutputStream out = null;
        Log.d(TAG, "Attempting to copy this file: " + filename);

        try {
            in = assetManager.open(filename);
            File outFile = new File(executableFilePath, filename);
            out = new FileOutputStream(outFile);
            copyFile(in, out);
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
            Log.d(TAG, "Copy success: " + filename);
        } catch(IOException e) {
            Log.e(TAG, "Failed to copy asset file: " + filename, e);
        }

    }
    void copyAll2Data(){
        int i;

        File folder=new File(executableFilePath);
        if (!folder.exists()){
            folder.mkdir();
        }

        for(i=0;i<assetsFiles.length;i++){
            //if (!resFileExist(assetsFiles[i]))
            {
                copyAssets(assetsFiles[i]);
                File execFile = new File(executableFilePath+"/"+assetsFiles[i]);
                execFile.setExecutable(true);
            }
        }
    }

    String getExecutableFilePath(){
        return executableFilePath;
    }
}