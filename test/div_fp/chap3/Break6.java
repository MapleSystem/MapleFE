//Check the output for different places to pu a label.
class Break6 {
  public static void main(String args[]) {
    int x =0, y = 0;

//Here put label before for statement.
stop1: for(x=0; x< 5; x++) {
         for(y=0; y < 5; y++) {
           if(y == 3) break stop1;;
           System.out.println( "x and y: " + x + " " + y);
         }
       }
       
       System.out.println();

//Put the label immediately before {
       for(x =0; x<5; x++)
stop2:   {
           for(y = 0; y <5; y++) {
             if(y == 3) break stop2;
             System.out.println("x and y: " + x + " " + y) ;
           }
         }
  }
}
      
