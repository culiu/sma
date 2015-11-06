package edu.asu.sma.client;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;


public class NativeComponent {
  protected NavActivity activity;
  private NativeConsumer consumer = null;
  protected Handler nativeHandler = null;

  private ServiceConnection svc_conn = new ServiceConnection() {
    @Override
    public void onServiceConnected(ComponentName componentName, IBinder binder) {
      nativeHandler = ((NativeHandlerBinder) binder).nativeHandler();
      if (consumer != null)
        consumer.onNativeHandler(nativeHandler);
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName) {
      nativeHandler = null;
    }
  };

  public NativeComponent(NativeConsumer consumer, NavActivity activity) {
    this.consumer = consumer;
    this.activity = activity;
    activity.bindService(new Intent(activity, NativeService.class),
                         svc_conn,
                         Context.BIND_AUTO_CREATE | Context.BIND_NOT_FOREGROUND);
  }

  public void dispose() {
    if (nativeHandler != null) {
      activity.unbindService(svc_conn);
      nativeHandler = null;
    }
  }
}
