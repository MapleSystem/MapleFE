// Get the canonical representation of the given Strings
public class StrCanonic {
  public static void main(String[] args) {
    String x = "Hello World";
    String y = new StringBuffer("Hello").append(" World").toString();
    String z = y.intern();

    System.out.println("x = \"" + "Hello World" + "\"");
    System.out.println("y = \"" + "new StringBuffer(\"Hello\").append(\" World\").toString()" + "\"");
    System.out.println("z = \"" + "y.intern()" + "\"");
    System.out.println("x == y? " + (x == y));
    System.out.println("x == z? " + (x == z));
  }
}
