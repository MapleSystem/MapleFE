@ interface annotID1 {
}

@	interface annotID2 {
}

@ /* comments */ interface annotID3 {
}

@ //one line comments
interface annotID4 {}

public class Annot3 {

    @annotID1 @annotID2 @annotID3 @ annotID4 public void m() {
    }

    public static void main(String argv[])   {
        System.out.println(run());
    }

    public static int run() {
        return 0;
    }
}

