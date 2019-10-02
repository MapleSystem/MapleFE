// Check if string x1 same as the given string x2, x3 and x4.
//  Case sensitive.  Returns true or false.
public class StrEquals {
  public static void main(String[] args) {
    String x1 = "Hello World";
    String x2 = "You are the best";
    String x3 = "Huawei";
    String x4 = "Hello World";

    boolean same1 = x1.equals(x2);
    boolean same2 = x1.equals(x3);
    boolean same3 = x1.equals(x4);
   
    System.out.println("\"" + x1 + "\" equals \"" + x2 + "\"? " + same1);
    System.out.println("\"" + x1 + "\" equals \"" + x3 + "\"? " + same2);
    System.out.println("\"" + x1 + "\" equals \"" + x4 + "\"? " + same3);
  }
}
