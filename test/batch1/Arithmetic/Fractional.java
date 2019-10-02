// Get the whole integral and fractional numbers of given number
import java.util.*;

public class Fractional {
  public static void main(String[] args) {
    double val1 = 88.88;
    double fractional = val1 % 1;
    double integral = val1 - fractional;

    System.out.print("\nValue : " + val1);
    System.out.print("\nIntegral value : " + integral);
    System.out.print("\nFractional value : " + fractional);
    System.out.println();
  }
}
