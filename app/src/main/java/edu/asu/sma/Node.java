package edu.asu.sma;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public final class Node {
  public static native String id();

  public static native void tick();

//  static {
//    System.loadLibrary(Sma.LIBRARY_NAME);
//  }
}
