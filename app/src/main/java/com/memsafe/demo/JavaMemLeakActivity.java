package com.memsafe.demo;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

public class JavaMemLeakActivity extends AppCompatActivity {

    private static final String TAG = "JavaMemLeakActivity";

    private Handler mHandler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
        }
    };

    class MyRunnable implements Runnable {
        @Override
        public void run() {
            Log.d(TAG, "run: test");
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_leakmem_activity_not_distroy);

        mHandler.postDelayed(new MyRunnable(), 100000);
        //finish();
        Log.d(TAG, "onCreate: leak");
    }

}
