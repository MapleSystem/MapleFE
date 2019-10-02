// Replace all the characters in x which matches the given character
public class StrReplace2 {
  public static void main(String[] args) {
    String x = "Failures are stepping stones to success.";

    String y = x.replace('.','!');

    System.out.println("Original string: " + x);
    System.out.println("New string: " + y);
  }
}
