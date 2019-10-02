/* 
  this file converts inches to meters.

  Call this program "IncToMetTable.java".
*/

class IncToMetTable {
  public static void main(String args[]) {
    double Inch, Met; //define variable type
    int Counter;
    Counter = 0;//set initial Counter to 0
 
    for (Inch = 1; Inch <= 144; Inch ++) { 
      Met = Inch / 39.73;// convert to Meters
      System.out.println(Inch + " Inches is " + Met +" Meters." );
    
      Counter ++; //every 12th line, print a blank line
   
      if(Counter == 12) { 
        System.out.println();
        Counter = 0;//reset the line counter
      }
    }
  }
}
