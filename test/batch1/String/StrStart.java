// Check whether String x and y starts with the given word Bread
// Returns true or false
public class StrStart {
  public static void main(String[] args) {
    String x = "Bread is my favorite food.";
    String y = "Ice cream is also my favorite food.";

    String sStr = "Bread";

    boolean str_start1 = x.startsWith(sStr);
    boolean str_start2 = y.startsWith(sStr);

    System.out.println("\"" + x + "\"" + " starts with " + sStr + "? " + str_start1);
    System.out.println("\"" + y + "\"" + " starts with " + sStr + "? " + str_start2);
  }
}
