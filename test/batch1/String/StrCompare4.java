/*
  Compare a given string to specified character seq
*/
public class StrCompare4 {
  public static void main(String[] args) {
    String x, y, z;
    x = "Huawei";
    y = "huawei";
    z = "HuaWei";
    CharSequence cmp = "Huawei";

    System.out.println("String 1 : " + x);
    System.out.println("String 2 : " + y);
    System.out.println("String 3 : " + z);
    System.out.println("Character SEQ to be compare : " + cmp);
    System.out.println("Compare \"" + x + "\" and \"" + cmp + "\" : " + x.contentEquals(cmp));
    System.out.println("Compare \"" + y + "\" and \"" + cmp + "\" : " + y.contentEquals(cmp));
    System.out.println("Compare \"" + z + "\" and \"" + cmp + "\" : " + z.contentEquals(cmp));
  }
}
