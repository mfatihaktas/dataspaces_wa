#include <stdio.h>
#include "common.h"

int common_put(int number){
	return dspaces_put("number", 1, sizeof(int), 0, 0, 0, 0, 0, 0, &number);
}
int common_get(int *number){
  return dspaces_get("number", 1, sizeof(int), 0, 0, 0, 0, 0, 0, number);
}
