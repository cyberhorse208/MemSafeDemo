package com.memsafe.demo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

public class LeakResourceActivity extends AppCompatActivity {
    private BroadcastReceiver mReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_leakmem_resource_not_distroy);

        // 创建BroadcastReceiver
        mReceiver = new MyReceiver();

        // 创建IntentFilter
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_POWER_CONNECTED);
        filter.addAction(Intent.ACTION_POWER_DISCONNECTED);

        // 注册BroadcastReceiver
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        //故意不取消注册BroadcastReceiver，看能否检测出泄露
        //unregisterReceiver(mReceiver);
    }

    private class MyReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_POWER_CONNECTED)) {
                // 处理电源连接事件
            } else if (intent.getAction().equals(Intent.ACTION_POWER_DISCONNECTED)) {
                // 处理电源断开事件
            }
        }
    }

}
