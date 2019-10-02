// Replace word frog in string x to the given word log
public class StrReplace3 {
  public static void main(String[] args) {
    String x = "A dog sits on a frog";
  
    String y = x.replaceAll("frog", "log");

    System.out.println("Original string: " + x);
    System.out.println("New string: " + y);
  }
}
