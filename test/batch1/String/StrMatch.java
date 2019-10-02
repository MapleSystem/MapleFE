// Check whether a region in String x match to String y
public class StrMatch {
  public static void main(String[] args) {
    String x = "Beijing Shanghai Guangzhou Shenzhen";
    String y = "Shenzhen Guangzhou Shanghai Beijing";

    // Is characters 0 thru 7 in string x 
    // match characters 28 thru 34 in string y 
    boolean str_match1 = x.regionMatches(0, y, 28, 7);

    // Is characters 8 thru 15 in string x 
    // match characters 9 thru 15 in string y
    boolean str_match2 = x.regionMatches(8, y, 9, 8);

    System.out.println("x[0 - 6] == y[28 - 34]? " + str_match1);
    System.out.println("x[8 - 15] == y[9 - 15]? " + str_match2);
  }
}
