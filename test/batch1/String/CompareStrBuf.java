public class CompareStrBuf {
  public static void main(String[] args) {
    String x, y, z; 
    x = "Huawei";
    y = "huawei";
    z = "HuaWei";
    StringBuffer cmp = new StringBuffer(x);

    System.out.println("Compare \"" + x + "\" and \"" + cmp + "\": " + x.contentEquals(cmp));
    System.out.println("Compare \"" + y + "\" and \"" + cmp + "\": " + y.contentEquals(cmp));
    System.out.println("Compare \"" + z + "\" and \"" + cmp + "\": " + z.contentEquals(cmp));
  }
}
