package edu.asu.sma;

import java.util.List;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public final class ContentHelper {
  private ContentHelper() {
  }

  public static native void create(String[] types, String name, byte[] data, long size);

  public static native List<String> local();

  public static native List<String> remote();

  public static native void fetch(String content_name);

//  static {
//    System.loadLibrary(Sma.LIBRARY_NAME);
//  }
}
