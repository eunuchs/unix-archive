main()
{
  int i, j, in;
  char buf[512];

  do {
    printf("Input device: ");
    gets(buf);
    in = open(buf, 0);
  } while (in <= 0);
  do {
    printf("Output device: ");
    gets(buf);
    j = open(buf, 1, 0777);
  } while (j <= 0);

  while (1) {
    if ((i = read(in, buf, 512)) <= 0) {
      exit(1);
    }
    write(j, buf, 512);
  }
}
