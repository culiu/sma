package edu.asu.sma;

import java.util.List;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public final class NeighborHelper {
  public static native List<String> all();

  static {
    System.loadLibrary(Sma.LIBRARY_NAME);
  }
}
