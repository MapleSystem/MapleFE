// This is a test file taken from open source project.
// https://github.com/eugenp/tutorials/blob/master/jmh/src/main/java/com/baeldung/bitset/Sizing.java
package com.baeldung.bitset;

import org.openjdk.jol.info.ClassLayout;
import org.openjdk.jol.info.GraphLayout;

import java.util.BitSet;

public class Sizing {

    public static void main(String[] args) {
        boolean[] ba = new boolean[10000];
        System.out.println(ClassLayout.parseInstance(ba).toPrintable());

        BitSet bitSet = new BitSet(10000);
        System.out.println(GraphLayout.parseInstance(bitSet).toPrintable());
    }
}
