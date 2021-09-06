// ref: cocos game.ts line 922
interface IntfX {
  IntfX_fd: boolean;
}

type Test = IntfX & { intersect_fd: NonNullable<IntfX["IntfX_fd"]> };
