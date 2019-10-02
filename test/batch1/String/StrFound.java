/*
  Use contains() to find whether string contains the given suffix.  If so, returns true else false.
  Case sensitive
*/
public class StrFound {
  public static void main(String args[]) {
    String x = "You are welcome";
    System.out.println(x + " contains you? " + x.contains("you"));   //  false.  case sensitive
    System.out.println(x + " conatins You are? " + x.contains("You are"));
    System.out.println(x + " contains Welcome? " + x.contains("Welcome"));   // false.  case sensitive
    System.out.println(x + " contains welcome? " + x.contains("welcome"));
  }
}
