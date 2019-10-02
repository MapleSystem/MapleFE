// Read input until a g is received.
class Break2 {
  public static void main(String args[])
    throws java.io.IOException {
    char ke, ignore;
     
    for( ; ; ) {
      System.out.println("Please input a letter: ");
      ke = (char) System.in.read(); //get a char from keyboard
        do {
           ignore = (char) System.in.read();
        }
        while (ignore != '\n'); 
      if( ke == 'g') break;
      System.out.println("The program will stop when you input the right letter!");
    }
    System.out.println("You pressed g! The program stops");
  }
}

