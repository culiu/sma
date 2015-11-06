package edu.asu.sma;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public final class Sma {
  public static final String LIBRARY_NAME = "smanative";

  static {
    System.loadLibrary(Sma.LIBRARY_NAME);
  }
}
