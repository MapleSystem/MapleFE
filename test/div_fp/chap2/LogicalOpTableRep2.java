//A true table for the logical operators.
class LogicalOpTableRep2 {
  public static void main(String args[]) {

    boolean p, q;

    System.out.println("P\tQ\tAND\tOR\tXOR\tNOT");

    p = true; q = true;
    System.out.print((int)(p) + "\t" +(int)(q) + "\t");
    System.out.print((int)(p&q) + "\t" +(int)(p|q) + "\t");
    System.out.println((int)(p^q) + "\t" +(int)(!q)); 
  }
}

