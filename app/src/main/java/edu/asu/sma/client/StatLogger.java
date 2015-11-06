package edu.asu.sma.client;

import android.util.Log;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;

public final class StatLogger {
  private static final String TAG = StatLogger.class.getSimpleName();

  private static List<String> data = new ArrayList<>();
  private static final Object dataMutex = new Object();

  private static volatile UploadThread thread = null;
  private static final Object threadMutex = new Object();

  public static void log(String message) {
    message = System.currentTimeMillis() + "\t" + NativeService.nodeId + "\t" + message + "\n";
    synchronized (dataMutex) {
      data.add(message);
    }
  }

  public static void upload(String host, int port) {
    synchronized (threadMutex) {
      if (thread == null) {
        thread = new UploadThread(host, port);
        thread.start();
      }
    }
  }

  private static final class UploadThread extends Thread {
    InetAddress hostAddr;
    int port;

    public UploadThread(String host, int port) {
      try {
        hostAddr = InetAddress.getByName(host);
      } catch (UnknownHostException e) {
        throw new RuntimeException(e);
      }
      this.port = port;
    }

    @Override
    public void run() {
      int attempts = 0;
      Socket socket = null;
      while (!data.isEmpty()) {
        try {
          socket = new Socket(hostAddr, port);
          uploadTo(socket);
        } catch (IOException e) {
          if (++attempts < 3)
            Log.e(TAG, "Exception uploading logs to server", e);
          else
            throw new RuntimeException("Too many failed attempts to upload logs", e);
        }
      }

      if (socket != null)
        try {
          socket.close();
        } catch (IOException ignored) {
        }

      thread = null;
    }
  }

  private static void uploadTo(Socket server) throws IOException {
    synchronized (dataMutex) {
      PrintWriter out = new PrintWriter(server.getOutputStream(), true);
      while (!data.isEmpty()) {
        String line = data.get(0);
        out.print(line);
        out.flush();
        data.remove(0);
      }
    }
  }
}
