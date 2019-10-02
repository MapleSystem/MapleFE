/*
 one-dimensional array Example 1
*/
public class OneDArray1 {
  public static void main(String[] args) {
    int month_days[] = new int[12];
 
    month_days[0] = 31;
    month_days[1] = 28;
    month_days[2] = 31;
    month_days[3] = 30;
    month_days[4] = 31;
    month_days[5] = 30;
    month_days[6] = 31;
    month_days[7] = 31;
    month_days[8] = 30;
    month_days[9] = 31;
    month_days[10] = 30;
    month_days[11] = 31;

    System.out.println("January has " + month_days[0] + " days.");
    System.out.println("November has " + month_days[10] + " days.");
  }
}
