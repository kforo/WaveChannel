#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "checksum_utils/rs_code.h"
#define RS_SYMSIZE			2
#define RS_GFPOLY			0x11d
#define RS_FCR				1
#define RS_PRIM				1


int main()
{
  void *rs_hander = NULL;
  char test_str[9] = { 0,1,2,1,3 };
  char checksum[2] = { 0 };
  rs_hander = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM,1, ((1 << RS_SYMSIZE) - 1 - (5 + 1)));
  if (rs_hander == NULL) {
    printf("init rs hander failed\n");
    return 1;
  }
  printf("%d %d %d %d %d %d %d %d %d\n", test_str[0], test_str[1], test_str[2], test_str[3], test_str[4], test_str[5], test_str[6], test_str[7], test_str[8]);
  printf("%d %d\n", checksum[0], checksum[1]);
  encode_rs_char(rs_hander, test_str, checksum);
  printf("%d %d %d %d %d %d %d %d %d\n", test_str[0], test_str[1], test_str[2], test_str[3], test_str[4], test_str[5], test_str[6], test_str[7], test_str[8]);
  printf("%d %d\n", checksum[0], checksum[1]);
  return 0;
}