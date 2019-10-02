/*
  Compare two strings dictionary order
*/
public class StrCompare2 {
  public static void main(String[] args) {
    String x = "Huawei 1";
    String y = "Huawei 2";

    System.out.println(x);
    System.out.println(y);

    int cmp = x.compareTo(y);

    if (cmp < 0) {
      System.out.println("\"" + x + "\"" + " is less than " + "\"" + y + "\"");
    } else if (cmp == 0) {
      System.out.println("\"" + x + "\"" + " is equal to " + "\"" + y + "\"");
    } else {
      System.out.println("\"" + x + "\"" + " is greater than " + "\"" + y + "\"");
    }
  }
}
