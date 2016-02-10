#!/usr/bin/awk -f
{
  print strftime("%Y-%m-%d %H:%M:%S"), $0;
  fflush();
}

