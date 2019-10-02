// Covert Roman number to an integer
public class Convertnum1 {
  public static void main(String[] args) {
    String s = "GOOD";
    int len = s.length();

    s = s + " ";
    int res = 0;

    for (int x = 0; x < len; x++) {
      char c = s.charAt(x);
      char next_c = s.charAt(x+1);

      if (c == 'M') {
        res += 1000;
      } else if (c == 'C') {
        if (next_c == 'M') {
          res += 900;
          x++;
        } else if (next_c == 'D') {
          res += 400;
          x++;
        } else {
          res += 100;
        }
      } else if (c == 'D') {
        res += 500;
      } else if (c == 'X') {
        if (next_c == 'C') {
          res += 90;
          x++;
        } else if (next_c == 'L') {
          res += 40;
          x++;
        } else {
          res += 10;
        }
      } else if (c == 'L') {
        res += 50;
      } else if (c == 'I') {
        if (next_c == 'X') {
          res += 9;
          x++;
        } else if (next_c == 'V') {
          res += 4;
          x++;
        } else {
          res++;
        }
      } else { // if (c == 'V')
        res += 5;
      }
    }
    System.out.println("\nRoman Number : " + s);
    System.out.println("Equivalent Integer number : " + res + "\n");
  }
}
