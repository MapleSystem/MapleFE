//Guess the letter game.
class Guess {
  public static void main(String args[])
    throws java.io.IOException {
 
    char ch, answer = 'K';

    System.out.println("I'm thinking of an upper case letter between A and Z.");
    System.out.print("Can you guess it: ");

    ch = (char) System.in.read(); //read a char from the keyboard

    if(ch == answer) System.out.println("**Right**");
    else {
      System.out.print("...Sorry, you're ");
        //a nested if
        if( ch < answer) System.out.println("too low");
        else System.out.println("too high");
    }  
  }
}

