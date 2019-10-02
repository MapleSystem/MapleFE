/*
  Use replace() to replace a word into the replacing word
*/
public class StrReplace {
  public static void main(String args[]) {
    String x = "Hello World!";

    String y = x.replace("World","There");
    System.out.println("Replacing World to There from " + x + " to " + y);
  }
}
