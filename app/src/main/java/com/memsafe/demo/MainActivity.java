package com.memsafe.demo;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.ApplicationExitInfo;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.os.IBinder;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.memsafe.demo.databinding.ActivityMainBinding;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

public class MainActivity extends AppCompatActivity {
    private final static String TAG = "nativetest";
    CopyELF ce;
    // Used to load the 'nativetest' library on application startup.
    static {
        System.loadLibrary("nativetest");
    }


    private void showAlert(String title, String content){
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(content)
                .setTitle(title);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    //not work even we are root and get system signature
    private void showLatestTombstone(){
//        String content = RunShellCmd.runCmd("cat /data/tombstones/$(ls -t /data/tombstones/ | head -1)");
   //     if(!content.isEmpty())
    //        showAlert("最新tombstone", content);
    }

    //when use failover mode, we can't get exit info because we don't crash
    private void showLatestExitInfo() {
        ActivityManager am = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        if (am == null) {
            return;
        }
        List<ApplicationExitInfo> reasonsList = am.getHistoricalProcessExitReasons(null, 0, 1);
        for (ApplicationExitInfo applicationExitInfo : reasonsList) {
            if (applicationExitInfo.getReason() == ApplicationExitInfo.REASON_CRASH_NATIVE) {
                try {
                    InputStream exitReasonsInputStream = applicationExitInfo.getTraceInputStream();
                    if (exitReasonsInputStream != null) {
                        String result = new BufferedReader(new InputStreamReader(exitReasonsInputStream))
                                .lines().collect(Collectors.joining(System.lineSeparator()));
                        showAlert("last_crash_native", result);
                    }
                } catch (Exception e) {
                    Log.e("showLatestExitInfo", e.toString());
                }
            }
        }

    }
    public void runCmdAndShowResult(String cmd, boolean noPrefix){
        TextView text_output = (TextView) findViewById(R.id.text_output);
        if(!noPrefix) {
            cmd = ce.getExecutableFilePath() + cmd;
        }
        String result = RunShellCmd.runCmd(cmd);
        text_output.setText("执行命令:" + cmd + "\n执行结果：\n" + result);
    }

    private ActivityMainBinding binding;

    int mJavaMemLeakCount = 0;
    int mJavaMemLeakResourceCount = 0;
    int mNativeMemLeakCount = 0;
    private String mlibPath = "";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        ce = new CopyELF(getBaseContext());
        ce.copyAll2Data();

        ActionBar actionBar = getSupportActionBar();
        actionBar.setTitle("");
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(R.drawable.back);

        Button btn_getmtelevel = (Button) findViewById(R.id.btn_getmtelevel);
        btn_getmtelevel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TextView mtelevel_output = (TextView) findViewById(R.id.mtelevel_output);
                mtelevel_output.setText(getMteLevel());
            }
        });

        ApplicationInfo appInfo = getApplicationInfo();
        mlibPath = appInfo.nativeLibraryDir;

        TextView text_output = (TextView) findViewById(R.id.text_output);
        text_output.setMovementMethod(ScrollingMovementMethod.getInstance());
        Button btn_nativemte = (Button) findViewById(R.id.btn_nativemte);
        btn_nativemte.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                runCmdAndShowResult(mlibPath + "/hello-mte.so 5", true);
            }
        });



        Button btn_localmte2 = (Button) findViewById(R.id.btn_localmte2);
        btn_localmte2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                text_output.setText(stringFromJNIOOB());

                showLatestTombstone();
            }
        });

        Button btn_localmte3 = (Button) findViewById(R.id.btn_localmte3);
        btn_localmte3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                text_output.setText(stringFromJNIUAF());

                showLatestTombstone();
            }
        });


        Button btn_localmte4 = (Button) findViewById(R.id.btn_localmte4);
        btn_localmte4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                text_output.setText(stringFromJNIUAF2());

                showLatestTombstone();
            }
        });

        Button btn_fdsan = (Button) findViewById(R.id.btn_fdsan);
        btn_fdsan.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                runCmdAndShowResult(mlibPath + "/fdsan-demo.so", true);
            }
        });


        Button btn_ubsan_app = (Button) findViewById(R.id.btn_ubsan_app);
        btn_ubsan_app.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                text_output.setText(ubsanDemo());
            }
        });
        Button btn_ubsan_native = (Button) findViewById(R.id.btn_ubsan_native);
        btn_ubsan_native.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                runCmdAndShowResult(mlibPath + "/ubsan-demo.so 2", true);
            }
        });


        Button btn_selfhandle = (Button) findViewById(R.id.btn_selfhandle);
        btn_selfhandle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String name = "debug.mte.sigsegv.no.userhandler";
                String value = AccessProperties.get(name);
                text_output.setText( name + " " + value + "\n" + handleSignalMyself());
            }
        });

        Button btn_nativememleak = (Button) findViewById(R.id.btn_nativememleak);
        btn_nativememleak.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mNativeMemLeakCount += ConsumeNativeMem.leak();
                text_output.setText("native内存泄露字节：" + String.valueOf(mNativeMemLeakCount));
            }
        });

        Button btn_javamemleak_activity_not_distroy = (Button) findViewById(R.id.btn_javamemleak_activity_not_distroy);
        btn_javamemleak_activity_not_distroy.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(MainActivity.this,JavaMemLeakActivity.class));
                mJavaMemLeakCount++;
                text_output.setText("java内存泄露(activity不被释放)次数: " + String.valueOf(mJavaMemLeakCount));
            }
        });

        Button btn_javamemleak_resouce_not_distroy = (Button) findViewById(R.id.btn_javamemleak_resouce_not_distroy);
        btn_javamemleak_resouce_not_distroy.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(MainActivity.this,LeakResourceActivity.class));
                mJavaMemLeakResourceCount++;
                text_output.setText("java内存泄露（资源不被释放）次数: " + String.valueOf(mJavaMemLeakResourceCount));
            }
        });

        Button btn_consume_java_mem_start = (Button) findViewById(R.id.btn_consume_java_mem_start);
        btn_consume_java_mem_start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ConsumeJavaMem.startConsuming();
            }
        });

        Button btn_consume_java_mem_stop = (Button) findViewById(R.id.btn_consume_java_mem_stop);
        btn_consume_java_mem_stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ConsumeJavaMem.stopConsuming();
                text_output.setText("累积java内存消耗次数: " + String.valueOf(ConsumeJavaMem.getConsumCount()));
            }
        });

        ScheduledExecutorService scheduledExecutorService =  Executors.newScheduledThreadPool(1);
        scheduledExecutorService.scheduleAtFixedRate(new Runnable() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        TextView text_output = (TextView) findViewById(R.id.mtelevel_output);
                        text_output.setText(getMteLevel());
                    }
                });

            }
        },0,100, TimeUnit.MILLISECONDS);


        Button btn_servicecmd = (Button) findViewById(R.id.btn_servicecmd);
        btn_servicecmd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText edit_input = (EditText) findViewById(R.id.edit_input);
                String cmd = edit_input.getText().toString();
                runCmdAndShowResult(cmd, true);
            }
        });
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // 通过解析菜单文件来创建 menu 接口实例
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    //返回按钮点击触发
    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        if (item.getItemId() == R.id.menu_refresh) {
            startActivity(new Intent(MainActivity.this,SettingsActivity.class));
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    //need system signature to run cmd
    //cmd may be denied by SELinux
    public static String execRunShellWithResult(String cmd) {
        String result = "";
        try {
            Class<?> ServiceManager = Class.forName("android.os.ServiceManager");
            Object service = ServiceManager.getMethod("getService", new Class[]{String.class}).invoke(ServiceManager, new Object[]{"vivo_daemon.service"});
            Class<?> VivoDmServiceProxy = Class.forName("com.vivo.services.daemon.VivoDmServiceProxy");
            Object dmService = VivoDmServiceProxy.getMethod("asInterface", new Class[]{IBinder.class}).invoke(VivoDmServiceProxy, new Object[]{service});
            return (String) VivoDmServiceProxy.getMethod("runShellWithResult", new Class[]{String.class}).invoke(dmService, new Object[]{cmd});
        } catch (Exception e) {
            Log.e(TAG, "Get vivo_daemon.service failed", e);
            return result;
        }
    }

    /**
     * A native method that is implemented by the 'nativetest' native library,
     * which is packaged with this application.
     */
    public native String getMteLevel();
    public native String stringFromJNIOOB();

    public native String stringFromJNIUAF();
    public native String stringFromJNIUAF2();

    public native String handleSignalMyself();

    public native String ubsanDemo();
}