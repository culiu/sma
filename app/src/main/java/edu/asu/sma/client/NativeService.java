package edu.asu.sma.client;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Process;
import android.util.Log;

import java.util.Random;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import edu.asu.sma.NodeContainer;
import edu.asu.sma.Sma;

public class NativeService extends Service {
  private final class BinderWrapper extends Binder implements NativeHandlerBinder {
    @Override
    public Handler nativeHandler() {
      return handler;
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Native Interface
  static {
    System.loadLibrary(Sma.LIBRARY_NAME);
  }

  // Give the native code a pointer to this object
  private native void captureServicePointer();

  // Invalidate the native code's pointer to this object
  private native void deleteServicePointer();

  // Run the next queued native task in the service thread
  private native void runNativeAsyncTask();

  private native void startLinkThread();

  private native void stopLinkThread();

  // Handle the next queued packet in the service thread
  private native void handlePacket();

  // Called by native code (in any thread) to schedule an async task on the service thread
  public void scheduleNativeAsync(long delay_nanos) {
    asyncExecutor.schedule(nativeTaskRunner, delay_nanos, TimeUnit.NANOSECONDS);
  }

  // Called by native link-reading thread to schedule a packet on the service thread
  public void packetAvailable() {
    packetHandler.run();
  }
  // Native Interface
  //////////////////////////////////////////////////////////////////////////////////////////////////

  public static int nodeId;

  private static final String TAG = NativeService.class.getSimpleName();

  private Looper looper;
  private Handler handler;

  // Manages the delayed execution of native async tasks
  private ScheduledExecutorService asyncExecutor = Executors.newScheduledThreadPool(1);
  // Runs the next native task (if any) on the service thread
  private Runnable nativeTaskRunner = new Runnable() {
    @Override
    public void run() {
      if (Thread.currentThread() == looper.getThread())
        runNativeAsyncTask();
      else
        handler.post(this);
    }
  };

  // Handles the next packet on the service thread (in native code)
  private Runnable packetHandler = new Runnable() {
    @Override
    public void run() {
      if (Thread.currentThread() == looper.getThread())
        handlePacket();
      else {
        handler.post(this);
      }
    }
  };


  public NativeService() {
    captureServicePointer();
  }

  @Override
  public synchronized void onCreate() {
    super.onCreate();
    HandlerThread thread = new HandlerThread("NativeService_Background",
                                             Process.THREAD_PRIORITY_BACKGROUND);
    thread.start();
    looper = thread.getLooper();
    handler = new Handler(looper);

    WifiManager wifi = (WifiManager) getSystemService(Context.WIFI_SERVICE);
    WifiInfo info = wifi.getConnectionInfo();
    String macAddr = info.getMacAddress();
    if (macAddr != null)
      nodeId = macAddr.hashCode();
    else
      nodeId = (new Random()).nextInt();

    StatLogger.log("Created node");

    if (NodeContainer.create(nodeId)) {
      Log.d(TAG, "Created native node on background thread");
      startLinkThread();
    } else {
      Log.d(TAG, "Native node already running");
    }

  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    return START_STICKY;
  }

  @Override
  public synchronized void onDestroy() {
    super.onDestroy();

    stopLinkThread();

    deleteServicePointer();

    looper.quit();
    looper = null;

    Log.d(TAG, "Destroying native node");
    NodeContainer.dispose();
  }

  @Override
  public IBinder onBind(Intent intent) {
    return new BinderWrapper();
  }
}
