//Guess the letter game, with do-while loop.
class Guess4 {
  public static void main(String args[]) 
    throws java.io.IOException {
    char ch, ignore, answer = 'h';
    do {
      System.out.println("I'm thinking of a letter between A and Z.");
      System.out.print("Can you guess it: ");
   
      //read a character
      ch = (char) System.in.read();
      //discard any other characters in the input buffer

      do {
        ignore = (char) System.in.read();
       // System.out.println(ignore);
        } while (ignore !='\n');

      if(ch == answer) System.out.println("** Right **");
      else {
        System.out.println("Sorry, you are ");
        if(ch < answer) System.out.println("too low");
        else System.out.println("too high");
        System.out.println("Try again!\n");
      }
    } while(answer != ch);
  }
}


