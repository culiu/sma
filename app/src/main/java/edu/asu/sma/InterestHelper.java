package edu.asu.sma;

import java.util.List;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public final class InterestHelper {
  private InterestHelper() {
  }

  public static native boolean create(String interest);

  public static native List<String> local();

  public static native List<String> remote();

  public static native void delete(String text);

//  static {
//    System.loadLibrary(Sma.LIBRARY_NAME);
//  }
}
