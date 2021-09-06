enum Direction {
  LEFT,
  RIGHT,
}

const rec: Record<string, string> = {
  [Direction.LEFT]: "left",
  [Direction.RIGHT]: "right",
  default: "Unknown",
};

console.log(rec);
