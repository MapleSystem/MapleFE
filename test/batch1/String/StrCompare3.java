/*
  Compare two strings dictionary order regardless case
*/
public class StrCompare3 {
  public static void main(String[] args) {
    String x = "Huawei 1";
    String y = "huawei 1";

    System.out.println(x);
    System.out.println(y);

    int cmp = x.compareToIgnoreCase(y);

    if (cmp < 0) {
      System.out.println("\"" + x + "\"" + " is less than " + "\"" + y + "\"");
    } else if (cmp == 0) {
      System.out.println("\"" + x + "\"" + " is equal to " + "\"" + y + "\"");
    } else {
      System.out.println("\"" + x + "\"" + " is greater than " + "\"" + y + "\"");
    }
  }
}
