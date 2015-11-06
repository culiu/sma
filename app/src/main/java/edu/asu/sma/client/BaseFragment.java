package edu.asu.sma.client;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.widget.Toast;

public abstract class BaseFragment extends Fragment {
  protected NavActivity activity;
  private NativeConsumer consumer = null;
  protected Handler nativeHandler = null;

  private ServiceConnection svc_conn = new ServiceConnection() {
    @Override
    public void onServiceConnected(ComponentName componentName, IBinder binder) {
      Log.i("basefragment", "oncreate");
      nativeHandler = ((NativeHandlerBinder) binder).nativeHandler();
      if (consumer != null)
        consumer.onNativeHandler(nativeHandler);
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName) {
      nativeHandler = null;
    }
  };

  protected void bind(NativeConsumer consumer) {
    this.consumer = consumer;
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    activity = (NavActivity) getActivity();
    activity.bindService(new Intent(activity, NativeService.class),
                         svc_conn,
                         Context.BIND_AUTO_CREATE | Context.BIND_NOT_FOREGROUND);
  }

  @Override
  public void onStop() {
    Log.i("basefragment", "onstop");
    super.onStop();
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    Log.i("basefragment", "ondestroy");
    if (nativeHandler != null) {
      activity.unbindService(svc_conn);
      nativeHandler = null;
    }
  }

  public void popToast (String toast_line) {
    Toast.makeText(activity, toast_line, Toast.LENGTH_SHORT);
  }
}
