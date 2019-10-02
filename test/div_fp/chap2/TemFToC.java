//Print a table of temperature.
class TemFToC {
  public static void main(String args[]) {
    double f, c;
    f = 0;
    while ( f<= 100) {
          c = (f-32)*5/9;
          System.out.println( f+ "F is " + c + "C");
          f++;
    }
  }
}



