public class Position {
  public static void main(String args[]) {
    String x = "Hello, How are you?";
    System.out.println(x);
   
    // charAt(0) = position 1.  charAt(1) = position 2.
    int pos1 = x.charAt(0);
    int pos5 = x.charAt(4);
    int pos10 = x.charAt(9);
    int pos15 = x.charAt(14);

    System.out.println("Character at position 1 is " + (char)pos1);
    System.out.println("Character at position 5 is " + (char)pos5);
    System.out.println("Character at position 10 is " + (char)pos10);
    System.out.println("Character at position 15 is " + (char)pos15);
  }
}
