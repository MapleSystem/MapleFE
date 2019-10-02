@interface testMarker {}

@interface testID {
    byte retByte();
    short retShort();
    int retInt();
    long retLong();
    float retFloat();
    double retDouble();
    boolean retBoolean();
    char retChar();
    String retString();
    Class retClass();
    testMarker retAttr();

    byte [] retByteArr();
    short [] retShortArr();
    int [] retIntArr();
    long [] retLongArr();
    float [] retFloatArr();
    double [] retDoubleArr();
    boolean [] retBooleanArr();
    char [] retCharArr();
    String [] retStringArr();
    Class [] retClassArr();
    testMarker [] retAttrArr();
}

public class Annot6 {

    static String str1 = "test1", str2 = "test2";
    public static enum test {str1, str2};

    @testID (retByte = 1, retShort = 1, retInt = 1, retLong = 1L, retFloat = 1.0f, retDouble = 1.0,
             retBoolean = true, retChar = 'a', retString = "qwe", retClass = Object.class, retAttr = @testMarker,
             retByteArr = 1, retShortArr = 1, retIntArr = 1, retLongArr = 1L, retFloatArr = 1.0f, retDoubleArr = 1.0, 
             retBooleanArr = true, retCharArr = 'a', retStringArr = "qwe", retClassArr = Object.class, retAttrArr = @testMarker)
             public void m() {
    }
    
    public static void main(String argv[])   {
        System.out.println(run());
    }

    public static int run() {
        return 0;
    }
}


