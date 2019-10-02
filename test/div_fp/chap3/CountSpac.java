//Count number of spaces for the characters read from the keyboard.
class CountSpac {
  public static void main(String args[])
    throws java.io.IOException {
      //char i;
      int x, count = 0;
      System.out.println("please input any charactors you want: ");
      System.out.println("input period sign to finsh!");
      do {
        // i = (char) System.in.read();//read charactors from keyboard
         x = System.in.read();//get numbers for the charators read from keyboard
           if (x ==32) count++; //count number of spaces
       } while ( x != 46);
    System.out.println(" There are " + count + " of spaces inputted!");
   }
}



