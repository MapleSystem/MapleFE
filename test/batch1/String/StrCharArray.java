//  Get the contents of a given string as a character array
public class StrCharArray {
  public static void main(String[] args) {
    String x = "Hello World.";

    // Get the contents of character array of a given string
    // Copy the character from 4 to 8 of given string x
    // Fill the array starting at position 3.
    char[] char_array = new char[] { ' ', ' ', ' ', ' ',
                                     ' ', ' ', ' ', ' ' };
    x.getChars(4, 8, char_array, 3);

    System.out.println("The character array is \"" + char_array + "\"");
  }
}
