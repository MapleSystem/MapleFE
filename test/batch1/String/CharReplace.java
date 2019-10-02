/*
  Use replace() to replace a character by another character
*/
public class CharReplace {
  public static void main(String args[]) {
    String x = "Hello World!";

    String y = x.replace("H","B");
    System.out.println("Replacing H to B from " + x + " to " + y);
  }
}
