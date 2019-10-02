import java.util.*;

public class Roundup {
  public static void main(String[] args) {
    int mark1 = 75, mark2 = 150, mark3 = 225;
    int percentage = ((mark1 + mark2) * 100) / mark3;
    System.out.println("(" + mark1 + " + " + mark2 + "} * 100) / " + mark3);
    System.out.print("\nPercentage of Mark : " + percentage + "%\n");
  }
}
