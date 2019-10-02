
interface Inter {
    @interface Marker {}
}

@interface ID {
    int id();
}

@interface Marker {}

public class Annot1 {

    @interface Marker {}

    public static void main(String argv[])   {
        System.out.println(run());
    }

    public static int run() {
        return 0;
    }
}


