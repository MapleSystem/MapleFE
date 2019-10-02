// Check and see the given numbers are DOUBLE number
import java.util.*;

public class Doublenum {
  public static void main(String[] args) {
    double val1 = 8.888;
    double val2 = 3;

    System.out.println("Value : " + val1);
    if ((val1 % 1) == 0) {
      System.out.println("It is not a double number!");
    } else {
      System.out.println("It is a double number!");
    }

    System.out.println("Value : " + val2);
    if ((val2 % 1) == 0) {
      System.out.println("It is not a double number!");
    } else {
      System.out.println("It is a double number!");
    }
  }
}
