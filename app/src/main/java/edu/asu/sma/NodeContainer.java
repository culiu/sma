package edu.asu.sma;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public class NodeContainer {
  public static native boolean create(int id);

  public static native void dispose();

  static {
    System.loadLibrary(Sma.LIBRARY_NAME);
  }
}
