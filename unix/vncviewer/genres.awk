{
  if ($0 == "char *fallback_resources[] = {") {
    printme = 1
  }
  else if ($0 == "NULL") {
    printme = 0
  }
  else if (printme == 1) {
    sub(/\"\,/, "");
    sub(/^\"/, "");
    sub(/^\/\*/, "!");
    sub(/^\ \*\//, "!");
    sub(/^\ \*/, "!");
    sub(/\\\\n/, "\\n");
    sub(/\ \*\/$/, "");
    print
  }
}
