/*
  Use equals() to find whether string match with given string.  If so, returns true else false.
  Case sensitive
*/
public class SameStr {
  public static void main(String args[]) {
    String i = "Hello";
    String j = "helLO";
    String k = "HelLo";
    String l = "hello";
    String m = "Hello";

    System.out.println(i + " equals to " + j + "? " + i.equals(j));
    System.out.println(i + " equals to " + k + "? " + i.equals(k));
    System.out.println(i + " equals to " + l + "? " + i.equals(l));
    System.out.println(i + " equals to " + m + "? " + i.equals(m));
  }
}
