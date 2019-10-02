/*
  Compare two strings.
  If i == second string, then returns 0
  If i > second string, then returns +num
  If i < second string, then returns -num
*/
public class StrCompare {
  public static void main(String args[]) {
    String i, j, k, l, m, n, o;
    i = "Compare";
    j = "no";
    k = "Compare";
    l = "HI";
    m = "Conpare";
    n = "Comparee";
    o = "Compareee";

    System.out.println(i.compareTo(j));
    System.out.println(i.compareTo(k));
    System.out.println(i.compareTo(l));
    System.out.println(i.compareTo(m));
    System.out.println(i.compareTo(n));
    System.out.println(i.compareTo(o));
  }
}
