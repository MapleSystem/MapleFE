public class StrUnicode {
  public static void main(String args[]) {
    String x = "Hello, How are you?";

    // charAt(0) = position 1.  charAt(1) = position 2.
    int pos1 = x.charAt(0);
    int pos5 = x.charAt(4);
    int pos10 = x.charAt(9);
    int pos15 = x.charAt(14);

    System.out.println("Unicode at position 1 is " + pos1);
    System.out.println("Unicode at position 5 is " + pos5);
    System.out.println("Unicode at position 10 is " + pos10);
    System.out.println("Unicode at position 15 is " + pos15);
  }
}
