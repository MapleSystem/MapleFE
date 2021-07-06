import myX, * as M from "./re-export";
console.log(myX, M.getx());
M.setx(3);
console.log(myX, M.getx());
