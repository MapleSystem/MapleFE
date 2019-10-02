// Check given string end with another given string.
// Case sensitive.  Returns true or false.
public class EndsWith {
  public static void main(String[] args) {
    String x = "Huawei";
    String y = "Huawei 2";
    String z = "HuaWei";

    // Check x, y, z ends with the following given string.  case sensitive.
    String estr = "wei";

    boolean str1 = x.endsWith(estr);
    boolean str2 = y.endsWith(estr);
    boolean str3 = z.endsWith(estr);

    System.out.println("\"" + x + "\" ends with " + "\"" + estr + "\"? " + str1);
    System.out.println("\"" + y + "\" ends with " + "\"" + estr + "\"? " + str2);
    System.out.println("\"" + z + "\" ends with " + "\"" + estr + "\"? " + str3);
  }
}
