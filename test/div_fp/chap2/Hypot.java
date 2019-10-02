/*  
    Use the Pythagonrean theorem to 
    find the lenght of the hypotenuse
    given the lenghts of the two opposing
    sides.
*/
class Hypot {
  public static void main(String args[]) {
    double x, y, z;

    x = 3;
    y = 4;

    z = Math.sqrt(x*x + y*y);

    System.out.println("Hypotenuse is" + z);
  }
}

