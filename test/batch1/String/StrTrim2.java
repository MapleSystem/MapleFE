// Remove the whitespace from the front or the back of the given string
public class StrTrim2 {
  public static void main(String[] args) {
    String x = "  Failures are the stepping stone to success ";

    // Remove the whitespace from the front and back of a string
    String y = x.trim();

    System.out.println("Original : " + x);
    System.out.println("Trimmed : " + y);
  }
}
