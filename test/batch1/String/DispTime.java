// Display current system date and time

import java.util.Calendar;

public class DispTime {
  public static void main(String[] args) {
    Calendar x = Calendar.getInstance();

    System.out.println("Current Date and Time : ");
    System.out.format("%tB %te, %tY ", x, x, x);
    System.out.format("%tl:%tM %tp%n", x, x, x);
  }
}
