//A promotion test.
class PromDemoT{
  public static void main(String args[]) {
    byte b;
    int i;
    b = 20;
    i = b * b;

    b = 11;
    b =(byte) (b * b);//test to see if cast needed.

    System.out.println("i and b:" + i + " " + b);
   
    char ch1 = 'a', ch2 = 'b';
    ch1 =(char) (ch1 + ch2);

    System.out.println(" ch1 is " + ch1 + ch2);

  }
}


