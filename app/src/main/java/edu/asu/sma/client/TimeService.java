package edu.asu.sma.client;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.SystemClock;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.util.Date;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public class TimeService extends Service {
  private final class ListenerThread extends Thread {
    private static final String TAG = "TimeService::Listener";
    private DatagramSocket socket = null;
    private volatile boolean listen = true;

    @Override
    public void run() {
      super.run();

      try {
        socket = new DatagramSocket(9997);
      } catch (SocketException e) {
        throw new RuntimeException("Couldn't create socket for time service", e);
      }

      Log.d(TAG, "Listening for time sync on port " + socket.getPort());

      while (listen) {
        byte[] buf = new byte[64];
        DatagramPacket packet = new DatagramPacket(buf, buf.length);

        try {
          socket.receive(packet);
        } catch (IOException e) {
          if (e.getMessage().endsWith("closed"))
            return;
          else
            Log.e(TAG, "Exception receiving datagram", e);
        }

        if (packet.getLength() != 8) {
          Log.e(TAG,
                "Received time datagram with invalid size (expected 8 bytes, received " +
                packet.getLength());
          continue;
        }

        DataInputStream dis = new DataInputStream(new ByteArrayInputStream(buf));

        long time_ms;
        try {
          time_ms = dis.readLong();
        } catch (IOException e) {
          Log.e(TAG, "Exception reading long time value from packet", e);
          continue;
        }

        if (time_ms > 0) {
          if (SystemClock.setCurrentTimeMillis(time_ms))
            Log.d(TAG, "Synchronized time: " + new Date(time_ms));
          else
            Log.e(TAG, "setCurrentTimeMillis returned false");
        } else {
          Log.e(TAG, "Received time value overflowed signed 64-bit long type");
        }
      }
    }

    public void shutdown() {
      listen = false;
      socket.close();
    }
  }

  ListenerThread thread = null;

  @Override
  public void onCreate() {
    super.onCreate();

    if (ShellInterface.isSuAvailable()) {
      ShellInterface.runCommand("chmod 666 /dev/alarm");

      thread = new ListenerThread();
      thread.start();
    } else {
      Log.e("TimeService", "Need su shell to synchronize the clock");
    }
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    if (thread != null)
      thread.shutdown();

    ShellInterface.runCommand("chmod 664 /dev/alarm");
  }

  @Override
  public IBinder onBind(Intent intent) {
    return null;
  }
}
