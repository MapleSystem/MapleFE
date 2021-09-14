type Default = "default";
type DefaultUnknown = "unknown";

type MyType = {
    [K: string]: typeof K extends Default ? string :
      typeof K extends DefaultUnknown ? unknown : never
};

