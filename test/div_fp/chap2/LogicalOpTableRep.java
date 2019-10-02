//A true table for the logical operators.
class LogicalOpTableRep {
  public static void main(String args[]) {

    Boolean p, q;

   System.out.println("P\tQ\tAnd\tOR\tXOR\tNotP");
    
    p = true; q = true;
    System.out.print("1"+"\t"+"1"+"\t");
    if((p&q) == true)
      System.out.print("1"+"\t");
    if((p&q) == false)
      System.out.print("0"+"\t");
    if((p|q) == true)
      System.out.print("1"+"\t");
    if((p|q) == false)
      System.out.print("0"+"\t");
    if((p^q) == true)
      System.out.print("1"+"\t");
    if((p^q) == false)
      System.out.print("0"+"\t");
    if((!p) == true)
      System.out.println("1");
    if((!p) == false)
      System.out.println("0"); 
  }
}

