/*
  Use equalsIgnoreCase() to find whether string match with given string regardless to case sensitivity.
  If so, returns true else false.
*/
public class SameStrIgnCase {
  public static void main(String args[]) {
    String i = "Hello";
    String j = "Bello";
    String k = "helLO";
    String l = "hello";

    System.out.println(i.equalsIgnoreCase(j));
    System.out.println(i.equalsIgnoreCase(k));
    System.out.println(i.equalsIgnoreCase(l));
  }
}
