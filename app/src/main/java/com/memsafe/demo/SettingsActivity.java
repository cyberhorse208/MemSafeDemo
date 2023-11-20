package com.memsafe.demo;

import android.app.AlertDialog;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

public class SettingsActivity extends AppCompatActivity {

    private Button mBtStart;
    private Button mBtClose;
    private Button mBtRefresh;
    private Switch mSwAppMTE;
    private Switch mSwNativeMTE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        initView();
        initData();
        initListener();
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void initView() {
        ActionBar actionBar = getSupportActionBar();
        actionBar.setTitle("");
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(R.drawable.back);

        mBtStart = findViewById(R.id.bt_start);
        mBtClose = findViewById(R.id.bt_close);
        mBtRefresh = findViewById(R.id.bt_refresh);
        mSwAppMTE = findViewById(R.id.sw_app_mte);
        mSwNativeMTE = findViewById(R.id.sw_native_mte);
    }

    private String appDefaultProp = "persist.arm64.memtag.app_default";
    private String nativeDefaultProp = "persist.arm64.memtag.default";

    private void initData() {
        if ("sync".equals(AccessProperties.get(appDefaultProp))) {
            mSwAppMTE.setChecked(true);
        } else {
            mSwAppMTE.setChecked(false);
        }
        if ("sync".equals(AccessProperties.get(nativeDefaultProp))) {
            mSwNativeMTE.setChecked(true);
        } else {
            mSwNativeMTE.setChecked(false);
        }
    }

    private void unInitListener() {

        mSwAppMTE.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

            }
        });

        mSwNativeMTE.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            }
        });

    }
    private void initListener() {
        mBtStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setAll(true);
            }
        });

        mBtClose.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setAll(false);
            }
        });

        mBtRefresh.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                unInitListener();//make sure we don't set prop after refresh the buttons
                initData();
                initListener();//after refresh the buttons, recover the listeners so we can actually set prop
            }
        });

        mSwAppMTE.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                checkSetData(mSwAppMTE, isChecked, appDefaultProp, "sync", "off");
            }
        });

        mSwNativeMTE.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                checkSetData(mSwNativeMTE, isChecked, nativeDefaultProp, "sync", "off");
            }
        });

    }

    private void checkSetData(Switch button, boolean isChecked, String name, String data1, String data2) {
        if (isChecked) {
            if (!AccessProperties.set(name, data1)) {
                button.setChecked(!isChecked);
                showAlert("设置失败，请关闭SELinux或者用shell执行命令:\nsetprop " + name + " " + data1);
                return;
            }
        } else {
            if (!AccessProperties.set(name, data2)) {
                button.setChecked(!isChecked);
                showAlert( "设置失败，请关闭SELinux或者用shell执行命令:\nsetprop " + name + " " + data2);
                return;
            }
        }
        button.setChecked(isChecked);
    }

    private int currentDetectionIndex;
    private boolean state;
    private void setAll(boolean state) {
        currentDetectionIndex = -1;
        this.state = state;
        Handler.postDelayed(OpenAllSwithRunnable, 500);
    }

    private void showAlert(String content){
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(content)
                .setTitle("提示");
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private Handler Handler = new Handler(Looper.getMainLooper());
    private Runnable OpenAllSwithRunnable = new Runnable() {
        @Override
        public void run() {

            currentDetectionIndex++;
            switch (currentDetectionIndex) {
                case 0:
                    checkSetData(mSwAppMTE, state, appDefaultProp, state?"sync":"off", (!state)?"sync":"off");
                    break;
                case 1:
                    checkSetData(mSwNativeMTE, state, nativeDefaultProp, state?"sync":"off", (!state)?"sync":"off");
                    break;
                default:
                    // 已完成所有检测项
                    Handler.removeCallbacks(this);
                    break;
            }
            if (currentDetectionIndex < 5) {
                Handler.postDelayed(this, 500);
            }
        }
    };


}